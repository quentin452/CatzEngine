/******************************************************************************/
#include "stdafx.h"

#define SUPPORT_STEAM (DESKTOP && !WINDOWS_NEW && !ARM)
#define CLOUD_WORKAROUND 1 // perhaps this is needed only when running apps manually and not through Steam

#if SUPPORT_STEAM
#pragma warning(push)
#pragma warning(disable : 4996)
#include "../../../ThirdPartyLibs/begin.h"
//
#include "../../../ThirdPartyLibs/SteamWorks/steam_api.h"
//
#include "../../../ThirdPartyLibs/end.h"
#pragma warning(pop)
#endif

#if DEBUG && 0
#define ISteamMicroTxn "ISteamMicroTxnSandbox"
#else
#define ISteamMicroTxn "ISteamMicroTxn"
#endif

#if SUPPORT_MBED_TLS
#define STEAM_API "https://api.steampowered.com/" ISteamMicroTxn // use HTTPS if we can support it
#else
#define STEAM_API "http://api.steampowered.com/" ISteamMicroTxn // fall back to HTTP
#endif

#define DAYS_PER_MONTH 30.436875f // average number of days in a month (365.2425 days in a year / 12 months) https://en.wikipedia.org/wiki/Year#Summary
/******************************************************************************/
namespace EE {
/******************************************************************************/
Str SteamWorks::StoreLink(C Str &app_id) { return S + "https://store.steampowered.com/app/" + app_id; }
/******************************************************************************/
enum OPERATION_TYPE {
    PURCHASE,
    FINALIZE,
    QUERY_PURCHASE,
    QUERY_SUBSCRIPTION,
};
/******************************************************************************/
#if SUPPORT_STEAM
static struct SteamCallbacks // !! do not remove this !!
{
    STEAM_CALLBACK(SteamCallbacks, MicroTxnAuthorizationResponse, MicroTxnAuthorizationResponse_t, m_MicroTxnAuthorizationResponse);
    STEAM_CALLBACK(SteamCallbacks, PersonaStateChange, PersonaStateChange_t, m_PersonaStateChange);
    STEAM_CALLBACK(SteamCallbacks, AvatarImageLoaded, AvatarImageLoaded_t, m_AvatarImageLoaded);
    STEAM_CALLBACK(SteamCallbacks, GameOverlayActivated, GameOverlayActivated_t, m_GameOverlayActivated);
    STEAM_CALLBACK(SteamCallbacks, LobbyCreated, LobbyCreated_t, m_LobbyCreated);
    STEAM_CALLBACK(SteamCallbacks, LobbyEnter, LobbyEnter_t, m_LobbyEnter);
    STEAM_CALLBACK(SteamCallbacks, LobbyInvite, LobbyInvite_t, m_LobbyInvite);
    STEAM_CALLBACK(SteamCallbacks, GameLobbyJoinRequested, GameLobbyJoinRequested_t, m_GameLobbyJoinRequested);
    STEAM_CALLBACK(SteamCallbacks, LobbyChatUpdate, LobbyChatUpdate_t, m_LobbyChatUpdate);
    STEAM_CALLBACK(SteamCallbacks, LobbyChatMsg, LobbyChatMsg_t, m_LobbyChatMsg);
    STEAM_CALLBACK(SteamCallbacks, LobbyDataUpdate, LobbyDataUpdate_t, m_LobbyDataUpdate);
    STEAM_CALLBACK(SteamCallbacks, P2PSessionRequest, P2PSessionRequest_t, m_P2PSessionRequest);
    STEAM_CALLBACK(SteamCallbacks, CreateItemResult, CreateItemResult_t, m_CreateItemResult);
    STEAM_CALLBACK(SteamCallbacks, MessagesSessionRequest, SteamNetworkingMessagesSessionRequest_t, m_MessagesSessionRequest);
    STEAM_CALLBACK(SteamCallbacks, MessagesSessionRequestFailed, SteamNetworkingMessagesSessionFailed_t, m_MessagesSessionRequestFailed);
    STEAM_CALLBACK(SteamCallbacks, SteamNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t, m_SteamNetConnectionStatusChanged);

