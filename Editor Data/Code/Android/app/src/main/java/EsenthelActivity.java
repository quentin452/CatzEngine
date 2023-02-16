/******************************************************************************/
package com.esenthel.project;
import android.accounts.Account;
import android.accounts.AccountManager;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Application;
import android.app.NativeActivity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.TaskStackBuilder;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.DialogInterface;
import android.content.DialogInterface.OnShowListener;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ProviderInfo;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Insets;
import android.graphics.Point;
import android.graphics.Rect;
import android.hardware.input.InputManager;
import android.location.Location;
import android.location.LocationListener;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.media.MediaRecorder.AudioSource;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.IBinder;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.StatFs;
import android.os.Vibrator;
import android.os.VibrationEffect;
import android.preference.PreferenceManager;
import android.provider.OpenableColumns;
import android.provider.Settings;
import android.provider.Settings.Secure;
import android.text.Editable;
import android.text.InputType;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.Log;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.InputDevice;
import android.view.inputmethod.InputMethodManager;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.webkit.MimeTypeMap;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

/*CHARTBOOST*\
import com.chartboost.sdk.*;
import com.chartboost.sdk.Model.CBError.CBImpressionError;
/*CHARTBOOST*/
/*FACEBOOK*\
import com.facebook.AccessToken;
import com.facebook.CallbackManager;
import com.facebook.FacebookCallback;
import com.facebook.FacebookException;
import com.facebook.GraphRequest;
import com.facebook.GraphResponse;
import com.facebook.login.LoginManager;
import com.facebook.login.LoginResult;
import com.facebook.share.model.ShareLinkContent;
import com.facebook.share.Sharer;
import com.facebook.share.widget.ShareDialog;
/*FACEBOOK*/
/*ADMOB*\
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdListener;
/*ADMOB*/
/*LICENSE_KEY*\
import com.google.android.vending.licensing.AESObfuscator;
import com.google.android.vending.licensing.LicenseChecker;
import com.google.android.vending.licensing.LicenseCheckerCallback;
import com.google.android.vending.licensing.Policy;
import com.google.android.vending.licensing.ServerManagedPolicy;
/*LICENSE_KEY*/
import java.io.IOException;
import java.io.File;
import java.io.FileNotFoundException;
import java.lang.UnsupportedOperationException;
import java.math.BigInteger;
import java.net.NetworkInterface;
import java.nio.ByteBuffer;
import java.security.KeyFactory;
import java.security.MessageDigest;
import java.security.PublicKey;
import java.security.Signature;
import java.security.spec.X509EncodedKeySpec;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import org.json.JSONObject;
import org.json.JSONArray;
/******************************************************************************/
public class EsenthelActivity extends NativeActivity
{
   public static class EsenthelLocationListener implements LocationListener
   {
      public boolean gps;

      public EsenthelLocationListener(boolean gps) {this.gps=gps;}

      @Override public final void onLocationChanged (Location   location) {com.esenthel.Native.location(gps, location);}
      @Override public final void onProviderDisabled(String providerName) {}
      @Override public final void onProviderEnabled (String providerName) {}
      @Override public final void onStatusChanged   (String providerName, int status, Bundle extras) {}
   }
   public static class DetectForceKill extends Service // !! if changing name of this class then update AndroidManifest.xml !!
   {
      @Override public final IBinder onBind        (Intent intent) {return null;}
      @Override public final int     onStartCommand(Intent intent, int flags, int startId) {return START_NOT_STICKY;}
    //@Override public final void    onDestroy     () {super.onDestroy();}
      @Override public final void    onTaskRemoved (Intent rootIntent) {removeNotifications(); super.onTaskRemoved(rootIntent);} // this is called on force kill
   }

   // !! these must be equal to 'AWAKE_MODE' !!
   public static final byte AWAKE_OFF   =0;
   public static final byte AWAKE_SYSTEM=1;
   public static final byte AWAKE_SCREEN=2;

   // !! these must be equal to 'SYSTEM_BAR' !!
   public static final byte SYSTEM_BAR_HIDDEN =0;
   public static final byte SYSTEM_BAR_OVERLAY=1;
   public static final byte SYSTEM_BAR_VISIBLE=2;

   // !! these must be equal to 'PlatformStore.RESULT' !!
   public static final byte RES_OK                 =-1; // this is not a 'PlatformStore.RESULT' but a temp value
   public static final byte RES_PURCHASED          =0;
   public static final byte RES_CONSUMED           =1;
   public static final byte RES_REFUND             =2;
   public static final byte RES_WAITING            =3;
   public static final byte RES_USER_CANCELED      =4;
   public static final byte RES_SERVICE_CANCELED   =5;
   public static final byte RES_SERVICE_UNAVAILABLE=6;
   public static final byte RES_ITEM_UNAVAILABLE   =7;
   public static final byte RES_ALREADY_OWNED      =8;
   public static final byte RES_NOT_OWNED          =9;
   public static final byte RES_VERIFICATION_FAIL  =10;
   public static final byte RES_UNKNOWN            =11;
   public static final byte RES_REFRESHED_ITEMS    =12;
   public static final byte RES_REFRESHED_PURCHASES=13;

   // !! these must be equal to 'PlatformStore.LICENSE_TEST_RESULT' !!
   public static final byte LTR_NONE   =0;
   public static final byte LTR_WAITING=1;
   public static final byte LTR_OK     =2;
   public static final byte LTR_FAIL   =3;
   public static final byte LTR_RETRY  =4;
   public static final byte LTR_ERROR  =5;

   // !! these must be equal to 'Facebook.RESULT' !!
   public static final byte POST_ERROR=0;
   public static final byte POST_CANCEL=1;
   public static final byte POST_SUCCESS=2;

   public static final byte REQUEST_CODE_IAB=0;
   public static final byte REQUEST_CODE_FILE_PICKER=1;

   // variables
   static EsenthelActivity     activity;
   static Application          application;
   static Context              context;
   static Intent               background_intent;
   static String               background_text;

   static Handler              handler;
   static PublicKey            license_key;
   static String               android_id;
   static Vibrator             vibrator;
          EditText             edit_text;
          TextWatcher          text_watcher;
   static WakeLock             wake_lock;
   static WifiLock             wifi_lock;
   static TextView             text_view;
   static InputManager.InputDeviceListener         input_device_listener;
   static ViewTreeObserver.OnGlobalLayoutListener global_layout_listener;

   public static final void    log   (String s) {Log.e("Titan", s);}
   public static final boolean Is    (String s) {return !TextUtils.isEmpty(s);} // this supports null 's' too
   public static final String  GetExt(String path) {final int last_dot=path.lastIndexOf('.'); return (last_dot>=0) ? path.substring(last_dot+1) : null;}

   static
   {
      /*LOAD_LIBRARIES*/
   }
   /******************************************************************************/
   // MAIN
   /******************************************************************************/
   @Override public final void onSaveInstanceState(Bundle savedInstanceState)
   {
      super.onSaveInstanceState(savedInstanceState);
   }
   @Override public final void onRestoreInstanceState(Bundle savedInstanceState)
   {
      super.onRestoreInstanceState(savedInstanceState);
   /*ADMOB*\
      if(Is(inters_id))adCreate(inters_id, AD_INTERSTITIAL);
      if(Is(banner_id))adCreate(banner_id, banner_type    );
   /*ADMOB*/
   }
   @Override public final void onNewIntent(Intent intent)
   {
      super.onNewIntent(intent);
      checkNotification(intent, true);
   }
   @Override public final void onBackPressed()
   {
   /*CHARTBOOST*\
      if(!Chartboost.onBackPressed()) // this will detect if there's an interstitial and close it, otherwise process below:
   /*CHARTBOOST*/
      {
         super.onBackPressed();
      }
   }
   @Override public final void onResume()
   {
                             super.onResume();
   /*ADMOB*\
      if(ad_view  !=null)ad_view  .  resume();
   /*ADMOB*/
   /*CHARTBOOST*\
                        Chartboost.onResume(this);
   /*CHARTBOOST*/
   }
   @Override public final void onPause()
   {
   /*ADMOB*\
      if(ad_view  !=null)ad_view  .  pause();
   /*ADMOB*/
                             super.onPause();
   /*CHARTBOOST*\
                        Chartboost.onPause(this);
   /*CHARTBOOST*/
   }
   @Override public final void onStart()
   {
           super.onStart();
   /*CHARTBOOST*\
      Chartboost.onStart(this);
   /*CHARTBOOST*/
   }
   @Override public final void onStop()
   {
           super.onStop();
   /*CHARTBOOST*\
      Chartboost.onStop(this);
   /*CHARTBOOST*/
   }
   @Override public final void onDestroy()
   {
      if(global_layout_listener!=null)
      {
         View root_view=getWindow().getDecorView().getRootView();
         root_view.getViewTreeObserver().removeOnGlobalLayoutListener(global_layout_listener);
         global_layout_listener=null;
      }
   /*ADMOB*\
      bannerHide();
      adViewDel();
   /*ADMOB*/
                             super.onDestroy();
   /*CHARTBOOST*\
                        Chartboost.onDestroy(this);
   /*CHARTBOOST*/
    //shutIAB();
   /*LICENSE_KEY*\
      shutLicenseTest();
   /*LICENSE_KEY*/
      context=application; activity=null; // when activity becomes unavailable, then use application context because we always need one
   }
   @Override public final void onCreate(Bundle savedInstanceState)
   {
      application=getApplication(); context=activity=this; // use activity when available, as it's more powerful - https://stackoverflow.com/questions/4128589/difference-between-activity-context-and-application-context

      if(handler==null)handler=new Handler()
      {
         @Override public final void handleMessage(Message message)
         {
            super.handleMessage(message);  
            String  title=message.getData().getString ("title"),
                    text =message.getData().getString ("text" );
            boolean exit =message.getData().getBoolean("exit" );
            if(title!=null && text!=null)
            {
               AlertDialog dialog=new AlertDialog.Builder(context).setTitle(title).setMessage(text).setNeutralButton("OK", null).show();
               if(exit && dialog!=null)
                  dialog.setOnDismissListener(new DialogInterface.OnDismissListener()
                  {
                     @Override public final void onDismiss(final DialogInterface dialog) {com.esenthel.Native.closedError();} // can't call "activity.finish();" because it may suspend the app, while we need to force close it
                  });
            }
         }
      };
      if(android_id==null)android_id=Secure.getString(getContentResolver(), Secure.ANDROID_ID);
      if(wake_lock==null)
      {
         PowerManager power_manager=(PowerManager)getSystemService(POWER_SERVICE); if(power_manager!=null)
         {
            wake_lock=power_manager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, ""); // will crash if null is used
            if(wake_lock!=null)wake_lock.setReferenceCounted(false); // disable ref counting so one 'release' will be immediate
         }
      }
      if(wifi_lock==null)
      {
         WifiManager wifi_manager=(WifiManager)getSystemService(WIFI_SERVICE); if(wifi_manager!=null)
         {
            wifi_lock=wifi_manager.createWifiLock(WifiManager.WIFI_MODE_FULL_HIGH_PERF, ""); // will crash if null is used
            if(wifi_lock!=null)wifi_lock.setReferenceCounted(false); // disable ref counting so one 'release' will be immediate
         }
      }
      if(vibrator==null)
      {
         vibrator=(Vibrator)getSystemService(Context.VIBRATOR_SERVICE); if(vibrator!=null)
         {
          //log("hasVibrator"+vibrator.hasVibrator());
          //log("hasAmplitudeControl"+vibrator.hasAmplitudeControl());
            if(!vibrator.hasVibrator())vibrator=null;
         }
      }
      if(input_device_listener==null)
      {
         InputManager input_manager=(InputManager)getSystemService(Context.INPUT_SERVICE); if(input_manager!=null)
         {
            input_device_listener=new InputManager.InputDeviceListener()
            {
               @Override public void onInputDeviceChanged(int deviceId) {}
               @Override public void onInputDeviceRemoved(int deviceId) {com.esenthel.Native.deviceRemoved(deviceId);}
               @Override public void onInputDeviceAdded  (int deviceId)
               {
                  InputDevice device=InputDevice.getDevice(deviceId);
                  if(device!=null)
                     if((device.getSources()&((InputDevice.SOURCE_DPAD|InputDevice.SOURCE_GAMEPAD|InputDevice.SOURCE_JOYSTICK) & ~InputDevice.SOURCE_CLASS_BUTTON))!=0) // disable 'SOURCE_CLASS_BUTTON' because SOURCE_KEYBOARD has it
                        if(!device.isVirtual())
                           com.esenthel.Native.deviceAdded(deviceId);
               }
            };
            input_manager.registerInputDeviceListener(input_device_listener, null);
            for(int deviceId:InputDevice.getDeviceIds())input_device_listener.onInputDeviceAdded(deviceId);
         }
      }

