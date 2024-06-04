#pragma once
/******************************************************************************
 * Copyright (c) Grzegorz Slazinski. All Rights Reserved.                     *
 * Titan Engine (https://esenthel.com) header file.                           *
/******************************************************************************/
struct SteamWorks {
    enum RESULT {
        STEAM_NOT_INITIALIZED,      // Steam failed to initialize
        STEAM_NOT_SETUP,            // 'Steam.webApiKey' was not specified
        APP_ID_UNAVAILABLE,         // App  Steam ID is unavailable
        USER_ID_UNAVAILABLE,        // User Steam ID is unavailable
        WAITING,                    // result is not yet known as the command was forwarded further, the result will be passed to the 'callback' function (if specified) at a later time
        ORDER_REQUEST_FAILED,       // order request failed
        ORDER_REQUEST_OK,           // order request was processed by Steam and it should be displayed on the app screen by "Steam Game Overlay" for user to authorize it
        ORDER_USER_CANCELED,        // order was canceled by the user
        ORDER_FINALIZE_FAILED,      // user authorized the order, however its finalization failed
        ORDER_FINALIZE_OK,          // user authorized the order and Steam finalized it successfully, this means that the order was fully processed and payment was transferred
        PURCHASE_STATUS_FAILED,     // failed to obtain the purchase status
        PURCHASE_STATUS_OK,         // purchase status was obtained successfully (this does not mean that the purchase was finalized, only that the status is known, for finalized status please check 'Purchase.finalized' member)
        SUBSCRIPTION_STATUS_FAILED, // failed to obtain the subscription status
        SUBSCRIPTION_STATUS_OK,     // subscription status was obtained successfully (this does not mean that a subscription exists, to check if it exists please investigate the 'subscription' function parameter)
    };

    enum E_LOBBYRESULT {
        E_EResultOK = 1,             // success
        E_EResultFail = 2,           // generic failure
        E_EResultNoConnection = 3,   // no/failed network connection
        E_EResultAccessDenied = 15,  // access is denied
        E_EResultTimeout = 16,       // operation timed out
        E_EResultLimitExceeded = 25, // Too much of a good thing
    };

    enum E_WORKSHOPRESULT {
        E_k_EResultOK = 1,                     // The operation completed successfully.
        E_k_EResultInsufficientPrivilege = 24, // The user is currently restricted from uploading content due to a hub ban, account lock, or community ban.They would need to contact Steam Support.
        E_k_EResultBanned = 17,                // The user doesn't have permission to upload content to this hub because they have an active VAC or Game ban.
        E_k_EResultTimeout = 16,               // The operation took longer than expected.Have the user retry the creation process.
        E_k_EResultNotLoggedOn = 21,           // The user is not currently logged into Steam.
        E_k_EResultServiceUnavailable = 20,    // The workshop server hosting the content is having issues - have the user retry.
        E_k_EResultInvalidParam = 8,           // One of the submission fields contains something not being accepted by that field.
        E_k_EResultAccessDenied = 15,          // There was a problem trying to save the title and description.Access was denied.
        E_k_EResultLimitExceeded = 25,         // The user has exceeded their Steam Cloud quota.Have them remove some items and try again.
        E_k_EResultFileNotFound = 9,           // The uploaded file could not be found.
        E_k_EResultDuplicateRequest = 29,      // The file was already successfully uploaded.The user just needs to refresh.
        E_k_EResultDuplicateName = 14,         // The user already has a Steam Workshop item with that name.
        E_k_EResultServiceReadOnly = 44,       // Due to a recent password or email change, the user is not allowed to upload new content.Usually this restriction will expire in 5 days, but can last up to 30 days if the account has been inactive recently.
    };