    SteamCallbacks() : // this will register the callbacks using Steam API, using callbacks requires 'SteamUpdate' to be called
                       m_MicroTxnAuthorizationResponse(this, &SteamCallbacks::MicroTxnAuthorizationResponse),
                       m_PersonaStateChange(this, &SteamCallbacks::PersonaStateChange),
                       m_AvatarImageLoaded(this, &SteamCallbacks::AvatarImageLoaded),
                       m_GameOverlayActivated(this, &SteamCallbacks::GameOverlayActivated),
                       m_LobbyCreated(this, &SteamCallbacks::LobbyCreated),
                       m_LobbyEnter(this, &SteamCallbacks::LobbyEnter),
                       m_LobbyInvite(this, &SteamCallbacks::LobbyInvite),
                       m_GameLobbyJoinRequested(this, &SteamCallbacks::GameLobbyJoinRequested),
                       m_LobbyChatUpdate(this, &SteamCallbacks::LobbyChatUpdate),
                       m_LobbyChatMsg(this, &SteamCallbacks::LobbyChatMsg),
                       m_LobbyDataUpdate(this, &SteamCallbacks::LobbyDataUpdate),
                       m_P2PSessionRequest(this, &SteamCallbacks::P2PSessionRequest),
                       m_CreateItemResult(this, &SteamCallbacks::CreateItemResult),
                       m_MessagesSessionRequest(this, &SteamCallbacks::MessagesSessionRequest),
                       m_MessagesSessionRequestFailed(this, &SteamCallbacks::MessagesSessionRequestFailed),
                       m_SteamNetConnectionStatusChanged(this, &SteamCallbacks::SteamNetConnectionStatusChanged) {}
} SC;

void SteamCallbacks::MicroTxnAuthorizationResponse(MicroTxnAuthorizationResponse_t *response) {
    if (response && response->m_unAppID == Steam.appID()) {
        if (response->m_bAuthorized) {
            Memt<HTTPParam> p;
            p.New().set("key", Steam._web_api_key, HTTP_POST);
            p.New().set("appid", response->m_unAppID, HTTP_POST);
            p.New().set("orderid", (ULong)response->m_ulOrderID, HTTP_POST);

            SteamWorks::Operation &op = Steam._operations.New();
            op.type = FINALIZE;
            op.order_id = response->m_ulOrderID;
            op.create(STEAM_API "/FinalizeTxn/V0001/", p);
        } else if (auto callback = Steam.order_callback)
            callback(SteamWorks::ORDER_USER_CANCELED, response->m_ulOrderID, S, null, null);
    }
}
void SteamCallbacks::PersonaStateChange(PersonaStateChange_t *change) {
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_Name == k_EPersonaChangeName);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_Status == k_EPersonaChangeStatus);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_ComeOnline == k_EPersonaChangeComeOnline);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_GoneOffline == k_EPersonaChangeGoneOffline);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_GamePlayed == k_EPersonaChangeGamePlayed);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_GameServer == k_EPersonaChangeGameServer);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_Avatar == k_EPersonaChangeAvatar);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_JoinedSource == k_EPersonaChangeJoinedSource);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_LeftSource == k_EPersonaChangeLeftSource);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_RelationshipChanged == k_EPersonaChangeRelationshipChanged);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_NameFirstSet == k_EPersonaChangeNameFirstSet);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_Broadcast == k_EPersonaChangeBroadcast);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_Nickname == k_EPersonaChangeNickname);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_SteamLevel == k_EPersonaChangeSteamLevel);
    ASSERT((int)SteamWorks::FRIEND_STATE_CHANGE_RichPresence == k_EPersonaChangeRichPresence);
    if (change)
        if (auto callback = Steam.friend_state_changed)
            callback(change->m_ulSteamID, change->m_nChangeFlags);
}
void SteamCallbacks::AvatarImageLoaded(AvatarImageLoaded_t *avatar) // called when 'GetLargeFriendAvatar' was requested but not yet available, simply notify user with callback that new avatar is available for a user
{
    if (avatar)
        if (auto callback = Steam.friend_state_changed)
            callback(avatar->m_steamID.ConvertToUint64(), SteamWorks::FRIEND_STATE_CHANGE_Avatar);
}
void SteamCallbacks::GameOverlayActivated(GameOverlayActivated_t *data) {
    if (data)
        Steam._overlay_visible = data->m_bActive;
}

void SteamCallbacks::LobbyCreated(LobbyCreated_t *data) {
    if (auto callback = Steam.OnLobbyCreated)
        callback((SteamWorks::E_LOBBYRESULT)data->m_eResult, data->m_ulSteamIDLobby);
}

void SteamCallbacks::LobbyEnter(LobbyEnter_t *data) {
    if (auto callback = Steam.OnLobbyEntered)
        callback(data->m_ulSteamIDLobby, data->m_rgfChatPermissions, data->m_bLocked, data->m_EChatRoomEnterResponse);
}

void SteamCallbacks::LobbyInvite(LobbyInvite_t *data) {
    if (auto callback = Steam.OnLobbyInvite)
        callback(data->m_ulSteamIDUser, data->m_ulSteamIDLobby, data->m_ulGameID);
}

void SteamCallbacks::GameLobbyJoinRequested(GameLobbyJoinRequested_t *data) {
    if (auto callback = Steam.OnLobbyJoined)
        callback(data->m_steamIDLobby.ConvertToUint64(), data->m_steamIDFriend.ConvertToUint64());
}

void SteamCallbacks::LobbyChatUpdate(LobbyChatUpdate_t *data) {
    if (auto callback = Steam.OnLobbyChatUpdate)
        callback(data->m_ulSteamIDLobby, data->m_ulSteamIDUserChanged, data->m_ulSteamIDMakingChange, data->m_rgfChatMemberStateChange);
}

void SteamCallbacks::LobbyChatMsg(LobbyChatMsg_t *data) {
    if (auto callback = Steam.OnLobbyChatMsg)
        callback(data->m_ulSteamIDLobby, data->m_ulSteamIDUser, data->m_eChatEntryType, data->m_iChatID);
}

void SteamCallbacks::LobbyDataUpdate(LobbyDataUpdate_t *data) {
    if (auto callback = Steam.OnLobbyDataUpdate)
        callback(data->m_ulSteamIDLobby, data->m_ulSteamIDMember, data->m_bSuccess);
}

void SteamCallbacks::P2PSessionRequest(P2PSessionRequest_t *data) {
    if (auto callback = Steam.OnP2PSessionRequest)
        callback(data->m_steamIDRemote.ConvertToUint64());
}