      super.onCreate(savedInstanceState);

      startService(new Intent(this, DetectForceKill.class)); // start service that detects force kill

      // detect showing nav bar by the user manually in order to hide it automatically
      Window window=activity.getWindow(); if(window!=null)
      {
         if(Build.VERSION.SDK_INT>=28)window.getAttributes().layoutInDisplayCutoutMode=WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES; // allow drawing into cutout areas
         View view=window.getDecorView(); if(view!=null)
         {
            /* Auto-hide bars after user revealed them - this is not needed because View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY and BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE do the same
				view.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener()
            {
               @Override public void onSystemUiVisibilityChange(int visibility)
               {
                  if(navBar()==SYSTEM_BAR_HIDDEN && (visibility&View.SYSTEM_UI_FLAG_HIDE_NAVIGATION)==0) // if want to hide, but user disabled
                  {
                     handler.postDelayed(new Runnable()
                     {
                        @Override public void run() {systemBars(system_bars);} // restore
                     }, 2000); // after 2 seconds
                  }
               }
            });*/
            View root_view=view.getRootView(); if(root_view!=null)
            {
               global_layout_listener=new ViewTreeObserver.OnGlobalLayoutListener()
               {
                  @Override public void onGlobalLayout()
                  {
                     com.esenthel.Native.resized();
                  }
               };
               root_view.getViewTreeObserver().addOnGlobalLayoutListener(global_layout_listener);

             /*use 'OnGlobalLayoutListener' because this is out of sync (values are outdated)
               root_view.setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener()
               {
                  @Override public WindowInsets onApplyWindowInsets(View v, WindowInsets insets)
                  {
                     //WindowInsetsCompat insets_compat=WindowInsetsCompat.toWindowInsetsCompat(insets, v);
                     return v.onApplyWindowInsets(insets);
                  }
               });*/
            }
         }
      }

    //initIAB();
   /*LICENSE_KEY*\
      initLicenseTest();
   /*LICENSE_KEY*/
   /*ADMOB*\
      initAdMob();
   /*ADMOB*/
   /*CHARTBOOST*\
      initChartboost();
   /*CHARTBOOST*/
   /*FACEBOOK*\
      initFB();
   /*FACEBOOK*/
      initNotification();

