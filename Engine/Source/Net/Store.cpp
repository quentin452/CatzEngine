/******************************************************************************

   For 'callback' pass temporary 'Purchase' object, and not from '_purchases',
      this is in case user would call 'consume' inside the callback,
      which would remove it from container making the reference invalid.

/******************************************************************************/
#include "../../stdafx.h"
#include "../Platforms/iOS/iOS.h"

#if DEBUG
#define WIN_STORE Windows::ApplicationModel::Store::CurrentAppSimulator
#else
#define WIN_STORE Windows::ApplicationModel::Store::CurrentApp
#endif
namespace EE {
/******************************************************************************/
PlatformStore Store;
/******************************************************************************/
static void ConvertDate(Str text, DateTime &date) {
    // sample text "2016-03-08T04:49:36Z"
    text.replace('T', ' ').replace('Z', '\0');
    date.fromText(text);
}
/******************************************************************************/
static Bool BackgroundUpdate(Thread &thread) { return ((PlatformStore *)thread.user)->backgroundUpdate(); }
Bool PlatformStore::backgroundUpdate() {
    if (_get_item_details.elms()) // want to get item details
    {
        SyncLockerEx locker(_lock);
        if (_get_item_details.elms()) {
            Memc<Str> item_ids;
            Swap(item_ids, _get_item_details);
            locker.off();
#if ANDROID
            JNI jni;
            if (jni && ActivityClass)
                if (JMethodID getItemDetails = jni.staticFunc(ActivityClass, "getItemDetails", "(ZZIJJ)Z")) {
                    Memt<Item> items;
                    MemPtr<Item> items_ptr = items;
                    MemPtr<Str> item_ids_ptr = item_ids;
                    Bool ok = jni->CallStaticBooleanMethod(ActivityClass, getItemDetails, jboolean(supportsItems()), jboolean(supportsSubscriptions()), jint(item_ids_ptr.elms()), jlong(&item_ids_ptr), jlong(&items_ptr));
                    if (items.elms()) {
                        locker.on();
                        REPA(items)
                        Swap(_new_items.New(), items[i]);
                        locker.off();
                    }
                }
#endif
        }
        return true; // keep thread alive
    }

    if (_refresh_purchases) // want to get active purchases
    {
        SyncLockerEx locker(_lock);
        if (_refresh_purchases) {
            locker.off();
#if ANDROID
            JNI jni;
            if (jni && ActivityClass)
                if (JMethodID getPurchases = jni.staticFunc(ActivityClass, "getPurchases", "(ZJ)Z")) {
                    Memc<Purchase> purchases;
                    MemPtr<Purchase> purchases_ptr = purchases;
                    Bool regular = (supportsItems() && jni->CallStaticBooleanMethod(ActivityClass, getPurchases, jboolean(false), jlong(&purchases_ptr)));
                    Bool subs = (supportsSubscriptions() && jni->CallStaticBooleanMethod(ActivityClass, getPurchases, jboolean(true), jlong(&purchases_ptr)));
                    locker.on();
                    Swap(_new_purchases, purchases);
                    _has_new_purchases = true;
                    locker.off();
                }
#endif
            _refresh_purchases = false; // disable at the end because this is just a bool, doesn't require 'SyncLocker'
        }
        return true; // keep thread alive
    }

    if (_consume.elms()) // want to consume purchases
    {
        SyncLockerEx locker(_lock);
        if (_consume.elms()) // want to consume purchases
        {
            Processed purchase;
            Swap(purchase.token, _consume.first());
            _consume.remove(0, true);
            if (C Purchase *existing = findPurchaseByToken(purchase.token))
                SCAST(Purchase, purchase) = *existing;
            locker.off();
#if ANDROID
            JNI jni;
            if (jni && ActivityClass)
                if (JMethodID consume = jni.staticFunc(ActivityClass, "consume", "(Ljava/lang/String;)I"))
                    if (JString j_token = JString(jni, purchase.token)) {
                        purchase.result = (RESULT)jni->CallStaticIntMethod(ActivityClass, consume, j_token());
                        locker.on();
                        Swap(_processed.New(), purchase);
                        locker.off();
                    }
#endif
        }
        return true; // keep thread alive
    }

    return false;
}
/******************************************************************************/
PlatformStore::~PlatformStore() {
    _thread.del(); // delete the thread before anything else
#if IOS
    [IAPMgr release];
    IAPMgr = null;
#endif
}
PlatformStore::PlatformStore() {
#if WINDOWS_NEW
    try // exception may occur when using 'CurrentApp' instead of 'CurrentAppSimulator' on a debug build
    {
        _supports_items = (WIN_STORE::LicenseInformation->IsActive && !WIN_STORE::LicenseInformation->IsTrial); // purchases are unavailable for trial according to this page, https://msdn.microsoft.com/en-us/windows/uwp/monetize/enable-in-app-product-purchases "In-app products cannot be offered from a trial version of an app"
    } catch (...) {
    }

// list purchases
#if 0 // can't do this because 'token' is unavailable in this method, instead 'refreshPurchases' needs to be used
      auto product=WIN_STORE::LicenseInformation->ProductLicenses->First();
      REP(WIN_STORE::LicenseInformation->ProductLicenses->Size)
      {
         if(Bool active=product->Current->Value->IsActive)
         {
          //Str key=product->Current->Key->Data(); this is the same as 'id'
            Purchase &purchase=_purchases.New();
            purchase.id=product->Current->Value->ProductId->Data();
          //purchase.token=; // unknown
            purchase.date.zero(); // unknown
         }
         product->MoveNext();
      }
#endif
#elif IOS
    if (IAPMgr = [[IAPManager alloc] init]) {
        _supports_items = _supports_subs = [SKPaymentQueue canMakePayments];
        [[SKPaymentQueue defaultQueue] addTransactionObserver:IAPMgr];
        //[[SKPaymentQueue defaultQueue] transactions] are always empty here, so there's no point in listing them
    }
#endif
}
C PlatformStore::Item *PlatformStore::findItem(C Str &item_id) C {
    if (item_id.is())
        REPA(_items)
    if (Equal(_items[i].id, item_id, true))
        return &_items[i];
    return null;
}
C PlatformStore::Purchase *PlatformStore::findPurchase(C Str &item_id) C {
    if (item_id.is())
        REPA(_purchases)
    if (Equal(_purchases[i].id, item_id, true))
        return &_purchases[i];
    return null;
}
C PlatformStore::Purchase *PlatformStore::findPurchaseByToken(C Str &token) C {
    if (token.is())
        REPA(_purchases)
    if (Equal(_purchases[i].token, token, true))
        return &_purchases[i];
    return null;
}

static void Update(PlatformStore &store) { store.update(); }
void PlatformStore::update() {
    // del thread
    if (_thread.created() && !_thread.active()) // delete thread if it's no longer active to free system resources
    {
        SyncLocker locker(_lock);
        if (_thread.created() && !_thread.active())
            _thread.del();
    }
    if (_thread.created())
        App.includeFuncCall(Update, T); // if still created then call the callback again later

    // process updates
    if (_new_items.elms()) {
        SyncLockerEx locker(_lock);
        if (_new_items.elms()) {
            REPA(_new_items) {
                Item &src = _new_items[i];
                Item *dest = ConstCast(findItem(src.id));
                if (!dest)
                    dest = &_items.New();
                Swap(src, *dest);
            }
            _new_items.clear();
            locker.off();
            if (callback)
                callback(REFRESHED_ITEMS, null);
        }
    }
    if (_has_new_purchases) // test bool because purchases container may be empty when it was received
    {
        SyncLockerEx locker(_lock);
        if (_has_new_purchases) {
            _has_new_purchases = false;
            Swap(_purchases, _new_purchases);
            _new_purchases.clear();
            locker.off();
            if (callback)
                callback(REFRESHED_PURCHASES, null);
        }
    }
    if (_processed.elms()) {
        SyncLockerEx locker(_lock);
        REPA(_processed) {
            Processed &purchase = _processed[i];
            C Purchase *existing = findPurchaseByToken(purchase.token);
            switch (purchase.result) // first add/remove to list of purchases
            {
            case PlatformStore::CONSUMED:
            case PlatformStore::REFUND:
                _purchases.removeData(existing, true);
                break; // remove
            case PlatformStore::PURCHASED:
                if (!existing)
                    _purchases.add(purchase);
                break; // add
            }
            if (callback)
                callback(purchase.result, &purchase); // !! here don't pass purchase from '_purchases' !!
        }
        _processed.clear();
    }
}
Bool PlatformStore::refreshItems(C CMemPtr<Str> &item_ids) {
    if (item_ids.elms()) {
#if WINDOWS_NEW
        try // exception may occur when using 'CurrentApp' instead of 'CurrentAppSimulator' on a debug build
        {
#if 0 // this crashes on Desktop
         Platform::Collections::Vector<Platform::String^> ^product_ids=ref new Platform::Collections::Vector<Platform::String^>(item_ids.elms());
         FREPA(item_ids)product_ids->SetAt(i, ref new Platform::String(item_ids[i]));
         auto op=WIN_STORE::LoadListingInformationByProductIdsAsync(product_ids);
#else // this works ok but returns all items
            auto op = WIN_STORE::LoadListingInformationAsync();
#endif
            op->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<Windows::ApplicationModel::Store::ListingInformation ^>([this](Windows::Foundation::IAsyncOperation<Windows::ApplicationModel::Store::ListingInformation ^> ^ op, Windows::Foundation::AsyncStatus status) {
                // this will be called on the main thread
                if (status == Windows::Foundation::AsyncStatus::Completed)
                    if (auto listing = op->GetResults()) {
                        /*Str name=listing->Name->Data(),
                              desc=listing->Description->Data(),
                             price=listing->FormattedPrice->Data(),
                            market=listing->CurrentMarket->Data();*/

                        // even though we're getting full list of items and we could first delete existing '_items', don't do that for thread-safety in case secondary threads already operate on existing items
                        if (listing->ProductListings) {
                            auto product = listing->ProductListings->First();
                            {
                                SyncLocker locker(_lock);
                                REP(listing->ProductListings->Size) {
                                    // Str key=product->Current->Key->Data(); this is the same as 'id'
                                    Str id = product->Current->Value->ProductId->Data();
                                    PlatformStore::Item *item = ConstCast(findItem(id));
                                    if (!item)
                                        item = &_items.New();
                                    item->subscription = false;
                                    item->id = id;
                                    item->name = product->Current->Value->Name->Data();
                                    item->desc = product->Current->Value->Description->Data();
                                    item->price = product->Current->Value->FormattedPrice->Data();
                                    product->MoveNext();
                                }
                            }
                            if (callback)
                                callback(PlatformStore::REFRESHED_ITEMS, null);
                        }
                    }
            });
        } catch (...) {
            return false;
        }
        return true;
#elif ANDROID
        if (supportsItems() || supportsSubscriptions()) {
            Bool added = false;
            SyncLocker locker(_lock);
            REPA(item_ids) {
                C Str &item_id = item_ids[i];
                REPA(_get_item_details)
                if (Equal(_get_item_details[i], item_id, true))
                    goto has;
                _get_item_details.add(item_id);
                added = true;
            has:;
            }
            if (_get_item_details.elms() && !_thread.active()) {
                _thread.create(BackgroundUpdate, this);
                App.includeFuncCall(Update, T);
            } // !! Call this under lock because of '_thread.create' !!
            return true;
        }
        return false;
#elif IOS
        if (IAPMgr)
            if (NSMutableArray *array = [NSMutableArray arrayWithCapacity:item_ids.elms()]) {
                FREPA(item_ids)[array addObject:NSStringAuto(item_ids[i])];
                SKProductsRequest *pr = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:array]];
                pr.delegate = IAPMgr;
                [pr start];
                //[pr release]; don't release here, instead this is released in 'IAPManager.didReceiveResponse'
                //[array release]; // 'arrayWithCapacity' shouldn't be manually released
                return true;
            }
        return false;
#endif
    }
    return true;
}
Bool PlatformStore::refreshPurchases() {
#if WINDOWS_NEW
    try // exception may occur when using 'CurrentApp' instead of 'CurrentAppSimulator' on a debug build
    {
        auto op = WIN_STORE::GetAppReceiptAsync();
        op->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<Platform::String ^>([this](Windows::Foundation::IAsyncOperation<Platform::String ^> ^ op, Windows::Foundation::AsyncStatus status) {
            // this will be called on the main thread
            if (status == Windows::Foundation::AsyncStatus::Completed)
                if (auto receipt = op->GetResults()) {
                    C wchar_t *r = receipt->Data();
                    FileText f;
                    f.readMem(r, Length(r) * SIZE(*r), UTF_16); // use 'UTF_16' because 'r' is a 16-bit string
                    XmlData xml;
                    xml.load(f);
                    if (XmlNode *receipt = xml.findNode("Receipt")) {
                        Memc<Purchase> purchases;
                        FREPA(receipt->nodes) {
                            XmlNode &node = receipt->nodes[i];
                            if (node.name == "ProductReceipt") {
                                UID token_id;
                                if (XmlParam *token = node.findParam("Id"))
                                    if (token_id.fromCanonical(token->value)) // token in guid format
                                        if (XmlParam *id = node.findParam("ProductId")) {
                                            Purchase &purchase = purchases.New();
                                            purchase.id = id->value;
                                            purchase.token = token_id.asHex();
                                            purchase.date.zero();
                                            if (XmlParam *date = node.findParam("PurchaseDate"))
                                                ConvertDate(date->value, purchase.date);
                                        }
                            }
                        }
                        SyncLocker locker(_lock);
                        Swap(_purchases, purchases);
                    }
                    if (callback)
                        callback(REFRESHED_PURCHASES, null);
                }
        });
    } catch (...) {
        return false;
    }
#elif ANDROID
    if (supportsItems() || supportsSubscriptions()) {
        if (!_refresh_purchases) {
            SyncLocker locker(_lock);
            if (!_refresh_purchases) {
                _refresh_purchases = true;
                if (!_thread.active()) {
                    _thread.create(BackgroundUpdate, this);
                    App.includeFuncCall(Update, T);
                } // !! Call this under lock because of '_thread.create' !!
            }
        }
        return true;
    }
    return false;
#elif IOS
    // there's no such method for Apple
#endif
    return true;
}
PlatformStore &PlatformStore::restorePurchases() {
#if IOS
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
#endif
    return T;
}
PlatformStore::RESULT PlatformStore::buy(C Str &id, Bool subscription, C Str &data) {
    if (!id.is())
        return ITEM_UNAVAILABLE;
    if (!(subscription ? supportsSubscriptions() : supportsItems()))
        return SERVICE_UNAVAILABLE;
#if WINDOWS_NEW
    auto op = WIN_STORE::RequestProductPurchaseAsync(ref new Platform::String(id));
    op->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<Windows::ApplicationModel::Store::PurchaseResults ^>([this, id](Windows::Foundation::IAsyncOperation<Windows::ApplicationModel::Store::PurchaseResults ^> ^ op, Windows::Foundation::AsyncStatus status) // !! have to pass 'id' as a copy and not reference because the function might return before op finishes !!
                                                                                                                                     {
                                                                                                                                         // this will be called on the main thread
                                                                                                                                         RESULT result = UNKNOWN;
                                                                                                                                         Purchase purchase;
                                                                                                                                         purchase.id = id;
                                                                                                                                         purchase.date.zero();
                                                                                                                                         if (status == Windows::Foundation::AsyncStatus::Completed)
                                                                                                                                             if (auto results = op->GetResults()) {
                                                                                                                                                 switch (results->Status) {
                                                                                                                                                 case Windows::ApplicationModel::Store::ProductPurchaseStatus::Succeeded:
                                                                                                                                                     result = PURCHASED;
                                                                                                                                                     break;
                                                                                                                                                 case Windows::ApplicationModel::Store::ProductPurchaseStatus::AlreadyPurchased:
                                                                                                                                                     result = ALREADY_OWNED;
                                                                                                                                                     break;
                                                                                                                                                 case Windows::ApplicationModel::Store::ProductPurchaseStatus::NotFulfilled:
                                                                                                                                                     result = ALREADY_OWNED;
                                                                                                                                                     break;                                                                  // The transaction did not complete because the last purchase of this consumable in-app product has not been reported as fulfilled to the Windows Store - https://msdn.microsoft.com/en-us/library/windows/apps/windows.applicationmodel.store.productpurchasestatus
                                                                                                                                                 case Windows::ApplicationModel::Store::ProductPurchaseStatus::NotPurchased: // The purchase did not occur because the user decided not to complete the transaction (or the transaction failed for other reasons) - https://msdn.microsoft.com/en-us/library/windows/apps/windows.applicationmodel.store.productpurchasestatus
                                                                                                                                                 {
                                                                                                                                                     if (_items.elms() && !findItem(id))
                                                                                                                                                         result = ITEM_UNAVAILABLE; // on Windows if we have information about one item, then it means we know all items, and if this item is not among them, then we know that it's unavailable
                                                                                                                                                     else
                                                                                                                                                         result = USER_CANCELED;
                                                                                                                                                 } break;
                                                                                                                                                 }
                                                                                                                                                 if (result == PURCHASED || result == ALREADY_OWNED) {
                                                                                                                                                     UID token;
                                                                                                                                                     token.guid() = results->TransactionId;
                                                                                                                                                     purchase.token = token.asHex();
                                                                                                                                                 }

                                                                                                                                                 if (results->ReceiptXml) {
                                                                                                                                                     C wchar_t *receipt = results->ReceiptXml->Data();
                                                                                                                                                     FileText f;
                                                                                                                                                     f.readMem(receipt, Length(receipt) * SIZE(*receipt), UTF_16); // use 'UTF_16' because 'receipt' is a 16-bit string
                                                                                                                                                     XmlData xml;
                                                                                                                                                     xml.load(f);
                                                                                                                                                     if (XmlNode *receipt = xml.findNode("Receipt"))
                                                                                                                                                         if (XmlNode *ProductReceipt = receipt->findNode("ProductReceipt"))
                                                                                                                                                             if (XmlParam *date = ProductReceipt->findParam("PurchaseDate"))
                                                                                                                                                                 ConvertDate(date->value, purchase.date);
                                                                                                                                                 }

                                                                                                                                                 // first add to the list of purchases
                                                                                                                                                 if (result == PURCHASED && !findPurchaseByToken(purchase.token)) {
                                                                                                                                                     SyncLocker locker(_lock);
                                                                                                                                                     _purchases.add(purchase);
                                                                                                                                                 }
                                                                                                                                             }

                                                                                                                                         // now call the callback
                                                                                                                                         if (callback)
                                                                                                                                             callback(result, &purchase); // !! here don't pass purchase from '_purchases' !!
                                                                                                                                     });
    return WAITING;
#elif ANDROID
    JNI jni;
    if (jni && ActivityClass)
        if (JMethodID buy = jni.staticFunc(ActivityClass, "buy", "(Ljava/lang/String;Ljava/lang/String;Z)I"))
            if (JString j_id = JString(jni, id))
                if (JString j_data = JString(jni, data))
                    return (RESULT)jni->CallStaticIntMethod(ActivityClass, buy, j_id(), j_data(), jboolean(subscription));
#elif IOS
    if (NSStringAuto ns_id = id) {
        SKMutablePayment *payment = [SKMutablePayment paymentWithProductIdentifier:ns_id];
        [[SKPaymentQueue defaultQueue] addPayment:payment];
        return WAITING;
    }
#endif
    return SERVICE_UNAVAILABLE;
}
PlatformStore::RESULT PlatformStore::consume(C Str &token) {
    if (!token.is())
        return NOT_OWNED;
#if WINDOWS_NEW
    C Purchase *purchase = findPurchaseByToken(token);
    if (!purchase)
        return NOT_OWNED;
#if 0 // this is unavailable in the simulator, however since 'ReportConsumableFulfillmentAsync' works fine, then use just that
   WIN_STORE::ReportProductFulfillment(ref new Platform::String(purchase->id));
#else
    UID token_id;
    token_id.fromHex(token);
    auto op = WIN_STORE::ReportConsumableFulfillmentAsync(ref new Platform::String(purchase->id), token_id.guid());                                                                                                                                                                         // both 'ProductId' and 'TransactionId' must be specified
    op->Completed = ref new Windows::Foundation::AsyncOperationCompletedHandler<Windows::ApplicationModel::Store::FulfillmentResult>([this, token](Windows::Foundation::IAsyncOperation<Windows::ApplicationModel::Store::FulfillmentResult> ^ op, Windows::Foundation::AsyncStatus status) // !! have to pass 'token' as a copy and not reference because the function might return before op finishes !!
                                                                                                                                     {
                                                                                                                                         // this will be called on the main thread
                                                                                                                                         Purchase temp;
                                                                                                                                         SyncLocker locker(_lock);
                                                                                                                                         C Purchase *existing = findPurchaseByToken(token);
                                                                                                                                         if (existing)
                                                                                                                                             temp = *existing;
                                                                                                                                         else {
                                                                                                                                             temp.date.zero();
                                                                                                                                             temp.token = token;
                                                                                                                                         } // copy to temp because we will remove it
                                                                                                                                         if (status == Windows::Foundation::AsyncStatus::Completed)
                                                                                                                                             switch (op->GetResults()) {
                                                                                                                                             case Windows::ApplicationModel::Store::FulfillmentResult::PurchaseReverted: {
                                                                                                                                                 _purchases.removeData(existing, true); // first remove from list of purchases
                                                                                                                                                 if (callback)
                                                                                                                                                     callback(REFUND, &temp); // now call the callback
                                                                                                                                             } break;

                                                                                                                                             case Windows::ApplicationModel::Store::FulfillmentResult::Succeeded: {
                                                                                                                                                 _purchases.removeData(existing, true); // first remove from list of purchases
                                                                                                                                                 if (callback)
                                                                                                                                                     callback(CONSUMED, &temp); // now call the callback
                                                                                                                                             } break;
                                                                                                                                             }
                                                                                                                                         else if (callback)
                                                                                                                                             callback(UNKNOWN, &temp);
                                                                                                                                     });
#endif
#elif ANDROID
    {
        SyncLocker locker(_lock);
        REPA(_consume)
        if (Equal(_consume[i], token, true))
            return WAITING;
        _consume.add(token);
        if (!_thread.active()) {
            _thread.create(BackgroundUpdate, this);
            App.includeFuncCall(Update, T);
        } // !! Call this under lock because of '_thread.create' !!
    }
    return WAITING;
#elif IOS
    for (SKPaymentTransaction *transaction in [[SKPaymentQueue defaultQueue] transactions])
        if (Equal(AppleString(transaction.transactionIdentifier), token, true)) {
            [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
            _purchases.removeData(findPurchaseByToken(token), true);
            return CONSUMED;
        }
    return NOT_OWNED;
#endif
    return SERVICE_UNAVAILABLE;
}
/******************************************************************************/
void PlatformStore::licenseTest() {
#if ANDROID
    JNI jni;
    if (jni && ActivityClass)
        if (JMethodID licenseTest = jni.staticFunc(ActivityClass, "licenseTest", "()V")) {
            _ltr = LTR_WAITING; // set before calling
            jni->CallStaticVoidMethod(ActivityClass, licenseTest);
            return;
        }
    _ltr = LTR_ERROR; // function not found
#endif
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if ANDROID
extern "C" {
/******************************************************************************/
JNIEXPORT void JNICALL Java_com_esenthel_Native_connected(JNIEnv *env, jclass clazz, jboolean supports_items, jboolean supports_subs) {
    Store._supports_items = supports_items;
    Store._supports_subs = supports_subs;
}
JNIEXPORT jstring JNICALL Java_com_esenthel_Native_getStr(JNIEnv *env, jclass clazz, jlong user, jint i) {
    JNI jni(env);
    MemPtr<Str> &strings = *(MemPtr<Str> *)user;
    return jni->NewStringUTF(UTF8(InRange(i, strings) ? strings[i] : S));
}
JNIEXPORT void JNICALL Java_com_esenthel_Native_listItem(JNIEnv *env, jclass clazz, jlong user, jstring sku, jstring name, jstring desc, jstring price, jboolean sub) {
    JNI jni(env);
    MemPtr<PlatformStore::Item> &items = *(MemPtr<PlatformStore::Item> *)user;
    PlatformStore::Item &item = items.New();
    item.subscription = sub;
    item.id = jni(sku);
    item.name = jni(name);
    item.desc = jni(desc);
    item.price = jni(price);
}
JNIEXPORT void JNICALL Java_com_esenthel_Native_listPurchase(JNIEnv *env, jclass clazz, jlong user, jstring sku, jstring data, jstring token, jlong date) {
    JNI jni(env);
    MemPtr<PlatformStore::Purchase> &purchases = *(MemPtr<PlatformStore::Purchase> *)user;
    PlatformStore::Purchase &purchase = purchases.New();
    purchase.id = jni(sku);
    purchase.data = jni(data);
    purchase.token = jni(token);
    if (date)
        purchase.date.from1970ms(date);
    else
        purchase.date.zero();
}
JNIEXPORT void JNICALL Java_com_esenthel_Native_purchased(JNIEnv *env, jclass clazz, jint result, jstring sku, jstring data, jstring token, jlong date) {
    JNI jni(env);
    PlatformStore::Processed purchase;
    purchase.result = PlatformStore::RESULT(result);
    purchase.id = jni(sku);
    purchase.data = jni(data);
    purchase.token = jni(token);
    if (date)
        purchase.date.from1970ms(date);
    else
        purchase.date.zero();

    {
        SyncLocker locker(Store._lock);
        Swap(Store._processed.New(), purchase);
    }
    App.includeFuncCall(Update, Store);
}
JNIEXPORT void JNICALL Java_com_esenthel_Native_licenseTest(JNIEnv *env, jclass clazz, jint result) {
    Store._ltr = PlatformStore::LICENSE_TEST_RESULT(result);
}
/******************************************************************************/
}
#endif
/******************************************************************************/