void SteamCallbacks::CreateItemResult(CreateItemResult_t *data) {
    if (auto callback = Steam.OnSteamWorkshopItemReceived)
        callback((SteamWorks::E_WORKSHOPRESULT)data->m_eResult, data->m_nPublishedFileId, data->m_bUserNeedsToAcceptWorkshopLegalAgreement);
}

void SteamCallbacks::MessagesSessionRequest(SteamNetworkingMessagesSessionRequest_t *data) {
    if (auto callback = Steam.OnSteamNetworkingMessagesSessionRequest)
        callback(data->m_identityRemote.GetSteamID64());
}

void SteamCallbacks::MessagesSessionRequestFailed(SteamNetworkingMessagesSessionFailed_t *data) {
    if (auto callback = Steam.OnSteamNetworkingMessagesSessionRequestFailed)
        callback(data->m_info.m_eEndReason);
}

void SteamCallbacks::SteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *data) {
    if (auto callback = Steam.OnSteamNetConnectionStatusChanged) {
        if (data->m_eOldState == k_ESteamNetworkingConnectionState_None && data->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting)
            callback();
        // callback(data);
    }
}

#endif
SteamWorks Steam;
/******************************************************************************/
static void SteamUpdate() { Steam.update(); }
static ULong SteamUnixTime() {
#if SUPPORT_STEAM
    if (ISteamUtils *i = SteamUtils())
        return i->GetServerRealTime();
#endif
    return 0;
}
static Bool SetDateFromYYYYMMDD(DateTime &dt, C Str &text) {
    dt.zero();
    if (text.length() == 8) {
        REPA(text)
        if (FlagOff(CharFlagFast(text[i]), CHARF_DIG))
            goto invalid;
        dt.year = CharInt(text[0]) * 1000 + CharInt(text[1]) * 100 + CharInt(text[2]) * 10 + CharInt(text[3]) * 1;
        dt.month = CharInt(text[4]) * 10 + CharInt(text[5]) * 1;
        dt.day = CharInt(text[6]) * 10 + CharInt(text[7]) * 1;
        return true;
    }
invalid:
    return false;
}
/******************************************************************************/
#if 0 // following function was used to check that MONTH formula will work on every possible date
void Test()
{
   DateTime last_payment; last_payment.zero();
   last_payment.day=1;
   last_payment.month=1;
   REP(365*400) // include leap-years
   {
      for(Int frequency=1; frequency<=12; frequency++)
      {
         DateTime cur_time=last_payment; REP(frequency)cur_time.incMonth();
         ULong unix_time=cur_time.seconds();
         ULong expiry=last_payment.seconds();
         switch(2)
         {
            case 0: expiry+=     ( frequency*             1+1        )*86400; break; // 86400=number of seconds in a day (60*60*24), +1 for 1 day tolerance
            case 1: expiry+=     ( frequency*            14+1        )*86400; break; // 86400=number of seconds in a day (60*60*24), +1 for 1 day tolerance
            case 2: expiry+=Trunc((frequency*DAYS_PER_MONTH+2.21f)*24)* 3600; break; //  3600=number of seconds in hour  (60*60   ), +1 for 1 day tolerance
            case 3: expiry+=     ( frequency*           365+1        )*86400; break; // 86400=number of seconds in a day (60*60*24), +1 for 1 day tolerance
         }
         if(!(unix_time<=expiry))
         {
            Flt a=unix_time/86400.0f,
                b=expiry   /86400.0f;
            int z=0; // FAIL
         }
      }
      last_payment.incDay();
   }
   int z=0; // OK
}
#endif
Bool SteamWorks::Subscription::valid() C {
    if (last_payment.valid()) {
        ULong unix_time = SteamUnixTime();
        if (!unix_time)
            return false; // we need to know Steam time
        ULong expiry = last_payment.seconds1970();
        switch (period) {
        case DAY:
            expiry += (frequency * 1 + 1) * 86400;
            break; // 86400=number of seconds in a  day  (60*60*24), +1 for 1 day tolerance
        case WEEK:
            expiry += (frequency * 14 + 1) * 86400;
            break; // 86400=number of seconds in a  day  (60*60*24), +1 for 1 day tolerance
        case MONTH:
            expiry += Trunc((frequency * DAYS_PER_MONTH + 2.21f) * 24) * 3600;
            break; //  3600=number of seconds in an hour (60*60   ),          day tolerance was calculated using 'Test' function above
        case YEAR:
            expiry += (frequency * 365 + 1) * 86400;
            break; // 86400=number of seconds in a  day  (60*60*24), +1 for 1 day tolerance
        }
        return unix_time <= expiry;
    }
    return false;
}
/******************************************************************************/
SteamWorks &SteamWorks::webApiKey(C Str8 &web_api_key) {
    T._web_api_key = web_api_key;
    return T;
}

#if SUPPORT_STEAM
extern void (*SteamSetTime)();
static void _SteamSetTime() // this is called at least once
{
    if (ISteamUtils *i = SteamUtils()) {
        Steam._start_time_s = i->GetSecondsSinceAppActive(); // 'GetSecondsSinceAppActive' is since Steam Client was started and not this application
    }
    if (App._callbacks.initialized())
        App.includeFuncCall(SteamUpdate); // include only if initialized, as this may be called before 'App' constructor and it would crash
}
#endif