    /*log("android_id "+android_id);
      log("BOARD "+Build.BOARD);
      log("BOOTLOADER "+Build.BOOTLOADER);
      log("BRAND "+Build.BRAND);
      log("DEVICE "+Build.DEVICE);
      log("DISPLAY "+Build.DISPLAY);
      log("FINGERPRINT "+Build.FINGERPRINT);
      log("HARDWARE "+Build.HARDWARE);
      log("HOST "+Build.HOST);
      log("ID "+Build.ID);
      log("MANUFACTURER "+Build.MANUFACTURER);
      log("MODEL "+Build.MODEL);
      log("PRODUCT "+Build.PRODUCT);
      log("SERIAL "+Build.SERIAL);
      log("TAGS "+Build.TAGS);
      log("TIME "+Build.TIME);
      log("TYPE "+Build.TYPE);
      log("USER "+Build.USER);*/
   }
   @Override protected final void onActivityResult(int requestCode, int resultCode, Intent data)
   {
      super.onActivityResult(requestCode, resultCode, data);
   /*FACEBOOK*\
      if(callbackManager!=null)callbackManager.onActivityResult(requestCode, resultCode, data);
   /*FACEBOOK*/
      switch(requestCode)
      {
         /*case REQUEST_CODE_IAB:
         {
            int    responseCode=-1;
            long   date=0;
            String purchaseData=null, sku=null, custom_data=null, token=null, signature=null;
            if(data!=null)
            {
               // data.getExtras()
               responseCode=getResponseCodeFromIntent(data);
               purchaseData=data.getStringExtra(RESPONSE_INAPP_PURCHASE_DATA);
               signature   =data.getStringExtra(RESPONSE_INAPP_SIGNATURE);
               if(purchaseData!=null)
               {
                  try
                  {
                     JSONObject o=new JSONObject(purchaseData);
                   //o.optString("orderId");
                   //o.optString("packageName");
                 sku=o.optString("productId");
                date=o.optLong("purchaseTime");
                   //o.optInt("purchaseState");
         custom_data=o.optString("developerPayload");
               token=o.optString("token", o.optString("purchaseToken"));
                  }catch(Exception exception){} // failed to parse data
               }
            }

            int result=billingToEE(responseCode);
            if( result==RES_OK     )result=((resultCode==Activity.RESULT_OK && sku!=null) ? (verifyPurchase(purchaseData, signature) ? RES_PURCHASED : RES_VERIFICATION_FAIL) : RES_UNKNOWN);
            if( result==RES_UNKNOWN)if(resultCode==Activity.RESULT_CANCELED)result=RES_SERVICE_CANCELED;
          //log("responseCode:"+responseCode+", resultCode:"+resultCode+", result:"+result);

            com.esenthel.Native.purchased(result, sku, custom_data, token, date);
         }break;*/

         case REQUEST_CODE_FILE_PICKER:
         {
            if(resultCode==RESULT_OK && data!=null)
            {
               ContentResolver resolver=getContentResolver(); if(resolver!=null)
               {
                  android.content.ClipData clipData=data.getClipData();
                  if(clipData!=null)for(int i=0; i<clipData.getItemCount(); i++)
                  {
                     android.content.ClipData.Item item=clipData.getItemAt(i); if(item!=null)
                     {
                        Uri uri=item.getUri(); if(uri!=null)try
                        {
                           ParcelFileDescriptor pfd=resolver.openFileDescriptor(uri, "r"); if(pfd!=null){com.esenthel.Native.drop(pfd.getFd()); pfd.close();} // must be closed manually
                        }catch(Exception e){}
                     }
                  }else
                  {
                     Uri uri=data.getData(); if(uri!=null)try
                     {
                        ParcelFileDescriptor pfd=resolver.openFileDescriptor(uri, "r"); if(pfd!=null){com.esenthel.Native.drop(pfd.getFd()); pfd.close();} // must be closed manually
                     }catch(Exception e){}
                  }
               }
            }
         }break;
      }
   }

   @Override public final void onConfigurationChanged(Configuration newConfig)
   {
      super.onConfigurationChanged(newConfig);
   /*ADMOB*\
      adPos(banner_x, banner_y); // reset banner position in new screen size/orientation
   /*ADMOB*/
   }
   public static final void messageBox(String title, String text, boolean exit)
   {
      if(handler!=null)
      {
         Message message=handler.obtainMessage(); if(message!=null)
         {
            Bundle bundle=new Bundle(); if(bundle!=null)
            {
                       bundle.putString ("title", title);
                       bundle.putString ("text" , text );
               if(exit)bundle.putBoolean("exit" , exit );
               message.setData(bundle);
               handler.sendMessage(message);
            }
         }
      }
   }
   public static final void toast(final String text)
   {
      Handler handler=new Handler(Looper.getMainLooper());
      handler.post(new Runnable()
      {
         @Override public final void run()
         {
            Toast.makeText(context, text, Toast.LENGTH_LONG).show();
         }
      });
   }
   public static final void overlay(final String text)
   {
      if(activity!=null)activity.runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(Is(text))
            {
               if(text_view==null)
               {
                  text_view=new TextView      (activity);
                  text_view.setTextColor      (Color.WHITE);
                  text_view.setBackgroundColor(Color.BLACK);
                  text_view.setGravity        (Gravity.CENTER_VERTICAL | Gravity.CENTER_HORIZONTAL);

                  WindowManager.LayoutParams layout_params=new WindowManager.LayoutParams(
                     ViewGroup.LayoutParams.MATCH_PARENT, // fill entire screen
                     ViewGroup.LayoutParams.MATCH_PARENT,
    	               android.view.WindowManager.LayoutParams.TYPE_APPLICATION_PANEL,
                     WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL,
                     android.graphics.PixelFormat.TRANSPARENT);
                  layout_params.gravity = Gravity.CENTER | Gravity.CENTER;
                  activity.getWindowManager().addView(text_view, layout_params);
               }
               text_view.setText(text);
            }else
            {
               if(text_view!=null)
               {
                  activity.getWindowManager().removeView(text_view);
                  text_view=null;
               }
            }
         }
      });
   }
   public static final void stayAwake(final int mode)
   {
      if(wake_lock!=null)
      {
         if(mode==AWAKE_SYSTEM)wake_lock.acquire();
         else                  wake_lock.release();
      }
      if(wifi_lock!=null)
      {
         if(mode==AWAKE_SYSTEM)wifi_lock.acquire();
         else                  wifi_lock.release();
      }
      if(activity!=null)activity.runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            Window window=activity.getWindow();
            if(window!=null)window.setFlags((mode==AWAKE_SCREEN) ? WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON : 0, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
         }
      });
   }
   public static final boolean openAppSettings()
   {
      Intent intent=new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
      intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
      intent.setData(Uri.fromParts("package", "com.esenthel.project", null));
      try{context.startActivity(intent); return true;}catch(Exception exception){}
      return false;
   }
   public static final String GetMime(String ext) {return MimeTypeMap.getSingleton().getMimeTypeFromExtension(ext);}

   public static List<String> SharingPaths;
   public final void addSharingPath(String path)
   {
      if(Is(path))
      {
         if(path.charAt(path.length()-1)!='/')path+='/';
         SharingPaths.add(path);
      }
   }
   public final void setSharingPaths(String sd_card)
   {
      SharingPaths=new ArrayList<String>();
      addSharingPath(                       getExternalFilesDir    (null).getAbsolutePath()); // app data public
      addSharingPath(android.os.Environment.getExternalStorageDirectory().getAbsolutePath()); //          public
      addSharingPath(                                                               sd_card); // SD card
    //log("SharingPaths: "+SharingPaths.size()); for(int i=0; i<SharingPaths.size(); i++)log(SharingPaths.get(i));
   }
   public static class FileProvider extends android.content.ContentProvider // !! if changing name of this class then update AndroidManifest.xml !!
   {
      private static final String[] COLUMNS={OpenableColumns.DISPLAY_NAME, OpenableColumns.SIZE};

      @Override public boolean onCreate() {return true;}
      @Override public void attachInfo(Context context, ProviderInfo info)
      {
         super.attachInfo(context, info);
         // security checks
         if( info.exported           )throw new SecurityException("Provider must not be exported");
         if(!info.grantUriPermissions)throw new SecurityException("Provider must grant uri permissions");
      }
      @Override public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
      {
         final File file=getFileForUri(uri);
         if(projection==null)projection=COLUMNS;
         String[] cols  =new String[projection.length];
         Object[] values=new Object[projection.length];
         int i=0;
         for(String col:projection)
         {
            if(OpenableColumns.DISPLAY_NAME.equals(col))
            {
               cols  [i  ]=OpenableColumns.DISPLAY_NAME;
               values[i++]=file.getName();
            }else
            if(OpenableColumns.SIZE.equals(col))
            {
               cols  [i  ]=OpenableColumns.SIZE;
               values[i++]=file.length();
            }
         }
         cols  =copyOf(cols  , i);
         values=copyOf(values, i);
         final MatrixCursor cursor=new MatrixCursor(cols, 1); cursor.addRow(values); return cursor;
      }
      @Override public String               getType (Uri uri) {final String mime=GetMime(GetExt(uri.getPath())); return (mime!=null) ? mime : "application/octet-stream";}
      @Override public Uri                  insert  (Uri uri, ContentValues values                                          ) {throw new UnsupportedOperationException("No external inserts");}
      @Override public int                  update  (Uri uri, ContentValues values, String selection, String[] selectionArgs) {throw new UnsupportedOperationException("No external updates");}
      @Override public int                  delete  (Uri uri,                       String selection, String[] selectionArgs) {return getFileForUri(uri).delete() ? 1 : 0;}
      @Override public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException                       {return ParcelFileDescriptor.open(getFileForUri(uri), modeToMode(mode));}

      private static final int modeToMode(String mode)
      {
         if("r"  .equals(mode)                     )return ParcelFileDescriptor.MODE_READ_ONLY;
         if("w"  .equals(mode) || "wt".equals(mode))return ParcelFileDescriptor.MODE_WRITE_ONLY|ParcelFileDescriptor.MODE_CREATE|ParcelFileDescriptor.MODE_TRUNCATE;
         if("wa" .equals(mode)                     )return ParcelFileDescriptor.MODE_WRITE_ONLY|ParcelFileDescriptor.MODE_CREATE|ParcelFileDescriptor.MODE_APPEND;
         if("rw" .equals(mode)                     )return ParcelFileDescriptor.MODE_READ_WRITE|ParcelFileDescriptor.MODE_CREATE;
         if("rwt".equals(mode)                     )return ParcelFileDescriptor.MODE_READ_WRITE|ParcelFileDescriptor.MODE_CREATE|ParcelFileDescriptor.MODE_TRUNCATE;
         throw new IllegalArgumentException("Invalid mode: "+mode);
      }
      private static final String[] copyOf(String[] original, int newLength) {final String[] result=new String[newLength]; System.arraycopy(original, 0, result, 0, newLength); return result;}
      private static final Object[] copyOf(Object[] original, int newLength) {final Object[] result=new Object[newLength]; System.arraycopy(original, 0, result, 0, newLength); return result;}

      public static final boolean pathOK(String path)
      {
         for(int i=0; i<SharingPaths.size(); i++)if(path.startsWith(SharingPaths.get(i)))return true;
         return false;
      }
      public static final File getFileForUri(Uri uri)
      {
         String path=uri.getPath();
         File file=new File(path);
         // code below is needed to calculate the full normalized path, which is required for precise path verification
         try{file=file.getCanonicalFile();}catch(IOException e){throw new IllegalArgumentException("Failed to resolve canonical path for "+file);}
         path=file.getPath();
         if(!pathOK(path))throw new SecurityException("Resolved path jumped beyond configured root");
         if(!file.exists())throw new IllegalArgumentException("Unable to find file for uri "+uri);
         return file;
      }
      public static final Uri getUriForPath(String path) {return new Uri.Builder().scheme("content").authority("com.esenthel.project.fileprovider").path(path).build();}
   }
   public static final boolean run(String path)
   {
      try{
         Intent intent=new Intent(Intent.ACTION_VIEW);
         if(path.charAt(0)=='/') // file
         {
            File file=new File(path);
            if(!FileProvider.pathOK(path) // without this check 'run' will return true even if later accessing path will fail
            || !file        .exists(    ))return false; // or file doesn't exist
            String mime=GetMime(GetExt(path)); if(!Is(mime))mime="*/*";
            intent.setDataAndType((Build.VERSION.SDK_INT>=24) ? FileProvider.getUriForPath(path) : Uri.fromFile(file), mime); // file provider needed starting Android Nougat, don't use below, as it may not work on older versions
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION|Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
         }else // HTTP, HTTPS, FTP, etc
         {
            intent.setData(Uri.parse(path));
         }
         context.startActivity(intent);
         return true;
      }catch(Exception exception){} return false;
   }

   public static final long driveSizeFree (String path) {File file=new File(path); if(file!=null && file.exists())return file.getUsableSpace(); return -1;}
   public static final long driveSizeTotal(String path) {File file=new File(path); if(file!=null && file.exists())return file.getTotalSpace (); return -1;}

   public static final String manufacturer() {return Build.MANUFACTURER;}
   public static final String model       () {return Build.MODEL;}
   public static final String androidID   () {return android_id;}
   public static final String userName    ()
   {
      try{
         Cursor cursor=context.getContentResolver().query(android.provider.ContactsContract.Profile.CONTENT_URI, null, null, null, null);
         if(cursor!=null && cursor.moveToFirst())
         {
            int col=cursor.getColumnIndex(android.provider.ContactsContract.Profile.DISPLAY_NAME); if(col>=0)return cursor.getString(col);
         }
      }catch(Exception exception){} // no permission
      return null;
   }
   public static final String userEmail()
   {
      try{
         AccountManager manager=AccountManager.get(context);
         Account[] accounts=manager.getAccountsByType("com.google");
         if(accounts!=null && accounts.length>0)return accounts[0].name;
      }catch(Exception exception){} // no permission
      return null;
   }
   public static final long mac()
   {
      try{
         List<NetworkInterface> nis=Collections.list(NetworkInterface.getNetworkInterfaces());
         for( NetworkInterface ni:nis)if(ni.getName().equalsIgnoreCase("wlan0"))
         {
            byte[] bytes =ni.getHardwareAddress();
            int    length=bytes.length; if(length>8)length=8;
            long   mac   =0; for(int i=0; i<length; i++)mac|=(((long)(bytes[i]&0xFF))<<(i*8));
            return mac;
         }
      }catch(Exception exception){}
      return 0;
   }

   public final int statusBarHeight()
   {
      int res_id=getResources().getIdentifier("status_bar_height", "dimen", "android");
      return (res_id>0) ? getResources().getDimensionPixelSize(res_id) : 0;
   }
   public final int navBarHeight()
   {
      int res_id=getResources().getIdentifier("navigation_bar_height", "dimen", "android");
      return (res_id>0) ? getResources().getDimensionPixelSize(res_id) : 0;
   }

   /*public static final int systemBarsActual() // what we actually have
   {
      int result=0; if(activity!=null)
      {
         Window window=activity.getWindow(); if(window!=null)
         {
            View view=window.getDecorView(); if(view!=null)
            {
					int  v=view.getSystemUiVisibility(), w=window.getAttributes().flags;
					byte status, nav;
               if((v&View.SYSTEM_UI_FLAG_FULLSCREEN                    )!=0
               || (w&WindowManager.LayoutParams.FLAG_FULLSCREEN        )!=0)status=SYSTEM_BAR_HIDDEN ;else // have to check this too
               if((w&WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS)!=0)status=SYSTEM_BAR_OVERLAY;else
                                                                            status=SYSTEM_BAR_VISIBLE;

               if((v&View.SYSTEM_UI_FLAG_HIDE_NAVIGATION                   )!=0)nav=SYSTEM_BAR_HIDDEN ;else
               if((w&WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION)!=0)nav=SYSTEM_BAR_OVERLAY;else
                                                                                nav=SYSTEM_BAR_VISIBLE;
               result=(status|(nav<<2));
            }
         }
      }
      return result;
   }*/
	static byte status_bar=SYSTEM_BAR_HIDDEN, nav_bar=SYSTEM_BAR_VISIBLE; // must match #SystemBar
   public static final void systemBars(byte status, byte nav, boolean status_color, boolean nav_color)
   {
		status_bar=status;
		   nav_bar=nav;
      if(activity!=null)activity.runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            Window window=activity.getWindow(); if(window!=null)
            {
               int v=View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY, w=0;
               switch(status)
               {
                  case SYSTEM_BAR_HIDDEN : v|=View.SYSTEM_UI_FLAG_FULLSCREEN; w|=WindowManager.LayoutParams.FLAG_FULLSCREEN; break;
                  case SYSTEM_BAR_OVERLAY: w|=WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS; break;
               }
               switch(nav)
               {
                  case SYSTEM_BAR_HIDDEN : v|=View.SYSTEM_UI_FLAG_HIDE_NAVIGATION; break;
                  case SYSTEM_BAR_OVERLAY: w|=WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION; break;
               }
					if(!status_color){v|=View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR    ; if(status==SYSTEM_BAR_OVERLAY)w|=WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS;} // this is for dark status bar on light background, can change color only with SYSTEM_BAR_OVERLAY & FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS, however this forces FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS for both status and nav
					if(!   nav_color){v|=View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR; if(nav   ==SYSTEM_BAR_OVERLAY)w|=WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS;} // this is for dark nav    bar on light background, can change color only with SYSTEM_BAR_OVERLAY & FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS, however this forces FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS for both status and nav

               View view=window.getDecorView(); if(view!=null)view.setSystemUiVisibility(v);
               window.setFlags(w, WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS|WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION|WindowManager.LayoutParams.FLAG_FULLSCREEN|WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);

               /* Alternative:
               int w=0;
               View view=window.getDecorView(); if(view!=null)
               {
                  WindowInsetsControllerCompat wic=WindowCompat.getInsetsController(window, view); if(wic!=null)
                  {
                     wic.setSystemBarsBehavior(WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                     if(status==SYSTEM_BAR_HIDDEN)wic.hide(WindowInsetsCompat.Type.    statusBars());else wic.show(WindowInsetsCompat.Type.    statusBars());
                     if(nav   ==SYSTEM_BAR_HIDDEN)wic.hide(WindowInsetsCompat.Type.navigationBars());else wic.show(WindowInsetsCompat.Type.navigationBars());
                  }
               }
               if(status==SYSTEM_BAR_OVERLAY)w|=WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS;
               if(nav   ==SYSTEM_BAR_OVERLAY)w|=WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION;
					need to update for status_color, nav_color
               window.setFlags(w, WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS|WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
               */
            }
         }
      });
   }

   public final float dipToPx(float f) {return f*getResources().getDisplayMetrics().density;}
   public final float pxToDip(float f) {return f/getResources().getDisplayMetrics().density;}
   public final int resW() {return getWindowManager().getDefaultDisplay().getWidth ();}
   public final int resH() {return getWindowManager().getDefaultDisplay().getHeight();}
   public final float refreshRate() {return getWindowManager().getDefaultDisplay().getRefreshRate();}
   public final long screen() {Point size=new Point(); getWindowManager().getDefaultDisplay().getRealSize(size); return size.x | (((long)size.y)<<32);}

   public final void getRects()
   {
    /*int     statusBarHeight=0; if(status    ==SYSTEM_BAR_OVERLAY)    statusBarHeight=statusBarHeight();
      int navigationBarHeight=0; if(navigation==SYSTEM_BAR_OVERLAY)navigationBarHeight=   navBarHeight();*/

      Window window=getWindow(); if(window!=null)
      {
         View view=window.getDecorView(); if(view!=null)
         {
            int l, t, r, b, kb_h=0;
          // insets are opposite of extends, they extend rectangle inwards
          //WindowInsets       insets       =view      .getRootWindowInsets(    ); WindowInsetsCompat insets_compat=WindowInsetsCompat.toWindowInsetsCompat(insets, view);
            WindowInsetsCompat insets_compat=ViewCompat.getRootWindowInsets(view); // use compat because 'View.getRootWindowInsets' is >=23 API, and 'WindowInsets.isVisible' is >=30 API
            if(insets_compat!=null)
            {
               int ime=WindowInsetsCompat.Type.ime(), nav=WindowInsetsCompat.Type.navigationBars(), status=WindowInsetsCompat.Type.statusBars(), cutout=WindowInsetsCompat.Type.displayCutout(), caption=WindowInsetsCompat.Type.captionBar();
               int v=caption|cutout; // this is without Screen Keyboard (ime). Also without status, nav, because of #ImmediateInsets and because:
               // 'getInsets' is buggy for 'status', in landscape mode, status=SYSTEM_BAR_VISIBLE, nav=SYSTEM_BAR_HIDDEN, it may not include status bar, so force it if we know it's visible
               // 'getInsets' is buggy for 'nav'   , it might be included even if it's hidden ("insets_compat.isVisible(nav)" can be true), so check it only if not hidden
               androidx.core.graphics.Insets ins=insets_compat.getInsets(v);
               l=ins.left; t=ins.top; r=ins.right; b=ins.bottom;

               v=0; // #ImmediateInsets, force setting as visible by using 'getInsetsIgnoringVisibility' if we know they're visible. This is so we can get latest 'D.rectUI' before waiting for Android/Java to process requests on other threads.
               if(status_bar!=SYSTEM_BAR_HIDDEN)v|=status;
               if(   nav_bar!=SYSTEM_BAR_HIDDEN)v|=nav   ;
               if(v!=0)
               {
                  ins=insets_compat.getInsetsIgnoringVisibility(v);
                  if(ins.left  >l)l=ins.left  ;
                  if(ins.right >r)r=ins.right ;
                  if(ins.top   >t)t=ins.top   ;
                  if(ins.bottom>b)b=ins.bottom;
               }
             //if(insets_compat.isVisible(ime)) // Kb.visible, check not needed because zeros will be returned below
               {
                  ins=insets_compat.getInsets(ime);
                  kb_h=ins.bottom; // TODO: this assumes screen keyboard is at the bottom
               }
            }else
            {
               Rect  visible =new Rect(); view.getWindowVisibleDisplayFrame(visible ); // in screen coordinates
               int[] view_pos=new int[2]; view.getLocationOnScreen         (view_pos); // in screen coordinates
               int   view_w=view.getWidth(), view_h=view.getHeight(); // same as D.res
               int   view_r=view_pos[0]+view_w, view_b=view_pos[1]+view_h;
             //log("r.top:"+visible.top+", r.bottom:"+visible.bottom+", r.w:"+visible.width()+", r.h:"+visible.height()+", view_pos.x:"+view_pos[0]+", view_pos.y:"+view_pos[1]+", view_w:"+view_w+", view_h:"+view_h);
               l=visible.left-view_pos[0];
               t=visible.top -view_pos[1];
               r=view_r-visible.right;
               b=view_b-visible.bottom;
            }
            com.esenthel.Native.setRects(l, t, r, b, kb_h);
         }
      }
   }

   public static final void vibrate(int intensity, int milliseconds) // intensity=0..255
   {
      if(vibrator!=null)
      {
         if(Build.VERSION.SDK_INT>=26)vibrator.vibrate(VibrationEffect.createOneShot(milliseconds, intensity));else
                                      vibrator.vibrate(milliseconds);
      }
   }
   /******************************************************************************/
   // KEYBOARD
   /******************************************************************************/
   @Override public final boolean dispatchKeyEvent(KeyEvent event)
   {
      if(event!=null)if(event.getAction()!=KeyEvent.ACTION_UP)
      {
         int key_code=event.getKeyCode    (); // use 'getKeyCode' instead of 'getScanCode' to match 'AKeyEvent_getKeyCode' on native C++ side
         int unicode =event.getUnicodeChar(); if(unicode!=0)com.esenthel.Native.key(unicode, key_code);else
         {
            String s=event.getCharacters(); if(s!=null)for(int i=0; i<s.length(); i++)com.esenthel.Native.key(s.codePointAt(i), key_code);
         }
      }
      return super.dispatchKeyEvent(event);
   }
   public final void editTextSetDo(String text, int start, int end, int mode)
   {
      if(edit_text!=null)
      {
         boolean password=((mode&1)!=0),
                   number=((mode&2)!=0),
                    email=((mode&4)!=0),
                      url=((mode&8)!=0);
         edit_text.removeTextChangedListener(text_watcher);
         edit_text.setText(text);
         edit_text.setSelection(start, end);
         edit_text.setInputType(url   ?             InputType.TYPE_TEXT_VARIATION_URI          |InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_FLAG_MULTI_LINE
                             :  email ?             InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS|InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_FLAG_MULTI_LINE
                             : number ? (password ? InputType.   TYPE_NUMBER_VARIATION_PASSWORD|InputType.TYPE_CLASS_NUMBER                                   : InputType.TYPE_CLASS_NUMBER                                  )
                                      : (password ? InputType.     TYPE_TEXT_VARIATION_PASSWORD|InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_FLAG_MULTI_LINE : InputType.TYPE_CLASS_TEXT|InputType.TYPE_TEXT_FLAG_MULTI_LINE)); // TYPE_TEXT_FLAG_MULTI_LINE prevents from Enter making the keyboard disappear (on Google and Samsung keyboards)
         edit_text.addTextChangedListener(text_watcher);
      }
   }
   public final void editTextSet(final String text, final int start, final int end, final int mode)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            editTextSetDo(text, start, end, mode);
         }
      });
   }
   public final void editText(final String text, final int start, final int end, final int mode)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(edit_text==null)
            {
               if(text_watcher==null)text_watcher=new TextWatcher()
               {
                  @Override public final void afterTextChanged(Editable s)
                  {
                     if(s!=null)
                     {
                        int start=0, end=0;
                        if(edit_text!=null)
                        {
                           start=edit_text.getSelectionStart();
                           end  =edit_text.getSelectionEnd  ();
                        }
                        com.esenthel.Native.text(s.toString(), start, end);
                     }
                  }
                  @Override public final void beforeTextChanged(CharSequence s, int start, int count , int after) {}
                  @Override public final void     onTextChanged(CharSequence s, int start, int before, int count) {}
               };

               FrameLayout.LayoutParams layout=new FrameLayout.LayoutParams(0, 0); // force zero size instead of something like WRAP_CONTENT because if entering many lines in text, it would cause weird offsets in views and touch inputs
               edit_text=new EditText(activity);
               edit_text.setLayoutParams(layout);
               edit_text.setImeOptions(edit_text.getImeOptions()|EditorInfo.IME_FLAG_NO_FULLSCREEN);
               activity.addContentView(edit_text, layout);
            }
            editTextSetDo(text, start, end, mode);
            edit_text.setVisibility(View.VISIBLE);
            edit_text.bringToFront();
            edit_text.requestFocus();

            InputMethodManager imm=(InputMethodManager)activity.getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.showSoftInput(edit_text, 0); // use 0 instead of 'SHOW_FORCED' because that one will keep keyboard visible even when app is hidden using the Home button
         }
      });
   }
   public final void editTextHide()
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            Window window=activity.getWindow(); if(window!=null)
            {
               View view=window.getDecorView(); if(view!=null)
               {
                  InputMethodManager imm=(InputMethodManager)activity.getSystemService(Activity.INPUT_METHOD_SERVICE);
                  imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
               }
            }
            if(edit_text!=null)edit_text.setVisibility(View.GONE);
         }
      });
   }
   /******************************************************************************/
   // GAMEPAD
   /******************************************************************************/
   public static final boolean DeviceHasAxis(android.view.InputDevice device, int axis) {return device.getMotionRange(axis)!=null;}
   public static final float   DeviceMinAxis(android.view.InputDevice device, int axis) {android.view.InputDevice.MotionRange motion_range=device.getMotionRange(axis); return (motion_range!=null) ? motion_range.getMin() : 0;}
   public static final float   DeviceMaxAxis(android.view.InputDevice device, int axis) {android.view.InputDevice.MotionRange motion_range=device.getMotionRange(axis); return (motion_range!=null) ? motion_range.getMax() : 0;}
   /******************************************************************************/
   // MICROPHONE
   /******************************************************************************/
   public static class EsenthelAudioRecord extends AudioRecord
   {
      public Thread thread;

      EsenthelAudioRecord(int frequency, int channelConfig, int audioFormat, int minBufferSize)
      {
         super(MediaRecorder.AudioSource.MIC, frequency, channelConfig, audioFormat, minBufferSize);
      }
   }
   public static final boolean hasAudioRecord()
   {
      return context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_MICROPHONE);
   }
   public static final void delAudioRecord(EsenthelAudioRecord audioRecord)
   {
      audioRecord.stop(); // first stop recording
      try{audioRecord.thread.join();}catch(Exception exception){} // now wait for thread to finish
      audioRecord.thread=null;
      audioRecord.release();
   }
   public static final EsenthelAudioRecord newAudioRecord(final long sound_record, int bits, int channels, int frequency)
   {
      if(channels>=1 && channels<=2 && (bits==8 || bits==16))
      {
         int channelConfig  =((channels==1) ? AudioFormat.CHANNEL_IN_MONO   : AudioFormat.CHANNEL_IN_STEREO ),
             audioFormat    =((bits    ==8) ? AudioFormat.ENCODING_PCM_8BIT : AudioFormat.ENCODING_PCM_16BIT),
             min_buffer_size=AudioRecord.getMinBufferSize(frequency, channelConfig, audioFormat); // for 44100 hz, 1 channel, 16 bits, Galaxy Note 4 returns 3840
         if( min_buffer_size>0)
         {
            try
            {
               final int poll_size=min_buffer_size; // this will affect latency, so don't increase this
               final EsenthelAudioRecord audioRecord=new EsenthelAudioRecord(frequency, channelConfig, audioFormat, min_buffer_size*2); // mul by 2 because "this size doesn't guarantee a smooth recording under load, and higher values should be chosen according to the expected frequency at which the AudioRecord instance will be polled for new data" - http://developer.android.com/reference/android/media/AudioRecord.html#getMinBufferSize(int, int, int)
               if(audioRecord.getState()==AudioRecord.STATE_INITIALIZED)
               {
                  audioRecord.startRecording(); // start recording before starting the thread, because inside it we're checking for RECORDSTATE_RECORDING
                  audioRecord.thread=new Thread()
                  {
                     @Override public final void run()
                     {
                        ByteBuffer buf=ByteBuffer.allocateDirect(poll_size);
                        if(buf!=null)for(; audioRecord.getRecordingState()==AudioRecord.RECORDSTATE_RECORDING; )
                        {
                           int read=audioRecord.read(buf, poll_size); // this is a blocking call, it will wait until data is available
                           if( read>0)com.esenthel.Native.audioRecord(sound_record, buf, read);else
                           if( read<0)break; // error
                        }
                     }
                  };
                  audioRecord.thread.setPriority(Thread.MAX_PRIORITY);
                  audioRecord.thread.start();
                  return audioRecord;
               }
            }catch(Exception exception){}
         }
      }
      return null;
   }
   /******************************************************************************/
   // ADMOB
   /******************************************************************************/
