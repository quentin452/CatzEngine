/******************************************************************************/
#include "stdafx.h"
#include "../Platforms/iOS/iOS.h"
namespace EE{
/******************************************************************************/
Facebook FB;
#if SUPPORT_FACEBOOK && IOS
enum GET_FLAG
{
   GET_ME     =1<<0,
   GET_FRIENDS=1<<1,
};
static Byte Get;
static void GetMe()
{
#if 0 // this doesn't support email
   [FBSDKProfile loadCurrentProfileWithCompletion:^(FBSDKProfile *profile, NSError *error)
   {
      if(profile)
      {
         FB._me.id  =[profile.userID longLongValue];
         FB._me.name= profile.name;
      }
   }];
#else
   if(FBSDKGraphRequest *graph_req=[[FBSDKGraphRequest alloc] initWithGraphPath:@"me" parameters:@{@"fields":@"id,name,email"}])
   {
      [graph_req startWithCompletionHandler:^(FBSDKGraphRequestConnection *connection, id result, NSError *error)
      {
         if(NSDictionary *dict=result)
         {
            // this is called on the main thread, so no need for 'SyncLock'
            if(NSString *s=[dict objectForKey:@"id"   ])FB._me.id   =[s longLongValue];
            if(NSString *s=[dict objectForKey:@"name" ])FB._me.name =s;
            if(NSString *s=[dict objectForKey:@"email"])FB._me.email=s;
         }
      }];
      [graph_req release];
   }
#endif
}
static void GetFriends()
{
   if(FBSDKGraphRequest *graph_req=[[FBSDKGraphRequest alloc] initWithGraphPath:@"me/friends" parameters:@{@"fields":@"id,name"} HTTPMethod:@"GET"])
   {
      [graph_req startWithCompletionHandler:^(FBSDKGraphRequestConnection *connection, id result, NSError *error)
      {
         if(NSArray *friends=[result objectForKey:@"data"])
         {
            // this is called on the main thread, so no need for 'SyncLock'
            FB._friends.setNum([friends count]); REPA(FB._friends) // go from the end as we may remove users below
            {
               if(NSDictionary *dict=[friends objectAtIndex:i])
               {
                  Facebook::User &user=FB._friends[i];
                  if(NSString *s=[dict objectForKey:@"id"  ])user.id  =[s longLongValue];else user.id=0;
                  if(NSString *s=[dict objectForKey:@"name"])user.name= s               ;else user.name.clear();
               }else FB._friends.remove(i, true);
            }
         }
      }];
      [graph_req release];
   }
}
static void FBUpdate()
{
   if(FB.loggedIn())
   {
      if(Get&GET_ME     ){FlagDisable(Get, GET_ME     ); GetMe     ();}
      if(Get&GET_FRIENDS){FlagDisable(Get, GET_FRIENDS); GetFriends();}
   }
}
#endif
/******************************************************************************/
Bool Facebook::loggedIn()C
{
#if SUPPORT_FACEBOOK
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID facebookLoggedIn=jni.staticFunc(ActivityClass, "facebookLoggedIn", "()Z"))
      return jni->CallStaticBooleanMethod(ActivityClass, facebookLoggedIn);
#elif IOS
   return [FBSDKAccessToken currentAccessToken]!=null;
#endif
#endif
   return false;
}
Facebook& Facebook::logIn()
{
#if SUPPORT_FACEBOOK
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
   if(JMethodID facebookLogIn=jni.func(ActivityClass, "facebookLogIn", "()V"))
      jni->CallVoidMethod(Activity, facebookLogIn);
#elif IOS
   if(FBSDKLoginManager *login=[[FBSDKLoginManager alloc] init])
   {
      [login logInWithReadPermissions: @[@"public_profile", @"email", @"user_friends"] fromViewController:ViewController handler:^(FBSDKLoginManagerLoginResult *result, NSError *error)
      {
         if(error)
         {
            // error
         }else
         if(result && result.isCancelled)
         {
            // canceled
         }else
         {
            // logged in
            FBUpdate();
         }
     }];
     [login release];
   }
#endif
#endif
   return T;
}
Facebook& Facebook::logOut()
{
#if SUPPORT_FACEBOOK
#if ANDROID
   JNI jni;
   if(jni && ActivityClass)
   if(JMethodID facebookLogOut=jni.staticFunc(ActivityClass, "facebookLogOut", "()V"))
      jni->CallStaticVoidMethod(ActivityClass, facebookLogOut);
#elif IOS
   #if 0
      if(FBSDKLoginManager *login=[[FBSDKLoginManager alloc] init])
      {
         [login logOut ];
         [login release];
      }
   #else
      [FBSDKAccessToken setCurrentAccessToken:nil];
      [FBSDKProfile     setCurrentProfile    :nil];
   #endif
#endif
#endif
  _me     .clear();
  _friends.clear();
   return T;
}
Facebook& Facebook::getMe()
{
#if SUPPORT_FACEBOOK
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
   if(JMethodID facebookGetMe=jni.func(ActivityClass, "facebookGetMe", "()V"))
      jni->CallVoidMethod(Activity, facebookGetMe);
#elif IOS
   if(loggedIn())GetMe();else{FlagEnable(Get, GET_ME); logIn();}
#endif
#endif
   return T;
}
Facebook& Facebook::getFriends()
{
#if SUPPORT_FACEBOOK
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
   if(JMethodID facebookGetFriends=jni.func(ActivityClass, "facebookGetFriends", "()V"))
      jni->CallVoidMethod(Activity, facebookGetFriends);
#elif IOS
   if(loggedIn())GetFriends();else{FlagEnable(Get, GET_FRIENDS); logIn();}
#endif
#endif
   return T;
}
Str Facebook::userImageURL(ULong user_id, Int width, Int height)C
{
   Str  url=S+"http://graph.facebook.com/"+user_id+"/picture";
   Char par='?';
   if(width >0){url+=par; par='&'; url+= "width="; url+=width ;}
   if(height>0){url+=par; par='&'; url+="height="; url+=height;}
   return url;
}
void Facebook::openPage(C Str &page_name, C Str &page_id)
{
#if ANDROID
   if(!page_id.is() || !Explore(S+"fb://page/"+page_id))
#elif IOS
   if(!page_id.is() || !Explore(S+"fb://profile/"+page_id))
#endif
      if(page_name.is())Explore(S+"https://www.facebook.com/"+page_name);else
      if(page_id  .is())Explore(S+"https://www.facebook.com/profile.php?id="+page_id);
}
void Facebook::post(C Str &url, C Str &quote)
{
#if SUPPORT_FACEBOOK
#if ANDROID
   JNI jni;
   if(jni && ActivityClass && Activity)
   if(JMethodID facebookPost=jni.func(ActivityClass, "facebookPost", "(Ljava/lang/String;Ljava/lang/String;)V"))
   if(JString   j_url  =JString(jni, url  ))
   if(JString   j_quote=JString(jni, quote))
      jni->CallVoidMethod(Activity, facebookPost, j_url(), j_quote());
#elif IOS
   if(FBSDKShareLinkContent *content=[[FBSDKShareLinkContent alloc] init])
   {
      NSURLAuto ns_url=url; content.contentURL=ns_url; // !! keep 'ns_url' as temp to be deleted later, in case 'content.contentURL' is a weak reference reusing its or NSString's memory
                            content.quote     =NSStringAuto(quote);
      if(FBSDKShareDialog *dialog=[[FBSDKShareDialog alloc] init])
      {
         dialog.fromViewController=ViewController;
         dialog.shareContent=content;
         dialog.delegate=GetAppDelegate();
         [dialog show];
         [dialog release];
      }
      [content release]; // release 'content' after 'dialog' finished, in case it has a weak reference
   }
#endif
#endif
}
/******************************************************************************/
} // namespace EE
/******************************************************************************/
#if SUPPORT_FACEBOOK && ANDROID
static      Facebook::UserEmail Me;
static Mems<Facebook::User >    Friends;
static Bool                     HasMe, HasFriends; // these are needed because 'Swap' is used, to make sure we don't swap 2 times, in case 'Update' is running and waiting for lock, while we receive new data and include one more 'Update'. This way already running 'Update' will get new data, and the next queued 'Update' will swap to old data.
static Facebook::RESULT         Result;
static void UpdateMe     () {SyncLocker locker(JavaLock); if(HasMe     ){Swap(FB._me     , Me     ); HasMe     =false;}}
static void UpdateFriends() {SyncLocker locker(JavaLock); if(HasFriends){Swap(FB._friends, Friends); HasFriends=false;}}
static void CallCallback () {if(void (*callback)(Facebook::RESULT result)=FB.callback)callback(Result);}
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_facebookMe(JNIEnv *env, jclass clazz, jstring id, jstring name, jstring email)
{
   JNI jni(env);
   {
      SyncLocker locker(JavaLock);
      Me.id   =TextULong(jni(id   ));
      Me.name =          jni(name ) ;
      Me.email=          jni(email) ;
      HasMe   =true;
   }
   App.includeFuncCall(UpdateMe);
}
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_facebookFriends(JNIEnv *env, jclass clazz, jobject ids, jobject names)
{
   JNI jni(env);
   Mems<Facebook::User> friends;
   if(JClass ArrayList=JClass(jni, ids))
   if(JMethodID size=jni.func(ArrayList, "size", "()I"))
   if(JMethodID get =jni.func(ArrayList, "get" , "(I)Ljava/lang/Object;"))
   {
      Int ids_elms=jni->CallIntMethod(ids  , size),
        names_elms=jni->CallIntMethod(names, size);
      if( ids_elms==names_elms)
      {
         friends.setNum(ids_elms);
         REPA(friends)
            if(JString id  =JString(jni, jni->CallObjectMethod(ids  , get, jint(i))))
            if(JString name=JString(jni, jni->CallObjectMethod(names, get, jint(i))))
         {  
            Facebook::User &f=friends[i];
            f.id  =TextULong(id  .str());
            f.name=          name.str() ;
         }
      }
   }
   {
      SyncLocker locker(JavaLock);
      Swap(Friends, friends);
        HasFriends=true;
   }
   App.includeFuncCall(UpdateFriends);
}
extern "C" JNIEXPORT void JNICALL Java_com_esenthel_Native_facebookPost(JNIEnv *env, jclass clazz, jint result)
{
   if(FB.callback)
   {
      Result=(Facebook::RESULT)result;
      App.includeFuncCall(CallCallback);
   }
}
#endif
/******************************************************************************/
