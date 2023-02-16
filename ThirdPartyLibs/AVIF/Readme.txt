Cmake
/******************************************************************************/
WINDOWS:
AVIF_CODEC_AOM=1
AVIF_CODEC_AOM_DECODE=0
AVIF_CODEC_AOM_ENCODE=1
AVIF_CODEC_DAV1D=1
AVIF_ENABLE_GTEST=0
AVIF_ENABLE_WERROR=0
BUILD_SHARED_LIBS=0
CMAKE_C_FLAGS_RELEASE /MD -> /MT

AOM_INCLUDE_DIR=C:\Esenthel\ThirdPartyLibs\AOM\lib
AOM_LIBRARY=C:\Esenthel\ThirdPartyLibs\AOM\Windows\Release\aom.lib

DAV1D_INCLUDE_DIR = C:\Esenthel\ThirdPartyLibs\dav1d\lib\include
DAV1D_LIBRARY = C:\Esenthel\ThirdPartyLibs\dav1d\Windows\src\libdav1d.lib
/******************************************************************************/
ANDROID
AVIF_CODEC_AOM=1
AVIF_CODEC_AOM_DECODE=1
AVIF_CODEC_AOM_ENCODE=1
AVIF_ENABLE_GTEST=0
AVIF_ENABLE_WERROR=0
BUILD_SHARED_LIBS=0
CMAKE_C_FLAGS_RELEASE /MD -> /MT

AOM_INCLUDE_DIR=C:\Esenthel\ThirdPartyLibs\AOM\lib
AOM_LIBRARY=C:\Esenthel\ThirdPartyLibs\AOM\Windows\Release\aom.lib

VS
new targets Android
MultiProcessor Compilation=No
C/C++ \ Command Line = delete all
/******************************************************************************/
MAC
AVIF_CODEC_AOM=1
AVIF_CODEC_AOM_DECODE=1
AVIF_CODEC_AOM_ENCODE=1
AVIF_ENABLE_GTEST=0
AVIF_ENABLE_WERROR=0
BUILD_SHARED_LIBS=0
AOM_INCLUDE_DIR=/Applications/Esenthel/ThirdPartyLibs/AOM/lib
AOM_LIBRARY=/Applications/Esenthel/ThirdPartyLibs/AOM/Mac/Release/libaom.a
/******************************************************************************/
iOS
open xcode project
add new Target "iOS"
architectures=Standard Architectures
Base SDK=iOS
supported platforms=iOS
product name=avif-iOS
/******************************************************************************/
LINUX
AVIF_CODEC_AOM=1
AVIF_CODEC_AOM_DECODE=1
AVIF_CODEC_AOM_ENCODE=1
AVIF_ENABLE_GTEST=0
AVIF_ENABLE_WERROR=0
BUILD_SHARED_LIBS=0
AOM_INCLUDE_DIR=/home/greg/Esenthel/ThirdPartyLibs/AOM/lib
AOM_LIBRARY=/home/greg/Esenthel/ThirdPartyLibs/AOM/Linux/libaom.a
cd Linux
make
copy files back to Windows
Clean.bat
Extract Linux Objs.bat