/*ADMOB*\
   // !! these must be equal to 'AdMobClass.BANNER_TYPE' !!
   public static final byte AD_BANNER          =0;
   public static final byte AD_MEDIUM_RECTANGLE=1;
   public static final byte AD_FULL_BANNER     =2;
   public static final byte AD_LEADERBOARD     =3;
   public static final byte AD_SMART_BANNER    =4;
   public static final byte AD_INTERSTITIAL    =5;

   // !! these must be equal to 'AdMobClass.STATE' !!
   public static final byte AD_NONE   =0;
   public static final byte AD_LOADING=1;
   public static final byte AD_DONE   =2;
   public static final byte AD_ERROR  =3;

   static AdRequest      ad_request;
          AdView         ad_view;
   static boolean        ad_view_loaded=false, // if ad_view is loaded
                         banner_visible=false, // if banner should be visible
                         inters_show=false; // if show interstitial once it's loaded
   static String         banner_id, // id of the banner       ad, null for none
                         inters_id; // id of the interstitial ad, null for none
   static byte           banner_type=AD_BANNER, // banner type
                         banner_x=0, banner_y=1; // banner position, -1..1
          InterstitialAd interstitial;
          PopupWindow    popup_window;
          LinearLayout   popup_window_layout, ad_view_layout;

   final void initAdMob()
   {
      MobileAds.initialize(this, "ADMOB_APP_ID");
      com.esenthel.Native.bannerSize(0, 0); // at this stage the banner is lost, but the banner size on the native size is still present, so reset it
      if(ad_request==null)setAdRequest(false);
   }
   public static final void setAdRequest(boolean test_mode)
   {
      AdRequest.Builder builder=new AdRequest.Builder().addTestDevice(AdRequest.DEVICE_ID_EMULATOR);
      if(test_mode)
      {
         String android_id_md5;
         try
         {
            MessageDigest md5=MessageDigest.getInstance("MD5");
            md5.update(android_id.getBytes(), 0, android_id.length());
            android_id_md5=String.format("%032X", new Object[]{new BigInteger(1, md5.digest())});
         }catch(Exception exception){android_id_md5=android_id.substring(0, 32);}
         builder.addTestDevice(android_id_md5);
      }
      ad_request=builder.build();
   }
   public final void bannerHide()
   {
      if(popup_window!=null)
      {
         popup_window.dismiss();
         popup_window=null;
      }
      if(popup_window_layout!=null)
      {
         ViewGroup parent=(ViewGroup)popup_window_layout.getParent(); if(parent!=null)parent.removeView(popup_window_layout);
         popup_window_layout.removeAllViews();
         popup_window_layout=null;
      }
      if(ad_view_layout!=null)
      {
         ViewGroup parent=(ViewGroup)ad_view_layout.getParent(); if(parent!=null)parent.removeView(ad_view_layout);
         ad_view_layout.removeAllViews();
         ad_view_layout=null;
      }
   }
   public final void adViewDel()
   {
      if(ad_view!=null)
      {
         ad_view.destroy(); ad_view=null; ad_view_loaded=false;
      }
   }
   public final void adDel(final boolean banner)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(banner)
            {
               banner_id=null;
               bannerHide();
               adViewDel();
            }else
            {
               inters_id=null;
               interstitial=null;
            }
         }
      });
   }
   public final void adCreate(final String unit_id, final int type)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(ad_request!=null)
            {
               AdSize ad_size=null;
               switch(type)
               {
                  case AD_BANNER          : ad_size=AdSize.BANNER          ; break;
                  case AD_MEDIUM_RECTANGLE: ad_size=AdSize.MEDIUM_RECTANGLE; break;
                  case AD_FULL_BANNER     : ad_size=AdSize.FULL_BANNER     ; break;
                  case AD_LEADERBOARD     : ad_size=AdSize.LEADERBOARD     ; break;
                  case AD_SMART_BANNER    : ad_size=AdSize.SMART_BANNER    ; break;
                  case AD_INTERSTITIAL    :
                  {
                     inters_id=unit_id;
                     interstitial=new InterstitialAd(activity);
                     interstitial.setAdUnitId(unit_id);
                     interstitial.setAdListener(new AdListener()
                     {
                        @Override public final void onAdLoaded         (             ) {com.esenthel.Native.adState(false, AD_DONE   ); if(inters_show){inters_show=false; adVisible(false, true);}}
                        @Override public final void onAdFailedToLoad   (int errorCode) {com.esenthel.Native.adState(false, AD_ERROR  );}
                        @Override public final void onAdOpened         (             ) {}
                        @Override public final void onAdClosed         (             ) {com.esenthel.Native.adState(false, AD_LOADING); adCreate(unit_id, type);} // when ad is closed, it cannot be displayed again, so reload again
                        @Override public final void onAdLeftApplication(             ) {}
                     });
                     interstitial.loadAd(ad_request);
                  }break;
               }
               if(ad_size!=null)
               {
                  banner_id  =unit_id;
                  banner_type=type;
                  bannerHide();
                  adViewDel();
                  ad_view=new AdView(activity);
                  ad_view.setAdSize(ad_size);
                  ad_view.setAdUnitId(unit_id);
                  ad_view.setAdListener(new AdListener()
                  {
                     @Override public final void onAdFailedToLoad   (int errorCode) {com.esenthel.Native.adState(true, AD_ERROR);}
                     @Override public final void onAdOpened         (             ) {}
                     @Override public final void onAdClosed         (             ) {}
                     @Override public final void onAdLeftApplication(             ) {}
                     @Override public final void onAdLoaded         (             )
                     {
                        if(ad_view!=null)
                        {
                           AdSize ad_size=ad_view.getAdSize();
                           if(ad_size!=null)com.esenthel.Native.bannerSize(ad_size.getWidthInPixels (activity),
                                                                           ad_size.getHeightInPixels(activity));
                        }
                        com.esenthel.Native.adState(true, AD_DONE);
                        ad_view_loaded=true; if(banner_visible)adVisible(true, true);
                     }
                  });
                  ad_view.loadAd(ad_request);
               }
            }
         }
      });
   }
   public final int popupWindowW() {return (popup_window!=null) ? (int)dipToPx(popup_window.getWidth ()) : 0;}
   public final int popupWindowH() {return (popup_window!=null) ? (int)dipToPx(popup_window.getHeight()) : 0;}
   public final int bannerX() {int w=popupWindowW(); return (banner_x<0) ? 0 : (banner_x>0) ? resW()-w : (resW()-w)/2;}
   public final int bannerY() {int h=popupWindowH(); return (banner_y>0) ? 0 : (banner_y<0) ? resH()-h : (resH()-h)/2;}
   public final void adVisible(final boolean banner, final boolean visible)
   {
      runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(banner) // operate on banner
            {
               banner_visible=visible;
               if(visible)
               {
                  if(ad_view!=null && ad_view_loaded)
                  {
                     AdSize ad_size=ad_view.getAdSize();
                     if(ad_size!=null && popup_window==null)
                     {
                        MarginLayoutParams params=new MarginLayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                        params.setMargins(0, 0, 0, 0);
                        popup_window=new PopupWindow(activity);
                        popup_window.setWindowLayoutMode(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                             ad_view_layout=new LinearLayout(activity);
                        popup_window_layout=new LinearLayout(activity);
                        ad_view_layout.setPadding(-11, -11, -11, -11); // these values are required for ads to display on high resolution screens like 2560x1440 (Galaxy Note 4), this was also tested on 1280x720 (Galaxy Note 2), if values are bigger then the ad will not display complaining it has not enough space
                          popup_window.setContentView(     ad_view_layout);
                              activity.setContentView(popup_window_layout, params);
                        ad_view_layout.addView  (ad_view, params);
                          popup_window.setWidth (ad_size.getWidth ());
                          popup_window.setHeight(ad_size.getHeight());
                          popup_window.showAtLocation(popup_window_layout, Gravity.LEFT|Gravity.TOP, bannerX(), bannerY());
                          popup_window.update();
                     }
                  }
               }else
               {
                  bannerHide();
               }
            }else // operate on interstitial
            {
               if(visible)
               {
                  if(interstitial!=null && interstitial.isLoaded())interstitial.show();
                  else                                             inters_show=true; // show next time
               }else
               {
                  inters_show=false; // disable showing next time
                  // there's no method to hide
               }
            }
         }
      });
   }
   public final void adPos(int x, int y)
   {
      banner_x=x;
      banner_y=y;
      if(popup_window!=null)runOnUiThread(new Runnable()
      {
         @Override public final void run()
         {
            if(popup_window!=null)popup_window.update(bannerX(), bannerY(), -1, -1);
         }
      });
   }
/*ADMOB*/
   /******************************************************************************/
   // IAB - In App Billing
   /******************************************************************************
   import com.android.vending.billing.IInAppBillingService;

   // Item types
   public static final String ITEM_TYPE_INAPP="inapp";
   public static final String ITEM_TYPE_SUBS ="subs";

   // some fields on the getSkuDetails response bundle
   public static final String GET_SKU_DETAILS_ITEM_LIST="ITEM_ID_LIST";

   // Keys for the responses from InAppBillingService
   public static final String RESPONSE_CODE = "RESPONSE_CODE";
   public static final String RESPONSE_GET_SKU_DETAILS_LIST = "DETAILS_LIST";
   public static final String RESPONSE_BUY_INTENT = "BUY_INTENT";
   public static final String RESPONSE_INAPP_PURCHASE_DATA = "INAPP_PURCHASE_DATA";
   public static final String RESPONSE_INAPP_SIGNATURE = "INAPP_DATA_SIGNATURE";
   public static final String RESPONSE_INAPP_ITEM_LIST = "INAPP_PURCHASE_ITEM_LIST";
   public static final String RESPONSE_INAPP_PURCHASE_DATA_LIST = "INAPP_PURCHASE_DATA_LIST";
   public static final String RESPONSE_INAPP_SIGNATURE_LIST = "INAPP_DATA_SIGNATURE_LIST";
   public static final String INAPP_CONTINUATION_TOKEN = "INAPP_CONTINUATION_TOKEN";

   // Billing response codes
   public static final int BILLING_RESPONSE_RESULT_OK = 0;
   public static final int BILLING_RESPONSE_RESULT_USER_CANCELED = 1;
   public static final int BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE = 3;
   public static final int BILLING_RESPONSE_RESULT_ITEM_UNAVAILABLE = 4;
   public static final int BILLING_RESPONSE_RESULT_DEVELOPER_ERROR = 5;
   public static final int BILLING_RESPONSE_RESULT_ERROR = 6;
   public static final int BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED = 7;
   public static final int BILLING_RESPONSE_RESULT_ITEM_NOT_OWNED = 8;

   // signature verification
   private static final String KEY_FACTORY_ALGORITHM="RSA";
   private static final String   SIGNATURE_ALGORITHM="SHA1withRSA";

   static IInAppBillingService iab_service;
   static ServiceConnection    iab_service_conn;

   final void initIAB()
   {
      if(iab_service_conn==null)
      {
         iab_service_conn=new ServiceConnection()
         {
            @Override public final void onServiceDisconnected(ComponentName name) {iab_service=null;}

            @Override public final void onServiceConnected(ComponentName name, IBinder binder)
            {
               iab_service=IInAppBillingService.Stub.asInterface(binder);
               boolean inapp_supported=false, subs_supported=false;
               try
               {
                  inapp_supported=(iab_service.isBillingSupported(3, "com.esenthel.project", ITEM_TYPE_INAPP)==BILLING_RESPONSE_RESULT_OK);
                   subs_supported=(iab_service.isBillingSupported(3, "com.esenthel.project", ITEM_TYPE_SUBS )==BILLING_RESPONSE_RESULT_OK);
               }catch(Exception exception){}
               com.esenthel.Native.connected(inapp_supported, subs_supported);
            }
         };

         Intent serviceIntent=new Intent("com.android.vending.billing.InAppBillingService.BIND");
         serviceIntent.setPackage("com.android.vending");
         List<ResolveInfo> services=getPackageManager().queryIntentServices(serviceIntent, 0);
         if(services!=null && !services.isEmpty())bindService(serviceIntent, iab_service_conn, Context.BIND_AUTO_CREATE);
      }
   }
   final void shutIAB()
   {
      if(iab_service_conn!=null){unbindService(iab_service_conn); iab_service_conn=null;}
   }
   static final int billingToEE(int result)
   {
      switch(result)
      {
         case BILLING_RESPONSE_RESULT_OK                 : return RES_OK;
         case BILLING_RESPONSE_RESULT_USER_CANCELED      : return RES_USER_CANCELED;
         case BILLING_RESPONSE_RESULT_BILLING_UNAVAILABLE: return RES_SERVICE_UNAVAILABLE;
         case BILLING_RESPONSE_RESULT_ITEM_UNAVAILABLE   : return RES_ITEM_UNAVAILABLE;
         case BILLING_RESPONSE_RESULT_ITEM_ALREADY_OWNED : return RES_ALREADY_OWNED;
         case BILLING_RESPONSE_RESULT_ITEM_NOT_OWNED     : return RES_NOT_OWNED;
         default                                         : return RES_UNKNOWN;
      }
   }
   static final int getResponseCodeFromObject(Object o)
   {
      if(o==null)return BILLING_RESPONSE_RESULT_OK;
      if(o instanceof Integer)return      ((Integer)o). intValue();
      if(o instanceof Long   )return (int)((Long   )o).longValue();
      return BILLING_RESPONSE_RESULT_ERROR; // Unexpected type for bundle response code
   }
   static final int getResponseCodeFromBundle(Bundle b) {return getResponseCodeFromObject(b.            get(RESPONSE_CODE));}
   static final int getResponseCodeFromIntent(Intent i) {return getResponseCodeFromObject(i.getExtras().get(RESPONSE_CODE));}

   static final PublicKey generatePublicKey(String encodedPublicKey)
   {
      if(Is(encodedPublicKey))
      {
         try
         {
            byte[] decodedKey=com.esenthel.Base64.decode(encodedPublicKey); if(decodedKey==null)return null;
            KeyFactory keyFactory=KeyFactory.getInstance(KEY_FACTORY_ALGORITHM);
            return keyFactory.generatePublic(new X509EncodedKeySpec(decodedKey));
         }catch(Exception exception){}
      }
      return null;
   }
   static final boolean verifyPurchase(String data, String signature)
   {
      if(license_key==null)
      {
         if(!Is("APP_LICENSE_KEY"))return true; // no license key specified, so always succeed
         license_key=generatePublicKey("APP_LICENSE_KEY");
         if(license_key==null)return false; // failed to generate license key
      }
      if(!Is(data) || !Is(signature))return false;
      try
      {
         Signature sig=Signature.getInstance(SIGNATURE_ALGORITHM);
         sig.initVerify(license_key);
         sig.update(data.getBytes());
         byte[] decodedKey=com.esenthel.Base64.decode(signature); if(decodedKey==null)return false;
         if(!sig.verify(decodedKey))return false;
         return true;
      }catch(Exception exception){}
      return false;
   }

   public static final boolean getItemDetails(boolean regular_items, boolean subs, int item_ids_elms, long item_ids, long user)
   {
      if(iab_service==null)return false;

      ArrayList<String> skuList=new ArrayList<String>();
      for(int i=0; i<item_ids_elms; i++)skuList.add(com.esenthel.Native.getStr(item_ids, i));
      Bundle querySkus=new Bundle(); querySkus.putStringArrayList(GET_SKU_DETAILS_ITEM_LIST, skuList);

      for(int i=0; i<2; i++)
      {
         boolean sub=(i!=0);
         if(sub ? subs : regular_items)
         {
            String itemType=(sub ? ITEM_TYPE_SUBS : ITEM_TYPE_INAPP);
            try
            {
               Bundle skuDetails=iab_service.getSkuDetails(3, "com.esenthel.project", itemType, querySkus);
               if(skuDetails.containsKey(RESPONSE_GET_SKU_DETAILS_LIST))
               {
                  ArrayList<String> responseList=skuDetails.getStringArrayList(RESPONSE_GET_SKU_DETAILS_LIST);
                  for(String json:responseList)
                  {
                     String sku, name, desc, price;
                     try
                     {
                        JSONObject o=new JSONObject(json);
                        sku  =o.optString("productId");
                            //o.optString("type");
                        price=o.optString("price");
                        name =o.optString("title");
                        desc =o.optString("description");
                     }catch(Exception exception){continue;} // failed to parse data, proceed to next one
                     if(sku!=null)com.esenthel.Native.listItem(user, sku, name, desc, price, sub);
                  }
               }
            }catch(Exception exception){}
         }
      }
      return true;
   }

   public static final boolean getPurchases(boolean sub, long user)
   {
      if(iab_service==null)return false;
      String itemType=(sub ? ITEM_TYPE_SUBS : ITEM_TYPE_INAPP), continueToken=null;
      do
      {
         try
         {
            Bundle ownedItems=iab_service.getPurchases(3, "com.esenthel.project", itemType, continueToken);
            int response =getResponseCodeFromBundle(ownedItems);
            if( response!=BILLING_RESPONSE_RESULT_OK)return false;

            if(!ownedItems.containsKey(RESPONSE_INAPP_ITEM_LIST         )
            || !ownedItems.containsKey(RESPONSE_INAPP_PURCHASE_DATA_LIST)
            || !ownedItems.containsKey(RESPONSE_INAPP_SIGNATURE_LIST    ))return false; // Bundle doesn't contain required fields

            ArrayList<String> ownedSkus       =ownedItems.getStringArrayList(RESPONSE_INAPP_ITEM_LIST);
            ArrayList<String> purchaseDataList=ownedItems.getStringArrayList(RESPONSE_INAPP_PURCHASE_DATA_LIST);
            ArrayList<String> signatureList   =ownedItems.getStringArrayList(RESPONSE_INAPP_SIGNATURE_LIST);

            for(int i=0; i<purchaseDataList.size(); i++)
            {
               String purchaseData=purchaseDataList.get(i),
                      signature   =signatureList   .get(i),
                      sku         =ownedSkus       .get(i),
                      custom_data=null, token=null;
               long   date        =0;
               if(purchaseData!=null)
               {
                  try
                  {
                     JSONObject o=new JSONObject(purchaseData);
                   //o.optString("orderId");
                   //o.optString("packageName");
               //sku=o.optString("productId");
                date=o.optLong("purchaseTime");
                   //o.optInt("purchaseState");
         custom_data=o.optString("developerPayload");
               token=o.optString("token", o.optString("purchaseToken"));
                  }catch(Exception exception){} // failed to parse data
               }
               if(verifyPurchase(purchaseData, signature))com.esenthel.Native.listPurchase(user, sku, custom_data, token, date);
            }
            continueToken=ownedItems.getString(INAPP_CONTINUATION_TOKEN);
         }catch(Exception exception){return false;}
      }while(Is(continueToken));

      return true;
   }

   public static final int buy(String id, String data, boolean subscription)
   {
      if(iab_service==null || activity==null)return RES_SERVICE_UNAVAILABLE;
      int result;
      try
      {
         Bundle buyIntentBundle=iab_service.getBuyIntent(3, "com.esenthel.project", id, subscription ? ITEM_TYPE_SUBS : ITEM_TYPE_INAPP, data);
         result=billingToEE(getResponseCodeFromBundle(buyIntentBundle));
         if(result==RES_OK)
         {
            result=RES_WAITING; // we're not done yet
            PendingIntent pendingIntent=buyIntentBundle.getParcelable(RESPONSE_BUY_INTENT);
            activity.startIntentSenderForResult(pendingIntent.getIntentSender(), REQUEST_CODE_IAB, new Intent(), Integer.valueOf(0), Integer.valueOf(0), Integer.valueOf(0));
         }
      }catch(Exception exception){result=RES_UNKNOWN;}
      return result;
   }

   public static final int consume(String token)
   {
      if(iab_service==null)return RES_SERVICE_UNAVAILABLE;
      int result;
      try
      {
         result=billingToEE(iab_service.consumePurchase(3, "com.esenthel.project", token));
         if(result==RES_OK)result=RES_CONSUMED;
      }catch(Exception exception){result=RES_UNKNOWN;}
      return result;
   }
   /******************************************************************************/
   // License Test
   /******************************************************************************/
