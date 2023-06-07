/******************************************************************************/
#define SUPPORT_WINDOWS_XP (!X64 && GL) // 0=minor performance improvements in some parts of the engine, but no WindowsXP support, 1=some extra checks in the codes but with WindowsXP support
#define SUPPORT_WINDOWS_7  (        GL) // 0=uses XAudio 2.9 (which requires Windows 10), 1=uses DirectSound

#define JP_X_INPUT       (WINDOWS_OLD && (SUPPORT_WINDOWS_XP || SUPPORT_WINDOWS_7))
#define JP_GAMEPAD_INPUT (WINDOWS     && !JP_X_INPUT) // always use on WINDOWS_NEW to allow 'App.joypad_user_changed'
#define JP_DIRECT_INPUT  (WINDOWS_OLD && 0) // disable DirectInput-only Joypads because it introduces 0.25s delay to engine startup. Modern Joypads use XInput, so this is only for old Joypads.
#define JP_RAW_INPUT     (WINDOWS_OLD && 0)

#if EE_PRIVATE
   // Threads
   #if WEB
      #define HAS_THREADS 0 // WEB has no real threads
   #else
      #define HAS_THREADS 1
   #endif
   /******************************************************************************/
   // SELECT WHICH LIBRARIES TO USE
   /******************************************************************************/
   // Renderer - Define "DX11" for DirectX 10/11, "DX12" for DirectX 12, "METAL" for Metal, "VULKAN" for Vulkan, "GL" or nothing for OpenGL
   // defines are specified through Project Settings
   #ifdef DX11
      #undef  DX11
      #define DX11 1
   #else
      #define DX11 0
   #endif

   #ifdef DX12
      #undef  DX12
      #define DX12 1
   #else
      #define DX12 0
   #endif

   #ifdef METAL
      #undef  METAL
      #define METAL 1
   #else
      #define METAL 0
   #endif

   #ifdef VULKAN
      #undef  VULKAN
      #define VULKAN 1
   #else
      #define VULKAN 0
   #endif

   #if defined GL || !(DX11 || DX12 || METAL || VULKAN)
      #undef  GL
      #define GL 1
   #else
      #define GL 0
   #endif

   #if (DX11+DX12+METAL+VULKAN+GL)!=1
      #error Unsupported platform detected
   #endif

   #define GL_ES (GL && (IOS || ANDROID || WEB))

   #define GPU_LOCK 0 // if force using a lock around GPU API calls (this shouldn't be needed)

   #define SLOW_SHADER_LOAD GL // Only OpenGL has slow shader loads because it compiles on the fly from text instead of binary

   #if DX11
      #define GPU_API(dx11, gl) dx11
   #elif GL
      #define GPU_API(dx11, gl) gl
   #endif

   #define LINEAR_GAMMA 1
   #define GPU_HALF_SUPPORTED   (!GL_ES) // depends on "GL_OES_vertex_half_float" GLES extension
   #define DEPTH_CLIP_SUPPORTED (!GL_ES)

   #define REVERSE_DEPTH (!GL) // if Depth Buffer is reversed. Can't enable on GL because for some reason (might be related to #glClipControl) it disables far-plane depth clipping, which can be observed when using func=FUNC_ALWAYS inside D.depthFunc. Even though we clear the depth buffer, there may still be performance hit, because normally geometry would already get clipped due to far plane, but without it, per-pixel depth tests need to be performed.

   #if PHYSX
      #define PHYS_API(physx, bullet) physx
   #else
      #define PHYS_API(physx, bullet) bullet
   #endif

   // Sound
   #define DIRECT_SOUND_RECORD WINDOWS_OLD                                                // use DirectSound Recording on Windows Classic
   #define DIRECT_SOUND        (WINDOWS_OLD && (SUPPORT_WINDOWS_XP || SUPPORT_WINDOWS_7)) // use DirectSound           on Windows XP and 7
   #define XAUDIO              (WINDOWS     && !DIRECT_SOUND)                             // use XAudio                on Windows when DirectSound is unused
   #define OPEN_AL             (APPLE || LINUX || WEB)                                    // use OpenAL                on Apple, Linux and Web. OpenAL on Windows requires OpenAL DLL file, however it can be enabled just for testing the implementation.
   #define OPEN_SL             ANDROID                                                    // use OpenSL                on Android
   #define CUSTOM_AUDIO        SWITCH
   #if (DIRECT_SOUND+XAUDIO+OPEN_AL+OPEN_SL+CUSTOM_AUDIO)>1
      #error Can't use more than 1 API
   #endif

   // Input
   #define KB_RAW_INPUT     1
   #define KB_DIRECT_INPUT  0
   #define MS_RAW_INPUT     1
   #define MS_DIRECT_INPUT  0
   #if (KB_RAW_INPUT+KB_DIRECT_INPUT)!=1 || (MS_RAW_INPUT+MS_DIRECT_INPUT)!=1 || (JP_X_INPUT && JP_GAMEPAD_INPUT) // XInput can't be used together with GamepadInput
      #error Invalid Input API configuration
   #endif
   #define DIRECT_INPUT (KB_DIRECT_INPUT || MS_DIRECT_INPUT || JP_DIRECT_INPUT)
   /******************************************************************************/
   // INCLUDE SYSTEM HEADERS
   /******************************************************************************/
   // this needs to be included first, as some macros may cause conflicts with the headers
   #include "../../../ThirdPartyLibs/begin.h"

   #if WINDOWS // Windows
      #if WINDOWS_OLD
         #if SUPPORT_WINDOWS_XP // https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745.aspx (this can be used for compilation testing if we don't use any functions not available on WindowsXP, however we can use defines and enums)
            #define _WIN32_WINNT 0x0502 // _WIN32_WINNT_WS03 , don't use any API's newer than WindowsXP SP2
         #elif SUPPORT_WINDOWS_7
            #define _WIN32_WINNT 0x0600 // _WIN32_WINNT_VISTA, don't use any API's newer than Windows Vista
         #else
            #define _WIN32_WINNT 0x0602 // _WIN32_WINNT_WIN8 , don't use any API's newer than Windows 8
         #endif
      #endif
      #define NOGDICAPMASKS
      #define NOICONS
      #define NOKEYSTATES
      #define OEMRESOURCE
      #define NOATOM
      #define NOCOLOR
      #define NODRAWTEXT
      #define NOKERNEL
      #define NOMEMMGR
      #define NOMETAFILE
      #define NOMINMAX
      #define NOOPENFILE
      #define NOSCROLL
      #define NOSERVICE
      #define NOSOUND
      #define NOCOMM
      #define NOKANJI
      #define NOHELP
      #define NOPROFILER
      #define NODEFERWINDOWPOS
      #define NOMCX
      #define _ISO646
      #define TokenType    WindowsTokenType
      #define UpdateWindow WindowsUpdateWindow
      #define TimeStamp    WindowsTimeStamp
      #define LOCK_WRITE   WindowsLOCK_WRITE
      #define Font         WindowsFont
      #define FontPtr      WindowsFontPtr
      #define _ALLOW_RTCc_IN_STL
   #elif APPLE // Apple
      #define Ptr       ApplePtr
      #define Point     ApplePoint
      #define Cell      AppleCell
      #define Rect      AppleRect
      #define Button    AppleButton
      #define Cursor    AppleCursor
      #define FileInfo  AppleFileInfo
      #define TextStyle AppleTextStyle
      #define gamma     AppleGamma
      #define ok        AppleOk
      #define require   AppleRequire
      #define __STDBOOL_H // to ignore stdbool.h header which defines _Bool which is used by PhysX otherwise
   #elif LINUX || WEB
      #define Time   LinuxTime
      #define Font   LinuxFont
      #define Region LinuxRegion
      #define Window XWindow
      #define Cursor XCursor
   #endif

   #include <stdio.h>
   #include <new>
   #include <typeinfo>
   #include <type_traits>
   #include <atomic>
   #include <thread>
   #include <fcntl.h>
   #include <locale.h>
   #include <string.h>

   #if WINDOWS
      #include <winsock2.h>
      #include <ws2tcpip.h>
      #include <process.h>
      #include <windows.h>
      #include <io.h>
      #include <share.h>
      #include <sys/stat.h>
      #include <intrin.h>
      #include <IPHlpApi.h>
      #if WINDOWS_OLD
         #include <shlobj.h>
         #include <psapi.h>
         #include <wbemidl.h>
         #include <tlhelp32.h>
         #define SECURITY_WIN32
         #include <Security.h>
         #include <comdef.h>
         #include <Icm.h>
      #else
         #include <collection.h>
         #include <ppltasks.h>
         #include <mmdeviceapi.h>
         #include <audioclient.h>
         #include <wrl/implements.h>
      #endif

      #if JP_X_INPUT
         #include <xinput.h>
      #endif
      #if WINDOWS_OLD && JP_GAMEPAD_INPUT
         #include <wrl.h>
         #include <windows.gaming.input.h>
      #endif
      #if WINDOWS_OLD && DIRECT_INPUT
         #define DIRECTINPUT_VERSION 0x0800
         #include <dinput.h>
      #endif
      #if JP_RAW_INPUT
         #include <hidsdi.h>
         #include <hidpi.h>
      #endif

      #if DIRECT_SOUND || DIRECT_SOUND_RECORD
         #include <dsound.h>
      #endif

      #if XAUDIO
         #include <xaudio2.h>
         #include <x3daudio.h>
      #endif

      #if OPEN_AL
         #include "../../../ThirdPartyLibs/OpenAL for Windows/al.h"
         #include "../../../ThirdPartyLibs/OpenAL for Windows/alc.h"
      #endif

      // always include these to support shader compilation
      #include <d3dcompiler.h>
      #include <d3d11_4.h>
      #if DX11 // DirectX 11
         #include <dxgi1_6.h>
         #include <d3dcommon.h>
      #elif GL // OpenGL
         #define  GLEW_STATIC
         #define  GLEW_NO_GLU
         #include "../../../ThirdPartyLibs/GL/glew.h"
         #include "../../../ThirdPartyLibs/GL/wglew.h"
      #endif

      #undef GetComputerName
      #undef THIS
      #undef IGNORE
      #undef TRANSPARENT
      #undef OPAQUE
      #undef ERROR
      #undef UNIQUE_NAME
      #undef INPUT_MOUSE
      #undef INPUT_KEYBOARD
      #undef near
      #undef min
      #undef max
      #undef TokenType
      #undef UpdateWindow
      #undef TimeStamp
      #undef LOCK_WRITE
      #undef Font
      #undef FontPtr
      #undef RGB
      #undef ReplaceText
      #undef FindText
      #undef Yield
   #elif APPLE // Mac, iOS
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <string>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/statvfs.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/sysctl.h>
      #include <sys/socket.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <ifaddrs.h>
      #include <net/if.h>
      #include <net/if_dl.h>  
      #include <arpa/inet.h>
      #include <mach/mach.h>
      #include <mach/clock.h>
      #include <mach/mach_time.h>
      #include <libkern/OSAtomic.h>
      #include <OpenAL/al.h>
      #include <OpenAL/alc.h>
      #include <CoreFoundation/CoreFoundation.h>
      #include <StoreKit/StoreKit.h>
      #include <AudioToolbox/AudioToolbox.h>
      #if MAC
         #ifdef __x86_64__
            #include <smmintrin.h>
            #include <wmmintrin.h>
         #endif
         #include <CoreAudio/CoreAudio.h>
         #include <net/if_types.h>
         #include <IOKit/hid/IOHIDLib.h>
         #include <IOKit/pwr_mgt/IOPMLib.h>
         #include <Carbon/Carbon.h>
         #include <OpenGL/OpenGL.h>
         #ifdef __OBJC__
            #include <Cocoa/Cocoa.h>
         #endif
         #if GL
            #include <OpenGL/gl3.h>
            #include <OpenGL/gl3ext.h>
         #endif
      #elif IOS
         #define IFT_ETHER 0x06 // iOS does not have this in headers
         #include <UIKit/UIKit.h>
         #include <PhotosUI/PhotosUI.h>
         #include <QuartzCore/QuartzCore.h>
         #include <CoreMotion/CoreMotion.h>
         #include <CoreLocation/CoreLocation.h>
         #include <CoreHaptics/CoreHaptics.h>
         #include <CoreHaptics/CHHapticPatternPlayer.h>
         #include <AVFoundation/AVFoundation.h>
         #include <AdSupport/ASIdentifierManager.h>
         #include <GameController/GCController.h>
         #include <GameController/GCExtendedGamepad.h>
         #include "../../../ThirdPartyLibs/Chartboost/Headers/Chartboost.h"
         #if GL
            #include <OpenGLES/EAGL.h>
            #include <OpenGLES/EAGLDrawable.h>
            #include <OpenGLES/ES3/gl.h>
            #include <OpenGLES/ES3/glext.h>
         #endif
      #endif
      #undef Ptr
      #undef Point
      #undef Cell
      #undef Rect
      #undef Button
      #undef Cursor
      #undef FileInfo
      #undef TextStyle
      #undef gamma
      #undef ok
      #undef require
      #undef verify
      #undef check
      #undef MIN
      #undef MAX
      #undef ABS
   #elif LINUX // Linux
      #include <malloc.h>
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <cpuid.h>
      #include <pwd.h>
      #include <smmintrin.h>
      #include <wmmintrin.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/statvfs.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/socket.h>
      #include <sys/wait.h>
      #include <sys/resource.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <ifaddrs.h>
      #include <linux/if.h>
      #include <arpa/inet.h>
      #include <X11/Xatom.h>
      #include <X11/XKBlib.h>
      #include <X11/Xmu/WinUtil.h>
      #include <X11/Xcursor/Xcursor.h>
      #include <X11/extensions/xf86vmode.h>
      #include <X11/extensions/XInput2.h>
      #include <Xm/MwmUtil.h>
      #if GL
         #define GL_GLEXT_PROTOTYPES
         #define GLX_GLXEXT_PROTOTYPES
         #include <GL/gl.h>
         #include <GL/glext.h>
         #include <GL/glx.h>
      #endif
      #if OPEN_AL
         #include "AL/al.h"
         #include "AL/alc.h"
      #endif
      #undef LOCK_READ
      #undef LOCK_WRITE
      #undef PropertyNotify
      #undef Status
      #undef Convex
      #undef Button1
      #undef Button2
      #undef Button3
      #undef Button4
      #undef Button5
      #undef Bool
      #undef Time
      #undef Region
      #undef Window
      #undef Cursor
      #undef Font
      #undef None
      #undef Success
      #undef B32
      #undef B16
   #elif ANDROID // Android
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/socket.h>
      #include <sys/wait.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <linux/if.h>
      #include <arpa/inet.h>
      #include <android/log.h>
      #include <android/sensor.h>
      #include <android/asset_manager.h>
      #include <native_app_glue/android_native_app_glue.h>
      #if GL
         #include <EGL/egl.h>
         #include <EGL/eglext.h>
         #include <GLES3/gl3.h>
      #endif
      #if OPEN_SL
         #include <SLES/OpenSLES.h>
         #include <SLES/OpenSLES_Android.h>
      #endif
      #include <android/api-level.h> // needed for __ANDROID_API__
      #if __ANDROID_API__>=21
         #include <sys/statvfs.h>
      #endif
      #undef LOCK_READ
      #undef LOCK_WRITE
   #elif SWITCH // Nintendo Switch
      #define NN_DETAIL_ENABLE_LOG
      #include <unistd.h>
      #include <pthread.h>
      #include <dirent.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/statvfs.h>
      #include <sys/time.h>
      #include <sys/ioctl.h>
      #include <sys/socket.h>
      #include <sys/wait.h>
      #include <nn/nn_Log.h>
      #include <nn/time.h>
      #include <nn/account.h>
      #include <nn/fs.h>
      #include <nn/os.h>
      #include <nn/oe.h>
      #include <nn/oe/oe_DisableAutoSleepApi.h>
      #include <nn/hid.h>
      #include <nn/swkbd/swkbd_InlineKeyboardApi.h>
      #include <nn/vi.h>
      #include <nn/err.h>
      #include <nn/crypto.h>
      #include <nn/util/util_Uuid.h>
      #include <nn/socket/sys/cdefs.h>
      #include <netdb.h>
      #include <arpa/inet.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <nn/socket.h>
      #include <nn/socket/netinet6/in6.h>
      #include <nn/nifm.h>
      #include <nv/nv_MemoryManagement.h>
      #include <movie/Decoder.h>
      #include <movie/Common.h>
      #include <nn/audio.h>
      #if GL
         #include <EGL/egl.h>
         #include <EGL/eglext.h>
         #include <nn/gll.h>
      #endif
   #elif WEB // Web
      #include <emscripten.h>
      #include <emscripten/html5.h>
      #include <stdlib.h>
      #include <unistd.h>
      #include <pthread.h>
      #include <signal.h>
      #include <errno.h>
      #include <wctype.h>
      #include <dirent.h>
      #include <sys/types.h>
      #include <sys/stat.h>
      #include <sys/statvfs.h>
      #include <sys/time.h>
      #include <sys/mount.h>
      #include <sys/ioctl.h>
      #include <sys/socket.h>
      #include <sys/wait.h>
      #include <netdb.h>
      #include <netinet/in.h>
      #include <netinet/tcp.h>
      #include <arpa/inet.h>
      #include <X11/Xatom.h>
      #include <X11/Xlib.h>
      #include <X11/Xutil.h>
      #if GL
         #define  GL_GLEXT_PROTOTYPES
         #include <EGL/egl.h>
         #include <GLES3/gl3.h>
      #endif
      #if OPEN_AL
         #include "AL/al.h"
         #include "AL/alc.h"
      #endif
      #undef LOCK_READ
      #undef LOCK_WRITE
      #undef PropertyNotify
      #undef Status
      #undef Convex
      #undef Button1
      #undef Button2
      #undef Button3
      #undef Button4
      #undef Button5
      #undef Bool
      #undef Time
      #undef Region
      #undef Window
      #undef Cursor
      #undef Font
      #undef None
      #undef Success
   #endif
   /******************************************************************************/
   // INCLUDE THIRD PARTY LIBRARIES
   /******************************************************************************/
   // Compression
   #define SUPPORT_LZ4 1
   #if     SUPPORT_LZ4
      #if __clang__
         #define LZ4_DISABLE_DEPRECATE_WARNINGS // fails to compile without it
      #endif
      #include "../../../ThirdPartyLibs/LZ4/lz4.h"
      #include "../../../ThirdPartyLibs/LZ4/lz4hc.h"

      #define LZ4_BUF_SIZE      65536 // headers say that up to 64Kb need to be kept in memory !! don't change in the future because it would break any past compressed data !!
      #define LZ4_RING_BUF_SIZE (LZ4_BUF_SIZE*2) // if changing, then have to use LZ4_DECODER_RING_BUFFER_SIZE for decompression to keep compatibility with any past compressed data
   #endif

   #define SUPPORT_ZSTD 1
   #if     SUPPORT_ZSTD
      #define ZSTD_STATIC_LINKING_ONLY
      #include "../../../ThirdPartyLibs/Zstd/lib/zstd.h"
      #include "../../../ThirdPartyLibs/Zstd/lib/decompress/zstd_decompress_internal.h"
      #undef MIN
      #undef MAX
      #undef ERROR
      #undef KB
      #undef MB
      #undef GB
   #endif

   // Physics
   #if PHYSX // use PhysX
      #ifndef NDEBUG
         #define NDEBUG // specify Release config for PhysX
      #endif
      #define PX_PHYSX_STATIC_LIB
      #define PX_GENERATE_META_DATA // will fix "#include <ciso646>" error
      #include "../../../ThirdPartyLibs/PhysX/physx/include/PxPhysicsAPI.h"
      #include "../../../ThirdPartyLibs/PhysX/physx/source/geomutils/src/mesh/GuMeshData.h"     // needed for PX_MESH_VERSION
      #include "../../../ThirdPartyLibs/PhysX/physx/source/geomutils/src/convex/GuConvexMesh.h" // needed for PX_CONVEX_VERSION
      using namespace physx;
   #endif

   // always include Bullet to generate optimized PhysBody if needed
   #include "../../../ThirdPartyLibs/Bullet/lib/src/btBulletDynamicsCommon.h"

   // Recast/Detour path finding
   #include "../../../ThirdPartyLibs/Recast/Recast/Include/Recast.h"
   #include "../../../ThirdPartyLibs/Recast/Recast/Include/RecastAlloc.h"
   #include "../../../ThirdPartyLibs/Recast/Detour/Include/DetourNavMesh.h"
   #include "../../../ThirdPartyLibs/Recast/Detour/Include/DetourNavMeshQuery.h"
   #include "../../../ThirdPartyLibs/Recast/Detour/Include/DetourNavMeshBuilder.h"

   // SSL/TLS/HTTPS
   #define SUPPORT_MBED_TLS (!WEB)
   #if     SUPPORT_MBED_TLS
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/config.h"

      #ifdef MBEDTLS_PLATFORM_C
         #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/platform.h"
      #else
         #define mbedtls_time    time 
         #define mbedtls_time_t  time_t
         #define mbedtls_fprintf fprintf
         #define mbedtls_printf  printf
      #endif

      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/net_sockets.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/debug.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/ssl.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/entropy.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/ctr_drbg.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/error.h"
      #include "../../../ThirdPartyLibs/mbedTLS/lib/include/mbedtls/certs.h"
   #endif

   // DirectX Shader Compiler
   #define DX_SHADER_COMPILER (WINDOWS_OLD && X64)
   #if     DX_SHADER_COMPILER
      #include "../../../ThirdPartyLibs/DirectXShaderCompiler/include/dxc/dxcapi.h"
      #include "../../../ThirdPartyLibs/DirectXShaderCompiler/include/dxc/Support/microcom.h"
   #endif

   // SPIR-V Cross
   #define SPIRV_CROSS (WINDOWS_OLD && X64)
   #if     SPIRV_CROSS
      #include "../../../ThirdPartyLibs/SPIRV-Cross/include/spirv_cross_c.h"
      #include "../../../ThirdPartyLibs/SPIRV-Cross/include/spirv_glsl.hpp"
   #endif

   #if GL // define required constants which may be missing on some platforms
      #define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF
      #define GL_TEXTURE_MAX_ANISOTROPY     0x84FE
      #define GL_TEXTURE_LOD_BIAS           0x8501

      #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT             0x83F1
      #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT             0x83F2
      #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT             0x83F3
      #define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT       0x8C4D
      #define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT       0x8C4E
      #define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT       0x8C4F
      #define GL_COMPRESSED_RGBA_BPTC_UNORM                0x8E8C
      #define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT        0x8E8F
      #define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM          0x8E8D
      #define GL_ETC1_RGB8_OES                             0x8D64
      #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG          0x8C02
      #define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG          0x8C03
      #define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT    0x8A56
      #define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT    0x8A57
      #define GL_COMPRESSED_RGBA_ASTC_4x4_KHR              0x93B0
      #define GL_COMPRESSED_RGBA_ASTC_5x4_KHR              0x93B1
      #define GL_COMPRESSED_RGBA_ASTC_5x5_KHR              0x93B2
      #define GL_COMPRESSED_RGBA_ASTC_6x5_KHR              0x93B3
      #define GL_COMPRESSED_RGBA_ASTC_6x6_KHR              0x93B4
      #define GL_COMPRESSED_RGBA_ASTC_8x5_KHR              0x93B5
      #define GL_COMPRESSED_RGBA_ASTC_8x6_KHR              0x93B6
      #define GL_COMPRESSED_RGBA_ASTC_8x8_KHR              0x93B7
      #define GL_COMPRESSED_RGBA_ASTC_10x5_KHR             0x93B8
      #define GL_COMPRESSED_RGBA_ASTC_10x6_KHR             0x93B9
      #define GL_COMPRESSED_RGBA_ASTC_10x8_KHR             0x93BA
      #define GL_COMPRESSED_RGBA_ASTC_10x10_KHR            0x93BB
      #define GL_COMPRESSED_RGBA_ASTC_12x10_KHR            0x93BC
      #define GL_COMPRESSED_RGBA_ASTC_12x12_KHR            0x93BD
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR      0x93D0
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR      0x93D1
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR      0x93D2
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR      0x93D3
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR      0x93D4
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR      0x93D5
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR      0x93D6
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR      0x93D7
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR     0x93D8
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR     0x93D9
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR     0x93DA
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR    0x93DB
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR    0x93DC
      #define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR    0x93DD
      #define GL_ALPHA8                                    0x803C
      #define GL_LUMINANCE8                                0x8040
      #define GL_LUMINANCE8_ALPHA8                         0x8045
      #define GL_BGR                                       0x80E0
      #define GL_BGRA                                      0x80E1
      #define GL_COMPRESSED_RGB8_ETC2                      0x9274
      #define GL_COMPRESSED_SRGB8_ETC2                     0x9275
      #define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2  0x9276
      #define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
      #define GL_COMPRESSED_RGBA8_ETC2_EAC                 0x9278
      #define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC          0x9279
      #define GL_COMPRESSED_R11_EAC                        0x9270
      #define GL_COMPRESSED_SIGNED_R11_EAC                 0x9271
      #define GL_COMPRESSED_RG11_EAC                       0x9272
      #define GL_COMPRESSED_SIGNED_RG11_EAC                0x9273
      #define GL_COMPRESSED_RED_RGTC1                      0x8DBB
      #define GL_COMPRESSED_RG_RGTC2                       0x8DBD
      #define GL_COMPRESSED_SIGNED_RED_RGTC1               0x8DBC
      #define GL_COMPRESSED_SIGNED_RG_RGTC2                0x8DBE
      #define GL_LUMINANCE                                 0x1909
   #endif

   // Other
   #define SUPPORT_FACEBOOK (ANDROID)// || (IOS && !IOS_SIMULATOR))
   #if     SUPPORT_FACEBOOK && IOS
      #include <FBSDKCoreKit/FBSDKCoreKit.h>
      #include <FBSDKLoginKit/FBSDKLoginKit.h>
      #include <FBSDKShareKit/FBSDKShareKit.h>
   #endif

   #include <algorithm> // must be after PhysX or compile errors will show on Android
   /******************************************************************************/
   // Finish including headers - this needs to be included after all headers
   #include "../../../ThirdPartyLibs/end.h"
   /******************************************************************************/
#else
#if WINDOWS
   #ifndef __PLACEMENT_NEW_INLINE
      #define __PLACEMENT_NEW_INLINE
      inline void* __cdecl operator new   (size_t, void *where) {return where;}
      inline void  __cdecl operator delete(void *, void *     )throw() {}
   #endif
   #undef GetComputerName
   #undef TRANSPARENT
   #undef ERROR
   #undef INPUT_MOUSE
   #undef INPUT_KEYBOARD
   #undef min
   #undef max
   #define _ALLOW_RTCc_IN_STL
   #include <vcruntime_string.h> // needed for 'memcpy' (inside "string.h")
   #include <xmmintrin.h> // needed for 'RSqrtSimd'
#else
   #include <new>
   #include <stdint.h>
   #include <stddef.h>
   #include <string.h> // needed for 'memcpy'
#endif
#include <math.h>
#include <typeinfo>
#include <type_traits> // needed for 'std::enable_if', 'std::is_enum'
#if ANDROID
   #include <android/api-level.h> // needed for __ANDROID_API__
#endif
#endif
#if !WINDOWS_NEW
namespace std{typedef decltype(nullptr) nullptr_t;}
#endif
/******************************************************************************/
