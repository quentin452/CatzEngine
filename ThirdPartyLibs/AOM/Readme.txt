Cmake
/******************************************************************************/
WINDOWS:
CMAKE_CXX_FLAGS_RELEASE /MD -> /MT
CMAKE_C_FLAGS_RELEASE /MD -> /MT
CONFIG_SHARED=0
ENABLE_DOCS=0
ENABLE_EXAMPLES=0
ENABLE_TESTDATA=0
ENABLE_TESTS=0
ENABLE_TOOLS=0
ENABLE_VSX=0
/******************************************************************************/
ANDROID:
C:\Esenthel\ThirdPartyLibs\AOM\lib
C:/Esenthel/ThirdPartyLibs/AOM/Android
select ARM64
ANDROID_ABI=arm64-v8a
ANDROID_PLATFORM=android-24
CMAKE_CXX_FLAGS_RELEASE /MD -> /MT
CMAKE_C_FLAGS_RELEASE /MD -> /MT
CONFIG_SHARED=0
ENABLE_DOCS=0
ENABLE_EXAMPLES=0
ENABLE_TESTDATA=0
ENABLE_TESTS=0
ENABLE_TOOLS=0
ENABLE_VSX=0
ENABLE_AVX=0
ENABLE_AVX2=0
ENABLE_MMX=0
ENABLE_SSE*=0
ENABLE_NEON=1
HAVE_AVX=0
HAVE_AVX2=0
HAVE_MMX=0
HAVE_NEON=1
HAVE_SSE*=0
generate
create Android platform
disable General \ Multiprocessor Compilation

Edit Android\config\aom_config.h
#define ARCH_ARM 1
#define ARCH_PPC 0
#define ARCH_X86 0
#define ARCH_X86_64 0
#define HAVE_AVX 0
#define HAVE_AVX2 0
#define HAVE_FEXCEPT 0
#define HAVE_MMX 0
#define HAVE_NEON 1
#define HAVE_PTHREAD_H 0
#define HAVE_SSE 0
#define HAVE_SSE2 0
#define HAVE_SSE3 0
#define HAVE_SSE4_1 0
#define HAVE_SSE4_2 0
#define HAVE_SSSE3 0

update C:\Esenthel\ThirdPartyLibs\AOM\Android\config\*.h
to replace:
//#include "aom_ports/x86.h"
static void setup_rtcd_internal(void)
{
    //int flags = x86_simd_caps();

    //(void)flags;

}
don't include:
aom_pc_dummy.o
ducky_encode.o
/******************************************************************************/
MAC:
Xcode
CONFIG_SHARED=0
ENABLE_DOCS=0
ENABLE_EXAMPLES=0
ENABLE_TESTDATA=0
ENABLE_TESTS=0
ENABLE_TOOLS=0
ENABLE_VSX=0
/******************************************************************************/
iOS:
Unix Makefiles
CONFIG_SHARED=0
ENABLE_DOCS=0
ENABLE_EXAMPLES=0
ENABLE_TESTDATA=0
ENABLE_TESTS=0
ENABLE_TOOLS=0
ENABLE_VSX=0
HAVE_NEON=1

cd /Applications/Esenthel/ThirdPartyLibs/AOM/iOS
make
/******************************************************************************/
Linux:
Unix Makefiles
CONFIG_SHARED=0
ENABLE_DOCS=0
ENABLE_EXAMPLES=0
ENABLE_TESTDATA=0
ENABLE_TESTS=0
ENABLE_TOOLS=0
ENABLE_VSX=0

cd /Esenthel/ThirdPartyLibs/AOM/Linux
make
copy files back to Windows
Clean.bat
Extract Linux Objs.bat