/*LICENSE_KEY*\
   private static class MyLicenseCheckerCallback implements LicenseCheckerCallback
   {
      public static final byte[] SALT=new byte[]{2, 53, 106, 6, 35, 69, 118, 115, 64, 101, 117, 1, 13, 83, 127, 7, 6, 85, 25, 8};
      private final void set(int reason)
      {
         int result;
         switch(reason)
         {
            case Policy.LICENSED: result=LTR_OK   ; break;
            case Policy.RETRY   : result=LTR_RETRY; break;
            default             : result=LTR_FAIL ; break; // Policy.NOT_LICENSED
         }
         com.esenthel.Native.licenseTest(result);
      }
      @Override public void            allow(int reason   ) {set(reason);}
      @Override public void        dontAllow(int reason   ) {set(reason);}
      @Override public void applicationError(int errorCode)
      {
         int result;
         switch(errorCode)
         {
            case LicenseCheckerCallback.ERROR_CHECK_IN_PROGRESS: result=LTR_WAITING; break;
            default                                            : result=LTR_ERROR  ; break;
         }
         com.esenthel.Native.licenseTest(result);
      }
   }
   private static LicenseCheckerCallback licenseCheckerCallback;
   private static LicenseChecker         licenseChecker;
   public final void initLicenseTest()
   {
      licenseCheckerCallback=new MyLicenseCheckerCallback();
      licenseChecker        =new   LicenseChecker        (this, new ServerManagedPolicy(this, new AESObfuscator(MyLicenseCheckerCallback.SALT, "com.esenthel.project", android_id)), "APP_LICENSE_KEY");
   }
   public        final void shutLicenseTest() {if(licenseChecker!=null)licenseChecker.onDestroy();}
   public static final void     licenseTest() {if(licenseChecker!=null)licenseChecker.checkAccess(licenseCheckerCallback);}
/*LICENSE_KEY*/
   /******************************************************************************/
   // FB - Facebook
   /******************************************************************************/