    enum USER_STATUS {
        STATUS_UNKNOWN, // Steam not connected
        STATUS_OFFLINE,
        STATUS_ONLINE,
        STATUS_AWAY,
    };
    enum FRIEND_STATE_CHANGE_FLAG {
        FRIEND_STATE_CHANGE_Name = 0x0001,
        FRIEND_STATE_CHANGE_Status = 0x0002,
        FRIEND_STATE_CHANGE_ComeOnline = 0x0004,
        FRIEND_STATE_CHANGE_GoneOffline = 0x0008,
        FRIEND_STATE_CHANGE_GamePlayed = 0x0010,
        FRIEND_STATE_CHANGE_GameServer = 0x0020,
        FRIEND_STATE_CHANGE_Avatar = 0x0040,
        FRIEND_STATE_CHANGE_JoinedSource = 0x0080,
        FRIEND_STATE_CHANGE_LeftSource = 0x0100,
        FRIEND_STATE_CHANGE_RelationshipChanged = 0x0200,
        FRIEND_STATE_CHANGE_NameFirstSet = 0x0400,
        FRIEND_STATE_CHANGE_Broadcast = 0x0800,
        FRIEND_STATE_CHANGE_Nickname = 0x1000,
        FRIEND_STATE_CHANGE_SteamLevel = 0x2000,
        FRIEND_STATE_CHANGE_RichPresence = 0x4000,
    };
    enum PERIOD : Byte {
        DAY,
        WEEK,
        MONTH,
        YEAR,
    };
    struct Purchase // purchase status, can be available only in PURCHASE_STATUS_OK
    {
        Bool finalized = false; // if purchase was finalized
        Int item_id = 0,        // Item       ID for this purchase
            cost_in_cents = 0;  // cost in cents for this purchase
        ULong user_id = 0;      // User Steam ID for this purchase
        Str currency,           // currency       of the  purchase
            country;            // country code   of the  purchase
        DateTime date;          // date           of the  purchase

        Purchase() { date.zero(); }
    };
    struct Subscription // subscription status, can be available only in SUBSCRIPTION_STATUS_OK
    {
        Bool active = false;   // if the subscription is active or canceled
        PERIOD period = DAY;   // period         of the  subscription
        Int item_id = 0,       // Item       ID for this subscription
            cost_in_cents = 0, // cost in cents for this subscription
            frequency = 0;     // frequency      of the  subscription
        Str currency;          // currency       of the  subscription
        DateTime created,      // date when the subscription was created
            last_payment,      // date of the last payment
            next_payment;      // date of the next payment

        Bool valid() C; // calculate if this subscription is currently valid (note that canceled subscription is still valid as long as it's not expired yet)

        Subscription() {
            created.zero();
            last_payment.zero();
            next_payment.zero();
        }
    };

    static Str StoreLink(C Str &app_id); // return a website link to Steam Store page for the specified App ID, 'app_id' example = "366810"

    void (*order_callback)(RESULT result, ULong order_id, C Str &error_message, C Purchase *purchase, C Subscription *subscription) = null; // pointer to a custom function that will be called with processed events of an order, 'result'=message received at the moment, 'purchase'=status of the purchase (can be available only in PURCHASE_STATUS_OK), 'subscription'=status of the subscription (can be available only in SUBSCRIPTION_STATUS_OK or it can be null which means that there's no subscription)

    void (*friend_state_changed)(ULong friend_id, UInt flags) = null; // pointer to a custom function that will be called when state of a friend changed, 'friend_id'=friend Steam ID, 'flags'=FRIEND_STATE_CHANGE_FLAG

    void (*OnLobbyCreated)(E_LOBBYRESULT result, ULong steamIDLobby) = null; // pointer to a custom function that will be called when state of lobby create is changed

    void (*OnLobbyEntered)(ULong m_ulSteamIDLobby, UInt m_rgfChatPermissions, bool m_bLocked, UInt m_EChatRoomEnterResponse) = null; // pointer to a custom function that will be called when state of lobby

    void (*OnLobbyInvite)(ULong m_ulSteamIDUser, ULong m_ulSteamIDLobby, ULong m_ulGameID) = null; // pointer to a custom function that will be called when state of lobby

    void (*OnLobbyJoined)(ULong m_steamIDLobby, ULong m_steamIDFriend) = null; // pointer to a custom function that will be called when state of lobby