Bool SteamWorks::init() {
#if SUPPORT_STEAM
    if (!_initialized) {
        _initialized = SteamAPI_Init();
        // we can't add 'SteamUpdate' to the 'App._callbacks' here because this may be called in the constructor and 'App._callbacks' may not be initialized yet, instead this will be done in '_SteamSetTime' which is called here and during Time creation where 'App._callbacks' will be initialized
        SteamSetTime = _SteamSetTime; // set function pointer, so when 'Time' gets initialized, it will call this function too
        _SteamSetTime();              // call anyway, in case 'Time' was already initialized
    }
#endif
    return _initialized;
}
void SteamWorks::shut() {
#if SUPPORT_STEAM
    if (_initialized) {
        SteamAPI_Shutdown();
        _initialized = false;
    }
#endif
}
/******************************************************************************/
// GET
/******************************************************************************/
UInt SteamWorks::appID() C {
#if SUPPORT_STEAM
    if (ISteamUtils *i = SteamUtils())
        return i->GetAppID();
#endif
    return 0;
}
ULong SteamWorks::userID() C {
#if SUPPORT_STEAM
    if (ISteamUser *i = SteamUser())
        return i->GetSteamID().ConvertToUint64();
#endif
    return 0;
}
Str SteamWorks::userName() C {
#if SUPPORT_STEAM
    if (ISteamFriends *i = SteamFriends())
        return FromUTF8(i->GetPersonaName());
#endif
    return S;
}
#if SUPPORT_STEAM
static SteamWorks::USER_STATUS SteamStatus(EPersonaState state) {
    ASSERT(k_EPersonaStateMax == 8);
    switch (state) {
    case k_EPersonaStateInvisible:
        return SteamWorks::STATUS_OFFLINE;

    case k_EPersonaStateOffline:
        return SteamWorks::STATUS_OFFLINE;

    case k_EPersonaStateLookingToTrade:
    case k_EPersonaStateLookingToPlay:
    case k_EPersonaStateOnline:
        return SteamWorks::STATUS_ONLINE;

    case k_EPersonaStateBusy:
    case k_EPersonaStateAway:
    case k_EPersonaStateSnooze:
        return SteamWorks::STATUS_AWAY;
    }
    return SteamWorks::STATUS_UNKNOWN;
}
#endif
SteamWorks::USER_STATUS SteamWorks::userStatus() C {
#if SUPPORT_STEAM
    if (ISteamFriends *i = SteamFriends())
        return SteamStatus(i->GetPersonaState());
#endif
    return STATUS_UNKNOWN;
}
Bool SteamWorks::userAvatar(Image &image) C { return userAvatar(userID(), image); }
#if SUPPORT_STEAM
struct SteamLanguage {
    LANG_TYPE lang;
    CChar8 *code;
} static const SteamLanguages[] =
    {
        // https://partner.steamgames.com/doc/store/localization#supported_languages
        {(LANG_TYPE)LANG_ARABIC, "arabic"},
        {(LANG_TYPE)LANG_BULGARIAN, "bulgarian"},
        {(LANG_TYPE)LANG_CHINESE, "schinese"}, // simplified
        {(LANG_TYPE)LANG_CHINESE, "tchinese"}, // traditional
        {(LANG_TYPE)LANG_CZECH, "czech"},
        {(LANG_TYPE)LANG_DANISH, "danish"},
        {(LANG_TYPE)LANG_DUTCH, "dutch"},
        {(LANG_TYPE)LANG_ENGLISH, "english"},
        {(LANG_TYPE)LANG_FINNISH, "finnish"},
        {(LANG_TYPE)LANG_FRENCH, "french"},
        {(LANG_TYPE)LANG_GERMAN, "german"},
        {(LANG_TYPE)LANG_GREEK, "greek"},
        {(LANG_TYPE)LANG_HUNGARIAN, "hungarian"},
        {(LANG_TYPE)LANG_ITALIAN, "italian"},
        {(LANG_TYPE)LANG_JAPANESE, "japanese"},
        {(LANG_TYPE)LANG_KOREAN, "korean"},  // keep in case Steam will change to this
        {(LANG_TYPE)LANG_KOREAN, "koreana"}, // yes "koreana" is correct according to Steam Docs and tests
        {(LANG_TYPE)LANG_NORWEGIAN, "norwegian"},
        {(LANG_TYPE)LANG_POLISH, "polish"},
        {(LANG_TYPE)LANG_PORTUGUESE, "portuguese"},
        {(LANG_TYPE)LANG_PORTUGUESE, "brazilian"}, // Portuguese-Brazil
        {(LANG_TYPE)LANG_ROMANIAN, "romanian"},
        {(LANG_TYPE)LANG_RUSSIAN, "russian"},
        {(LANG_TYPE)LANG_SPANISH, "spanish"},
        {(LANG_TYPE)LANG_SPANISH, "latam"}, // Spanish-Latin America
        {(LANG_TYPE)LANG_SWEDISH, "swedish"},
        {(LANG_TYPE)LANG_THAI, "thai"},
        {(LANG_TYPE)LANG_TURKISH, "turkish"},
        {(LANG_TYPE)LANG_UKRAINIAN, "ukrainian"},
        {(LANG_TYPE)LANG_VIETNAMESE, "vietnamese"},
};
#endif
LANG_TYPE SteamWorks::appLanguage() C {
#if SUPPORT_STEAM
    if (ISteamApps *i = SteamApps())
        if (auto text = i->GetCurrentGameLanguage())
            REPA(SteamLanguages)
    if (Equal(text, SteamLanguages[i].code))
        return SteamLanguages[i].lang;
#endif
    return LANG_NONE;
}
COUNTRY SteamWorks::country() C {
#if SUPPORT_STEAM
    if (ISteamUtils *i = SteamUtils())
        return CountryCode2(i->GetIPCountry());
#endif
    return COUNTRY_NONE;
}
DateTime SteamWorks::date() C {
    DateTime dt;
#if SUPPORT_STEAM
    if (ULong unix_time = SteamUnixTime())
        dt.from1970s(unix_time);
    else
#endif
        dt.zero();
    return dt;
}
UInt SteamWorks::curTimeS() C {
#if SUPPORT_STEAM
    if (ISteamUtils *i = SteamUtils())
        return i->GetSecondsSinceAppActive() - _start_time_s;
#endif
    return 0;
}
Bool SteamWorks::overlayAvailable() C {
#if SUPPORT_STEAM
    if (ISteamUtils *i = SteamUtils())
        return i->IsOverlayEnabled();
#endif
    return false;
}
Bool SteamWorks::overlayAvailableMsgBox() C {
#if SUPPORT_STEAM
    if (overlayAvailable())
        return true;
    Gui.msgBox("Enable Steam Game Overlay", "Making purchases requires \"Steam Game Overlay\" to be enabled.\nPlease go to \"Steam Client Settings \\ In-Game\", click on the \"Enable the Steam Overlay while in-game\" and restart this app.");
#endif
    return false;
}
Bool SteamWorks::drmExit() C {
#if SUPPORT_STEAM
    UInt app_id = appID();
    if (!app_id || SteamAPI_RestartAppIfNecessary(app_id)) {
        App.flag |= APP_EXIT_IMMEDIATELY;
        return true;
    }
#endif
    return false;
}
#if 0
SteamWorks::RESULT SteamWorks::getUserInfo()
{
   if(!init()           )return STEAM_NOT_INITIALIZED;
   if(!_web_api_key.is())return STEAM_NOT_SETUP;
   if(!userID()         )return USER_ID_UNAVAILABLE;

   Memt<HTTPParam> p;
   p.New().set("steamid", Steam.userID());
   d.create(STEAM_API "/GetUserInfo/V0001/", p);
}
#endif
/******************************************************************************/
// FRIENDS
/******************************************************************************/
Bool SteamWorks::getFriends(MemPtr<ULong> friend_ids) C {
#if SUPPORT_STEAM
    if (ISteamFriends *f = SteamFriends()) {
        UInt flags = k_EFriendFlagImmediate;
        friend_ids.setNum(f->GetFriendCount(flags));
        REPAO(friend_ids) = f->GetFriendByIndex(i, flags).ConvertToUint64();
        return true;
    }
#endif
    friend_ids.clear();
    return false;
}
Str SteamWorks::userName(ULong user_id) C {
#if SUPPORT_STEAM
    if (ISteamFriends *i = SteamFriends())
        return FromUTF8(i->GetFriendPersonaName((uint64)user_id));
#endif
    return S;
}
SteamWorks::USER_STATUS SteamWorks::userStatus(ULong user_id) C {
#if SUPPORT_STEAM
    if (ISteamFriends *i = SteamFriends())
        return SteamStatus(i->GetFriendPersonaState((uint64)user_id));
#endif
    return STATUS_UNKNOWN;
}
Bool SteamWorks::userAvatar(ULong user_id, Image &image) C {
#if SUPPORT_STEAM
    if (ISteamFriends *i = SteamFriends()) {
        Int image_id = i->GetLargeFriendAvatar((uint64)user_id); // request large avatar first, if not available then it will return -1 and 'AvatarImageLoaded' will be called where we notify of new avatar available
        if (image_id < 0)
            image_id = i->GetMediumFriendAvatar((uint64)user_id); // if not yet loaded, then get medium size, this should always be available
        if (!image_id) {
            image.del();
            return true;
        } // according to steam headers, 0 means no image set
        if (ISteamUtils *i = SteamUtils()) {
            UInt width = 0, height = 0;
            if (i->GetImageSize(image_id, &width, &height))
                if (image.createSoft(width, height, 1, IMAGE_R8G8B8A8_SRGB))
                    if (i->GetImageRGBA(image_id, image.data(), image.memUsage()))
                        return true;
        }
    }
#endif
    image.del();
    return false;
}
/******************************************************************************/
// ORDER
/******************************************************************************/
SteamWorks::RESULT SteamWorks::purchase(ULong order_id, Int item_id, C Str &item_name, Int cost_in_cents, C Str8 &currency) {
    if (!init())
        return STEAM_NOT_INITIALIZED;
    if (!_web_api_key.is())
        return STEAM_NOT_SETUP;
    if (!appID())
        return APP_ID_UNAVAILABLE;
    if (!userID())
        return USER_ID_UNAVAILABLE;

    Memt<HTTPParam> p;
    p.New().set("key", _web_api_key, HTTP_POST);
    p.New().set("steamid", userID(), HTTP_POST);
    p.New().set("appid", appID(), HTTP_POST);
    p.New().set("orderid", order_id, HTTP_POST);
    p.New().set("itemcount", 1, HTTP_POST);
    p.New().set("language", "EN", HTTP_POST);
    p.New().set("currency", currency, HTTP_POST);
    p.New().set("itemid[0]", item_id, HTTP_POST);
    p.New().set("qty[0]", 1, HTTP_POST);
    p.New().set("amount[0]", cost_in_cents, HTTP_POST);
    p.New().set("description[0]", item_name, HTTP_POST);

    SteamWorks::Operation &op = Steam._operations.New();
    op.type = PURCHASE;
    op.order_id = order_id;
    op.create(STEAM_API "/InitTxn/V0002/", p);
    return WAITING;
}
SteamWorks::RESULT SteamWorks::subscribe(ULong order_id, Int item_id, C Str &item_name, Int cost_in_cents, PERIOD period, Int frequency, C Str8 &currency) {
    if (!init())
        return STEAM_NOT_INITIALIZED;
    if (!_web_api_key.is())
        return STEAM_NOT_SETUP;
    if (!appID())
        return APP_ID_UNAVAILABLE;
    if (!userID())
        return USER_ID_UNAVAILABLE;
    ULong unix_time = SteamUnixTime();
    if (!unix_time)
        return STEAM_NOT_INITIALIZED; // use date from Steam in case the OS has an incorrect date set

    Memt<HTTPParam> p;
    p.New().set("key", _web_api_key, HTTP_POST);
    p.New().set("steamid", userID(), HTTP_POST);
    p.New().set("appid", appID(), HTTP_POST);
    p.New().set("orderid", order_id, HTTP_POST);
    p.New().set("itemcount", 1, HTTP_POST);
    p.New().set("language", "EN", HTTP_POST);
    p.New().set("currency", currency, HTTP_POST);
    p.New().set("itemid[0]", item_id, HTTP_POST);
    p.New().set("qty[0]", 1, HTTP_POST);
    p.New().set("amount[0]", cost_in_cents, HTTP_POST);
    p.New().set("description[0]", item_name, HTTP_POST);
    p.New().set("billingtype[0]", "steam", HTTP_POST);
    p.New().set("frequency[0]", frequency, HTTP_POST);
    p.New().set("recurringamt[0]", cost_in_cents, HTTP_POST);
    switch (period) {
    case DAY:
        p.New().set("period[0]", "day", HTTP_POST);
        unix_time += frequency * (86400);
        break; // 86400=number of seconds in a day (60*60*24)
    case WEEK:
        p.New().set("period[0]", "week", HTTP_POST);
        unix_time += frequency * (86400 * 14);
        break; // 86400=number of seconds in a day (60*60*24)
    case MONTH:
        p.New().set("period[0]", "month", HTTP_POST);
        unix_time += frequency * (86400 * 30);
        break; // 86400=number of seconds in a day (60*60*24), use 30 as avg number of days in a month
    case YEAR:
        p.New().set("period[0]", "year", HTTP_POST);
        unix_time += frequency * (86400 * 365);
        break; // 86400=number of seconds in a day (60*60*24)
    default:
        return ORDER_REQUEST_FAILED;
    }
    DateTime date;
    date.from1970s(unix_time);
    p.New().set("startdate[0]", TextInt(date.year, 4) + TextInt(date.month, 2) + TextInt(date.day, 2), HTTP_POST); // format is YYYYMMDD

    SteamWorks::Operation &op = Steam._operations.New();
    op.type = PURCHASE;
    op.order_id = order_id;
    op.create(STEAM_API "/InitTxn/V0002/", p);
    return WAITING;
}
SteamWorks::RESULT SteamWorks::purchaseState(ULong order_id) {
    if (!init())
        return STEAM_NOT_INITIALIZED;
    if (!_web_api_key.is())
        return STEAM_NOT_SETUP;
    if (!appID())
        return APP_ID_UNAVAILABLE;

    Memt<HTTPParam> p;
    p.New().set("key", _web_api_key);
    p.New().set("appid", appID());
    p.New().set("orderid", order_id);

    SteamWorks::Operation &op = Steam._operations.New();
    op.type = QUERY_PURCHASE;
    op.order_id = order_id;
    op.create(STEAM_API "/QueryTxn/V0001/", p);
    return WAITING;
}
SteamWorks::RESULT SteamWorks::subscriptionState() {
    if (!init())
        return STEAM_NOT_INITIALIZED;
    if (!_web_api_key.is())
        return STEAM_NOT_SETUP;
    if (!appID())
        return APP_ID_UNAVAILABLE;
    if (!userID())
        return USER_ID_UNAVAILABLE;

    Memt<HTTPParam> p;
    p.New().set("key", _web_api_key);
    p.New().set("steamid", userID());
    p.New().set("appid", appID());

    SteamWorks::Operation &op = Steam._operations.New();
    op.type = QUERY_SUBSCRIPTION;
    op.order_id = 0;
    op.create(STEAM_API "/GetUserAgreementInfo/V0001/", p);
    return WAITING;
}
void SteamWorks::update() {
    REPA(_operations) {
        Operation &op = _operations[i];
        if (op.state() == DWNL_ERROR || op.state() == DWNL_DONE) // have to process errors too, to report them into callback, try to read downloaded data from errors too, as they may contain some information
        {
            RESULT res;
            switch (op.type) {
            case PURCHASE:
                res = ORDER_REQUEST_FAILED;
                break;
            case FINALIZE:
                res = ORDER_FINALIZE_FAILED;
                break;
            case QUERY_PURCHASE:
                res = PURCHASE_STATUS_FAILED;
                break;
            case QUERY_SUBSCRIPTION:
                res = SUBSCRIPTION_STATUS_FAILED;
                break;
            default:
                res = STEAM_NOT_INITIALIZED;
                break;
            }
            Str error_message;
            Purchase purchase, *purchase_ptr = null;
            Subscription subscription, *subscription_ptr = null;
            TextData data;
            FileText f;
            data.loadJSON(f.readMem(op.data(), op.done()));
#if DEBUG && 0
            data.save(FFirst("d:/", "txt"));
#endif
            if (data.nodes.elms() == 1)
                if (TextNode *response = data.nodes[0].findNode("response")) {
                    TextNode *result = response->findNode("result");
                    if (result && result->asText() == "OK") {
                        TextNode *params = response->findNode("params");
                        if (op.type == QUERY_SUBSCRIPTION) // subscription status
                        {
                            res = SUBSCRIPTION_STATUS_OK;
                            if (params)
                                if (TextNode *agreements = params->findNode("agreements"))
                                    if (agreements->nodes.elms() == 1) // if this is null or it has no elements, then there are no subscriptions
                                    {
                                        subscription_ptr = &subscription;
                                        TextNode &agreement = agreements->nodes[0];
                                        if (TextNode *p = agreement.findNode("status"))
                                            subscription.active = (p->asText() == "Active");
                                        if (TextNode *p = agreement.findNode("itemid"))
                                            subscription.item_id = p->asInt();
                                        if (TextNode *p = agreement.findNode("frequency"))
                                            subscription.frequency = p->asInt();
                                        if (TextNode *p = agreement.findNode("recurringamt"))
                                            subscription.cost_in_cents = p->asInt();
                                        if (TextNode *p = agreement.findNode("currency"))
                                            subscription.currency = p->asText();
                                        if (TextNode *p = agreement.findNode("timecreated"))
                                            SetDateFromYYYYMMDD(subscription.created, p->asText());
                                        if (TextNode *p = agreement.findNode("nextpayment"))
                                            SetDateFromYYYYMMDD(subscription.next_payment, p->asText());
                                        if (TextNode *p = agreement.findNode("lastpayment"))
                                            SetDateFromYYYYMMDD(subscription.last_payment, p->asText());
                                        if (!subscription.last_payment.valid())
                                            subscription.last_payment = subscription.created; // "lastpayment" value can be "NIL", so in case that happens or it wasn't specified at all, then use the 'subscription.created'
                                        if (TextNode *p = agreement.findNode("period")) {
                                            if (p->asText() == "day")
                                                subscription.period = DAY;
                                            else if (p->asText() == "week")
                                                subscription.period = WEEK;
                                            else if (p->asText() == "month")
                                                subscription.period = MONTH;
                                            else if (p->asText() == "year")
                                                subscription.period = YEAR;
                                        }
                                    }
                        } else if (params)
                            if (TextNode *order_id = params->findNode("orderid"))
                                if (order_id->asULong() == op.order_id) {
                                    switch (op.type) {
                                    case PURCHASE:
                                        res = ORDER_REQUEST_OK;
                                        break;
                                    case FINALIZE:
                                        res = ORDER_FINALIZE_OK;
                                        break;
                                    case QUERY_PURCHASE: {
                                        res = PURCHASE_STATUS_OK;
                                        purchase_ptr = &purchase;
                                        if (TextNode *p = params->findNode("status"))
                                            purchase.finalized = (p->asText() == "Succeeded");
                                        if (TextNode *p = params->findNode("steamid"))
                                            purchase.user_id = p->asULong();
                                        if (TextNode *p = params->findNode("currency"))
                                            purchase.currency = p->asText();
                                        if (TextNode *p = params->findNode("country"))
                                            purchase.country = p->asText();
                                        if (TextNode *p = params->findNode("time"))
                                            purchase.date.fromText(p->value.replace('T', ' ').replace('Z', '\0'));
                                        if (TextNode *items = params->findNode("items"))
                                            if (items->nodes.elms() == 1) {
                                                TextNode &item = items->nodes[0];
                                                if (TextNode *itemid = item.findNode("itemid"))
                                                    purchase.item_id = itemid->asInt();
                                                if (TextNode *amount = item.findNode("amount"))
                                                    purchase.cost_in_cents = amount->asInt();
                                            }
                                    } break;
                                    }
                                }
                    } else if (TextNode *error = response->findNode("error")) {
                        if (TextNode *errordesc = error->findNode("errordesc"))
                            Swap(error_message, errordesc->value);
                        /*if(TextNode *errorcode=error->findNode("errorcode"))switch(errorcode->asInt())
                          {
                             case
                          }*/
                    }
                    goto finish;
                }
            f.rewind();
            f.getAll(error_message);
        finish:
            if (auto callback = order_callback)
                callback(res, op.order_id, error_message, purchase_ptr, subscription_ptr);
            _operations.removeValid(i);
        }
    }
#if SUPPORT_STEAM
    SteamAPI_RunCallbacks();
#endif
    App._callbacks.add(SteamUpdate);
}
/******************************************************************************/
// CLOUD SAVES
/******************************************************************************/
Long SteamWorks::cloudAvailableSize() C {
#if SUPPORT_STEAM
    if (ISteamRemoteStorage *i = SteamRemoteStorage()) {
        uint64 total, available;
        if (i->GetQuota(&total, &available))
            return available;
    }
#endif
    return -1;
}
Long SteamWorks::cloudTotalSize() C {
#if SUPPORT_STEAM
    if (ISteamRemoteStorage *i = SteamRemoteStorage()) {
        uint64 total, available;
        if (i->GetQuota(&total, &available))
            return total;
    }
#endif
    return -1;
}