/*FACEBOOK*\
   private CallbackManager callbackManager;
   private ShareDialog     shareDialog;
   private static boolean  facebook_get_me=false, facebook_get_friends=false;
   final void initFB()
   {
      com.facebook.FacebookSdk.sdkInitialize(getApplicationContext());
      callbackManager=CallbackManager.Factory.create();
      LoginManager.getInstance().registerCallback(callbackManager, new FacebookCallback<LoginResult>()
      {
         @Override public void onSuccess(LoginResult loginResult)
         {
            if(facebook_get_me     ){facebook_get_me     =false; facebookGetMeDo     ();}
            if(facebook_get_friends){facebook_get_friends=false; facebookGetFriendsDo();}
         }
         @Override public void onCancel() {}
         @Override public void onError(FacebookException exception) {}
      });
      shareDialog=new ShareDialog(this);
      shareDialog.registerCallback(callbackManager, new FacebookCallback<Sharer.Result>()
      {
         @Override public void onSuccess(Sharer.Result    result) {com.esenthel.Native.facebookPost(POST_SUCCESS);}
         @Override public void onCancel (                       ) {com.esenthel.Native.facebookPost(POST_CANCEL );}
         @Override public void onError  (FacebookException error) {com.esenthel.Native.facebookPost(POST_ERROR  );}
      });
   }
   public static final boolean facebookLoggedIn()
   {
      AccessToken accessToken=AccessToken.getCurrentAccessToken();
      return accessToken!=null && !accessToken.isExpired();
   }
   public final void facebookLogIn()
   {
      LoginManager.getInstance().logInWithReadPermissions(this, Arrays.asList("public_profile", "email", "user_friends"));
   }
   public static final void facebookLogOut()
   {
      LoginManager.getInstance().logOut();
   }
   public static final void facebookGetMeDo()
   {
      GraphRequest request=GraphRequest.newMeRequest(AccessToken.getCurrentAccessToken(), new GraphRequest.GraphJSONObjectCallback()
      {
         @Override public void onCompleted(JSONObject object, GraphResponse response)
         {
            if(object!=null)com.esenthel.Native.facebookMe(object.optString("id"), object.optString("name"), object.optString("email"));
         }
      });
      Bundle parameters=new Bundle();
      parameters.putString("fields", "id,name,email");
      request.setParameters(parameters);
      request.executeAsync();
   }
   public static final void facebookGetFriendsDo()
   {
      GraphRequest request=GraphRequest.newMyFriendsRequest(AccessToken.getCurrentAccessToken(), new GraphRequest.GraphJSONArrayCallback()
      {
         @Override public void onCompleted(JSONArray jsonArray, GraphResponse response)
         {
            if(jsonArray!=null)
            {
               ArrayList<String>   ids=new ArrayList<String>();
               ArrayList<String> names=new ArrayList<String>();
               for(int i=0; i<jsonArray.length(); i++)
               {
                  JSONObject object=jsonArray.optJSONObject(i); if(object!=null)
                  {
                       ids.add(object.optString("id"  ));
                     names.add(object.optString("name"));
                  }
               }
               com.esenthel.Native.facebookFriends(ids, names);
            }
         }
      });
      Bundle parameters=new Bundle();
      parameters.putString("fields", "id,name");
      request.setParameters(parameters);
      request.executeAsync();
   }
   public final void facebookGetMe     () {if(facebookLoggedIn())facebookGetMeDo     ();else{facebook_get_me     =true; facebookLogIn();}}
   public final void facebookGetFriends() {if(facebookLoggedIn())facebookGetFriendsDo();else{facebook_get_friends=true; facebookLogIn();}}
   public final void facebookPost(String url, String quote)
   {
      if(shareDialog!=null)
      {
         ShareLinkContent content=new ShareLinkContent.Builder()
           .setContentUrl(Uri.parse(url))
           .setQuote(quote)
           .build();
         shareDialog.show(content);
      }
   }
/*FACEBOOK*/
   /******************************************************************************/
   // CHARTBOOST
   /******************************************************************************/