    void (*OnLobbyChatUpdate)(ULong m_ulSteamIDLobby, ULong m_ulSteamIDUserChanged, ULong m_ulSteamIDMakingChange, UInt m_rgfChatMemberStateChange) = null; // pointer to a custom function that will be called when state of lobby

    void (*OnLobbyChatMsg)(ULong m_ulSteamIDLobby, ULong m_ulSteamIDUser, UInt m_eChatEntryType, UShort m_iChatID) = null; // pointer to a custom function that will be called when state of lobby

    void (*OnLobbyDataUpdate)(ULong m_ulSteamIDLobby, ULong m_ulSteamIDMember, UShort m_bSuccess) = null; // pointer to a custom function that will be called when state of lobby

    void (*OnP2PSessionRequest)(ULong m_steamIDRemote) = null; // pointer to a custom function that will be called when session request for p2p

    void (*OnSteamWorkshopItemReceived)(E_WORKSHOPRESULT EResult, ULong m_nPublishedFileId, bool m_bUserNeedsToAcceptWorkshopLegalAgreement) = null; // pointer to a custom function that will be called when workshop item is ready

    void (*OnSteamNetworkingMessagesSessionRequest)(ULong m_ulSteamIDMember) = null; // pointer to a custom function that will be called when we receive incoming connection

    void (*OnSteamNetworkingMessagesSessionRequestFailed)(int m_failedReason) = null; // pointer to a custom function that will be called when we receive notice of failed connection

    void (*OnSteamNetConnectionStatusChanged)() = null; // pointer to a custom function that will be called when we receive notice of conection status changed

    // get
    Bool initialized() C { return _initialized; }        // if Steam is initialized
    UInt appID() C;                                      // get app  Steam ID
    ULong userID() C;                                    // get user Steam ID
    Str userName() C;                                    // get user name
    USER_STATUS userStatus() C;                          // get user status
    Bool userAvatar(Image &image) C;                     // get user avatar
    LANG_TYPE appLanguage() C;                           // get current app language, LANG_NONE on fail
    COUNTRY country() C;                                 // get country in which user is currently located according to Steam
    DateTime date() C;                                   // get current date time in UTC time zone according to Steam
    UInt curTimeS() C;                                   // get real         time in current moment (seconds since application started, NOT modified by game 'speed', NOT affected by 'smooth' 'skipUpdate' and application pauses)
    Bool overlayAvailable() C;                           // get if "Steam Game Overlay" can be displayed to the user if needed (it is required for making any purchases), please note that the overlay could take a few seconds to start, so this function will initially return false while the overlay is loading
    Bool overlayAvailableMsgBox() C;                     // this method works like 'overlayAvailable', however on failure it additionally displays a message box, instructing the user to enable the Overlay through Steam Settings
    Bool overlayVisible() C { return _overlay_visible; } // get if "Steam Game Overlay" is currently visible
    Bool drmExit() C;                                    // perform a DRM (Digital Rights Management) check if the app should be closed. Calling this method is optional. If called then this method should be used in 'InitPre' function in the following way: "if(Steam.drmExit())return;" This method performs a check if the application was started by Steam Client. If it was, then false is returned and the app can continue to operate. If it was not started by Steam Client, then: APP_EXIT_IMMEDIATELY is added to the 'App.flag' (so that the App will exit after 'InitPre'), true is returned so you can return from 'InitPre' immediately, Steam Client will restart the app. If you're using "Steam DRM Wrapper" described in https://partner.steamgames.com/documentation/drm then you don't need this check.

    // friends
    Bool getFriends(MemPtr<ULong> friend_ids) C;    // get list of friend ID's, false          on fail
    Str userName(ULong user_id) C;                  // get user name          , ""             on fail
    USER_STATUS userStatus(ULong user_id) C;        // get user status        , STATUS_UNKNOWN on fail
    Bool userAvatar(ULong user_id, Image &image) C; // get user avatar        , false          on fail