Bool SteamWorks::cloudDel(C Str &file_name) {
#if SUPPORT_STEAM
    if (ISteamRemoteStorage *i = SteamRemoteStorage())
        return i->FileDelete(UTF8(file_name));
#endif
    return false;
}
Bool SteamWorks::cloudExists(C Str &file_name) {
#if SUPPORT_STEAM
    if (ISteamRemoteStorage *i = SteamRemoteStorage())
        return i->FileExists(UTF8(file_name));
#endif
    return false;
}
Long SteamWorks::cloudSize(C Str &file_name) {
#if SUPPORT_STEAM
    if (ISteamRemoteStorage *i = SteamRemoteStorage())
        return i->GetFileSize(UTF8(file_name));
#endif
    return 0;
}
DateTime SteamWorks::cloudTimeUTC(C Str &file_name) {
    DateTime dt;
#if SUPPORT_STEAM
    if (ISteamRemoteStorage *i = SteamRemoteStorage())
        if (int64 timestamp = i->GetFileTimestamp(UTF8(file_name)))
            return dt.from1970s(timestamp);
#endif
    return dt.zero();
}

Bool SteamWorks::cloudSave(C Str &file_name, File &f, Cipher *cipher) {
#if SUPPORT_STEAM
    if (file_name.is())
        if (ISteamRemoteStorage *i = SteamRemoteStorage()) {
            if (f._type == FILE_MEM && !f._cipher && !cipher) {
                Bool ok = i->FileWrite(UTF8(file_name), f.memFast(), f.left());
                f.pos(f.size());
                return ok;
            }

            Memt<Byte> data;
            data.setNum(f.left());
            if (f.getFast(data.data(), data.elms())) {
                if (cipher)
                    cipher->encrypt(data.data(), data.data(), data.elms(), 0);
                return i->FileWrite(UTF8(file_name), data.data(), data.elms());
            }
        }
#endif
    return false;
}
Bool SteamWorks::cloudLoad(C Str &file_name, File &f, Bool memory, Cipher *cipher) {
#if SUPPORT_STEAM
    if (file_name.is())
        if (ISteamRemoteStorage *i = SteamRemoteStorage()) {
            Str8 name = UTF8(file_name);
            Long size = i->GetFileSize(name);
            if (size > 0 || size == 0 && i->FileExists(name)) {
                if (memory)
                    f.writeMemFixed(size);
                if (size > 0) {
                    Memt<Byte> temp;
                    Ptr data;
                    Bool direct = (f._type == FILE_MEM && f._writable && f.left() >= size);
                    data = (direct ? f.memFast() : temp.setNum(size).data());
#if CLOUD_WORKAROUND
                    REPD(attempt, 1000)
#endif
                    {
                        auto read = i->FileRead(name, data, size);
                        if (read == size) {
                            if (cipher)
                                cipher->decrypt(data, data, size, 0);
                            if (direct) {
                                if (f._cipher)
                                    f._cipher->encrypt(data, data, size, f.posCipher());
                                return f.skip(size);
                            } else {
                                return f.put(data, size);
                            }
#if CLOUD_WORKAROUND
                            break;
#endif
                        }
#if CLOUD_WORKAROUND
                        // LogN(S+"failed:"+attempt+" "+read+'/'+size);
                        Time.wait(1);
#endif
                    }
                } else
                    return true; // "size==0"
            }
        }
#endif
    return false;
}