/*CHARTBOOST*\
   // !! these must be equal to 'ChartboostClass.RESULT' !!
   public static final byte INTERSTITIAL_LOADED   =0;
   public static final byte INTERSTITIAL_LOAD_FAIL=1;
   public static final byte INTERSTITIAL_DISPLAYED=2;
   public static final byte INTERSTITIAL_CLOSED   =3;
   public static final byte INTERSTITIAL_CLICKED  =4;
   public static final byte REWARDED_VIDEO_LOADED   =5;
   public static final byte REWARDED_VIDEO_LOAD_FAIL=6;
   public static final byte REWARDED_VIDEO_DISPLAYED=7;
   public static final byte REWARDED_VIDEO_CLOSED   =8;
   public static final byte REWARDED_VIDEO_COMPLETED=9;
   public static final byte REWARDED_VIDEO_CLICKED  =10;

   public static final boolean chartboostVisible() {return Chartboost.isAnyViewVisible();}

   public static final void    chartboostInterstitialLoad     () {       Chartboost.cacheInterstitial(CBLocation.LOCATION_DEFAULT);}
   public static final void    chartboostInterstitialShow     () {       Chartboost. showInterstitial(CBLocation.LOCATION_DEFAULT);}
   public static final boolean chartboostInterstitialAvailable() {return Chartboost.  hasInterstitial(CBLocation.LOCATION_DEFAULT);}

   public static final void    chartboostRewardedVideoLoad     () {       Chartboost.cacheRewardedVideo(CBLocation.LOCATION_DEFAULT);}
   public static final void    chartboostRewardedVideoShow     () {       Chartboost. showRewardedVideo(CBLocation.LOCATION_DEFAULT);}
   public static final boolean chartboostRewardedVideoAvailable() {return Chartboost.  hasRewardedVideo(CBLocation.LOCATION_DEFAULT);}

   final void initChartboost()
   {
      Chartboost.startWithAppId(this, "CHARTBOOST_APP_ID", "CHARTBOOST_APP_SIGNATURE");
      Chartboost.setDelegate(new ChartboostDelegate() { // it must be called between 'Chartboost.startWithAppId' and 'Chartboost.onCreate'
       //@Override public final boolean shouldRequestInterstitial(String location) {return true;}
       //@Override public final boolean shouldDisplayInterstitial(String location) {return true;}
         @Override public final void didCacheInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_LOADED);}
         @Override public final void didFailToLoadInterstitial(String location, CBImpressionError error) {com.esenthel.Native.chartboost(INTERSTITIAL_LOAD_FAIL);}
         @Override public final void didDisplayInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_DISPLAYED);}
         @Override public final void didDismissInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_CLOSED);}
       //@Override public final void didCloseInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_CLOSED);} this doesn't get called when interstitial is closed through clicking
         @Override public final void didClickInterstitial(String location) {com.esenthel.Native.chartboost(INTERSTITIAL_CLICKED);}

       //@Override public final boolean shouldDisplayRewardedVideo(String location) {return true;}
         @Override public final void didCacheRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_LOADED);}
         @Override public final void didFailToLoadRewardedVideo(String location, CBImpressionError error) {com.esenthel.Native.chartboost(REWARDED_VIDEO_LOAD_FAIL);}
         @Override public final void didDisplayRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_DISPLAYED);}
       //@Override public final void willDisplayVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_DISPLAYED);} this gets called everytime 'didDisplayRewardedVideo' gets called too, no need for 2 notifications
         @Override public final void didDismissRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_CLOSED);}
       //@Override public final void didCloseRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_CLOSED);} this gets called everytime 'didDismissRewardedVideo' gets called too, no need for 2 notifications
         @Override public final void didClickRewardedVideo(String location) {com.esenthel.Native.chartboost(REWARDED_VIDEO_CLICKED);}
         @Override public final void didCompleteRewardedVideo(String location, int reward) {com.esenthel.Native.chartboost(REWARDED_VIDEO_COMPLETED);}
      });
      Chartboost.onCreate(this);
   }
/*CHARTBOOST*/
   /******************************************************************************/
   // NOTIFICATIONS
   /******************************************************************************/
   public static class NotificationDismissed extends BroadcastReceiver
   {
      @Override public final void onReceive(Context context, Intent intent) {checkNotification(intent, false);}
   }
   public static final String            NOTIFICATION_CHANNEL_ID="BackgroundService";
   public static final String            NOTIFICATION_ID        ="NotificationID";
   public static final byte   BACKGROUND_NOTIFICATION_ID        =-1; // must not be 0 because 'startForeground' does not accept 0, use -1 because 0..+ are used by custom Notifications !! if this is changed then adjust 'Application.backgroundText' !!
   public static final void initNotification()
   {
      if(Build.VERSION.SDK_INT>=26)
      {
         NotificationManager manager=(NotificationManager)activity.getSystemService(Context.NOTIFICATION_SERVICE); if(manager!=null)
         { // !! ONCE CHANNEL IS CREATED, IT'S NOT POSSIBLE TO CHANGE ITS PROPERTIES, UNLESS APP IS UNINSTALLED !!
            android.app.NotificationChannel channel=new android.app.NotificationChannel(NOTIFICATION_CHANNEL_ID, "Background Service", NotificationManager.IMPORTANCE_DEFAULT);
            channel.setSound(null, null);
            manager.createNotificationChannel(channel);
         }
      }
   }
   public static final void checkNotification(Intent intent, boolean selected)
   {
      if(intent!=null)
      {
         int notification_id=intent.getIntExtra(NOTIFICATION_ID, -1); // -1=default value
         if( notification_id>=0)com.esenthel.Native.notification(notification_id, selected);
      }   
   }
   public static final Notification getNotification(int id, String title, String text, boolean dismissable)
   {
      Notification.Builder builder;
      if(Build.VERSION.SDK_INT>=26)builder=new Notification.Builder(context, NOTIFICATION_CHANNEL_ID);
      else                         builder=new Notification.Builder(context);
      if(builder!=null)
      {
         Intent intent=new Intent(context, EsenthelActivity.class);
         if(id>=0)intent.putExtra   (NOTIFICATION_ID, id); // store NOTIFICATION_ID only for regular notifications
                  intent.addCategory(Intent.CATEGORY_LAUNCHER);
                  intent.setAction  (Intent.ACTION_MAIN);
         if(dismissable && id>=0)
         {
            Intent intent_dismiss=new Intent(context, NotificationDismissed.class);
            intent_dismiss.putExtra(NOTIFICATION_ID, id);
            PendingIntent pending_intent_dismiss=PendingIntent.getBroadcast(context, id*2+1, intent_dismiss, PendingIntent.FLAG_UPDATE_CURRENT); // use id*2+1 to make unique
            builder.setDeleteIntent(pending_intent_dismiss);
         }
         PendingIntent pending_intent=PendingIntent.getActivity(context, id*2, intent, PendingIntent.FLAG_UPDATE_CURRENT); // use id*2 to make unique
         return builder.setContentTitle(title).setContentText(text).setSmallIcon(R.drawable.notification).setLargeIcon(BitmapFactory.decodeResource(context.getResources(), R.drawable.icon)).setOngoing(!dismissable).setContentIntent(pending_intent).build();
      }
      return null;
   }
   public static final void setNotification(int id, String title, String text, boolean dismissable)
   {
      if(id==BACKGROUND_NOTIFICATION_ID && background_intent==null)return; // don't adjust background notification if we're not actually in background (can happen if we call this in InitPre, when application hasn't finished initializing but it's also not in the background)
      NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE); if(manager!=null)
      { // !! NOTIFICATION MAY NOT SHOW IN THE STATUS BAR IF DEVICE IS IN "DO NOT DISTURB" MODE !!
         Notification notification=getNotification(id, title, text, dismissable);
         if(notification!=null)manager.notify(id, notification);
      }
   }
   public static final void removeNotification(int id)
   {
      NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE); if(manager!=null)manager.cancel(id);
   }
   public static final void removeNotifications()
   {
      NotificationManager manager=(NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE); if(manager!=null)manager.cancelAll();
   }

   /******************************************************************************/
   // BACKGROUND SERVICE
   /******************************************************************************/
   public static class BackgroundService extends Service // !! if changing name of this class then update AndroidManifest.xml !!
   {
      @Override public final IBinder onBind        (Intent intent) {return null;}
    //@Override public final void    onCreate      (             ) {super.onCreate(); background_service=this;}
    //@Override public final void    onDestroy     (             ) {background_service=null; super.onDestroy();}
      @Override public final int     onStartCommand(Intent intent, int flags, int startId)
      {
         Notification notification=getNotification(BACKGROUND_NOTIFICATION_ID, "APP_NAME", background_text, false);
         if(notification!=null)startForeground(BACKGROUND_NOTIFICATION_ID, notification);
         return START_STICKY; // restart if got killed
      }
   }
   public final void startBackgroundService(String text)
   {
      if(background_intent==null)
      {
         background_text=text;
         background_intent=new Intent(this, BackgroundService.class);
         startService(background_intent);
      }
   }
   public final void stopBackgroundService()
   {
      if(background_intent!=null)
      {
         stopService(background_intent);
         background_intent=null;
      }
   }

   /******************************************************************************/
   // PHOTO PICKER
   /******************************************************************************/
   public final void pickPhoto(int max)
   {
      Intent i=new Intent(Intent.ACTION_OPEN_DOCUMENT);
      i.addCategory(Intent.CATEGORY_OPENABLE);
      i.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, max>1);
      i.setType("image/*");
      startActivityForResult(i, REQUEST_CODE_FILE_PICKER);
   }
}
/******************************************************************************/