    // operations
    SteamWorks &webApiKey(C Str8 &web_api_key); // set Steam WEB API Key, you can create/obtain it on "https://partner.steamgames.com/pub/groups/" website
    void createLobby(int maxPerson);            // send steam to create lobby

    // making purchases requires that the "Steam Game Overlay" is available, which you can check using 'overlayAvailable' method
    RESULT purchase(ULong order_id, Int item_id, C Str &item_name, Int cost_in_cents, C Str8 &currency = "USD");                                // purchase an item, 'order_id'=unique order ID (making multiple calls with the same OrderID will fail), 'item_id'=ID of the item to purchase, 'item_name'=name of the item to purchase, 'cost_in_cents'=cost in cents (for example 99=0.99$, 150=1.50$), 'currency'=currency of the purchase
    RESULT subscribe(ULong order_id, Int item_id, C Str &item_name, Int cost_in_cents, PERIOD period, Int frequency, C Str8 &currency = "USD"); // purchase an item, 'order_id'=unique order ID (making multiple calls with the same OrderID will fail), 'item_id'=ID of the item to purchase, 'item_name'=name of the item to purchase, 'cost_in_cents'=cost in cents (for example 99=0.99$, 150=1.50$), 'currency'=currency of the subscription, 'period,frequency'=how often the subscription should recur (for example, for every 1 month set "MONTH, 2", for every 2 weeks set "WEEK, 2")
    RESULT purchaseState(ULong order_id);                                                                                                       // get status of a purchase, this method will contact Steam and return the result through the 'callback' function
    RESULT subscriptionState();                                                                                                                 // get status of user subscriptions, this method will contact Steam and return the result through the 'callback' function

    // cloud IO, these methods require setting up Cloud settings on your App Steam Page - https://partner.steamgames.com/apps/cloud/XXX
    Long cloudAvailableSize() C; // get number of available bytes for cloud storage, -1 on fail
    Long cloudTotalSize() C;     // get number of total     bytes for cloud storage, -1 on fail

    Bool cloudDel(C Str &file_name);         // delete                   'file_name', false on fail
    Bool cloudExists(C Str &file_name);      // check if                 'file_name' exists
    Long cloudSize(C Str &file_name);        // get size of              'file_name', 0 on fail
    DateTime cloudTimeUTC(C Str &file_name); // get modification time of 'file_name'

    Bool cloudSave(C Str &file_name, File &file, Cipher *cipher = null);              // save data from 'file' to 'file_name' cloud file, false on fail, only data from current 'file' file position to the end of the file is saved
    Bool cloudLoad(C Str &file_name, File &file, Bool memory, Cipher *cipher = null); // load file from 'file_name' cloud file to 'file', false on fail, 'file' should be already opened for writing if 'memory' is set to false, if 'memory' is set to true then 'file' will be first reinitialized with 'writeMemFixed' before loading, which means that load result will not be stored into original 'file' target, but instead into a dynamically allocated memory

    Bool cloudSave(C Str &file_name, CPtr data, Int size); // save data from 'data' to 'file_name' cloud file, false on fail
    Bool cloudLoad(C Str &file_name, Ptr data, Int size);  // load data from 'file_name' cloud file to 'data', false on fail

    Int cloudFiles() C;                             // get number of files that are currently stored in the cloud
    Bool cloudFile(Int i, Str &name, Long &size) C; // get i-th cloud file name and size, false on fail

    // manage
    Bool init(); // manually initialize and return if initializion was ok, you don't need to call this as it is automatically called in the constructor
    void shut(); // manually shutdown Steam                              , you don't need to call this as it is automatically called in the  destructor

#if !EE_PRIVATE
  private:
#endif
    struct Operation : Download {
        UInt type;
        ULong order_id;
    };
    Bool _initialized = false, _overlay_visible = false;
    UInt _start_time_s = 0;
    Str8 _web_api_key;
    Memx<Operation> _operations;
#if EE_PRIVATE
    void update();
#endif

    SteamWorks() {}           // automatically calls 'init'
    ~SteamWorks() { shut(); } // automatically calls 'shut'
} extern Steam;
/******************************************************************************/