Bool SteamWorks::cloudSave(C Str &file_name, CPtr data, Int size) {
#if SUPPORT_STEAM
    if (file_name.is())
        if (ISteamRemoteStorage *i = SteamRemoteStorage())
            return i->FileWrite(UTF8(file_name), data, size);
#endif
    return false;
}
Bool SteamWorks::cloudLoad(C Str &file_name, Ptr data, Int size) {
#if SUPPORT_STEAM
    if (file_name.is())
        if (ISteamRemoteStorage *i = SteamRemoteStorage())
            return i->FileRead(UTF8(file_name), data, size) == size;
#endif
    return false;
}

Int SteamWorks::cloudFiles() C {
#if SUPPORT_STEAM
    if (ISteamRemoteStorage *i = SteamRemoteStorage())
        return i->GetFileCount();
#endif
    return 0;
}
Bool SteamWorks::cloudFile(Int file_index, Str &name, Long &size) C {
#if SUPPORT_STEAM
    if (file_index >= 0)
        if (ISteamRemoteStorage *i = SteamRemoteStorage()) {
            int32 file_size;
            CChar8 *file_name = i->GetFileNameAndSize(file_index, &file_size);
            if (Is(file_name)) {
                name = FromUTF8(file_name);
                size = file_size;
                return true;
            }
        }
#endif
    name.clear();
    size = 0;
    return false;
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
