#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/98fc376a/Engine_Builder.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m64 -std=c++17 -fshort-wchar -fpermissive -ffast-math -ffriend-injection -fms-extensions -fno-pic -fno-pie -Wno-int-to-pointer-cast -Wno-invalid-offsetof -Wno-comment -Wno-parentheses -Wno-switch -Wno-null-dereference -Wno-empty-body -Wno-address-of-temporary -Wno-return-type-c-linkage -Wno-dynamic-class-memaccess -include ../stdafx.h
CXXFLAGS=-m64 -std=c++17 -fshort-wchar -fpermissive -ffast-math -ffriend-injection -fms-extensions -fno-pic -fno-pie -Wno-int-to-pointer-cast -Wno-invalid-offsetof -Wno-comment -Wno-parentheses -Wno-switch -Wno-null-dereference -Wno-empty-body -Wno-address-of-temporary -Wno-return-type-c-linkage -Wno-dynamic-class-memaccess -include ../stdafx.h

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../ThirdPartyLibs/PhysX/PhysX/Bin/linux64 -L../../ThirdPartyLibs/PhysX/PhysX/Lib/linux64 -L../../ThirdPartyLibs/PhysX/PxShared/bin/linux64 -L../../ThirdPartyLibs/PhysX/PxShared/lib/linux64 -L../../ThirdPartyLibs/PhysX/Dummy/Linux -Wl,-rpath,'Bin' -Wl,-rpath,'../Editor/Bin' ../../Engine/Linux/dist/Release/GNU-Linux/Engine.a ../../ThirdPartyLibs/Recast/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Png/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Vorbis/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Theora/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/SQLite/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Webp/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/LZ4/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Ogg/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Tiff/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Waifu2x/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/FreeType/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Bullet/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Snappy/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/LZMA/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/FBX/Linux/libfbxsdk.a ../../ThirdPartyLibs/JpegTurbo/Linux/libturbojpeg.a ../../ThirdPartyLibs/Flac/Linux/libFLAC-static.a ../../ThirdPartyLibs/Opus/Linux/libopusfile.a ../../ThirdPartyLibs/Opus/Linux/libopus.a ../../ThirdPartyLibs/VP/Linux/libvpx.a ../../ThirdPartyLibs/LZHAM/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Zstd/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/FDK-AAC/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/mbedTLS/Linux/dist/Release/GNU-Linux/liblinux.a ../../ThirdPartyLibs/Xml2/Linux/libxml2.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysX_static_64.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXCharacterKinematic_static_64.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXCooking_static_64.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXExtensions_static_64.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXFoundation_static_64.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXPvdSDK_static_64.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXVehicle_static_64.a ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXCommon_static_64.a ../../ThirdPartyLibs/AVIF/Linux/libavif.a ../../ThirdPartyLibs/AOM/Linux/libaom.a -lX11 -lXmu -lXxf86vm -lXinerama -lXrandr -lrt -lXi -lXcursor -lGL -lopenal -lz -lodbc -ludev -lpthread -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../Project

../Project: ../../Engine/Linux/dist/Release/GNU-Linux/Engine.a

../Project: ../../ThirdPartyLibs/Recast/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Png/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Vorbis/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Theora/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/SQLite/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Webp/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/LZ4/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Ogg/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Tiff/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Waifu2x/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/FreeType/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Bullet/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Snappy/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/LZMA/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/FBX/Linux/libfbxsdk.a

../Project: ../../ThirdPartyLibs/JpegTurbo/Linux/libturbojpeg.a

../Project: ../../ThirdPartyLibs/Flac/Linux/libFLAC-static.a

../Project: ../../ThirdPartyLibs/Opus/Linux/libopusfile.a

../Project: ../../ThirdPartyLibs/Opus/Linux/libopus.a

../Project: ../../ThirdPartyLibs/VP/Linux/libvpx.a

../Project: ../../ThirdPartyLibs/LZHAM/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Zstd/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/FDK-AAC/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/mbedTLS/Linux/dist/Release/GNU-Linux/liblinux.a

../Project: ../../ThirdPartyLibs/Xml2/Linux/libxml2.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysX_static_64.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXCharacterKinematic_static_64.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXCooking_static_64.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXExtensions_static_64.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXFoundation_static_64.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXPvdSDK_static_64.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXVehicle_static_64.a

../Project: ../../ThirdPartyLibs/PhysX/physx/bin/linux.clang/release/libPhysXCommon_static_64.a

../Project: ../../ThirdPartyLibs/AVIF/Linux/libavif.a

../Project: ../../ThirdPartyLibs/AOM/Linux/libaom.a

../Project: ${OBJECTFILES}
	${MKDIR} -p ..
	clang++ -o ../Project ${OBJECTFILES} ${LDLIBSOPTIONS} -static-libstdc++ -nopie -s

${OBJECTDIR}/_ext/98fc376a/Engine_Builder.o: ../Source/Engine\ Builder.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/98fc376a
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -DDEBUG=0 -I.. -I../../ThirdPartyLibs/PhysX/PhysX/Include -I../../ThirdPartyLibs/PhysX/PhysX/Include/cloth -I../../ThirdPartyLibs/PhysX/PhysX/Include/common -I../../ThirdPartyLibs/PhysX/PhysX/Include/extensions -I../../ThirdPartyLibs/PhysX/PhysX/Include/geometry -I../../ThirdPartyLibs/PhysX/PhysX/Include/vehicle -I../../ThirdPartyLibs/PhysX/PhysX/Source/Common/src -I../../ThirdPartyLibs/PhysX/PhysX/Source/GeomUtils/src -I../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXExtensions/src/serialization/Xml -I../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXMetaData/core/include -I../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXMetaData/extensions/include -I../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src -I../../ThirdPartyLibs/PhysX/PhysX/Source/PhysXVehicle/src/PhysXMetaData/include -I../../ThirdPartyLibs/PhysX/PxShared/include -I../../ThirdPartyLibs/PhysX/PxShared/src/foundation/include -I../../ThirdPartyLibs/PhysX/PxShared/src/pvd/include -I../../ThirdPartyLibs/Theora/include -I../../ThirdPartyLibs/FreeType/include -I../../ThirdPartyLibs/Vorbis/include -I../../ThirdPartyLibs/Ogg/include -I../../ThirdPartyLibs/FBX -I../../ThirdPartyLibs/Bullet/lib/src -I../../ThirdPartyLibs -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/98fc376a/Engine_Builder.o ../Source/Engine\ Builder.cpp

../stdafx.h.pch: ../stdafx.h
	${MKDIR} -p ..
	@echo Performing Custom Build Step
	clang++ -x c++-header ../stdafx.h -o ../stdafx.h.pch $(CXXFLAGS) -O3 -DDEBUG=0 -I.. -I../H/_ -I../../ThirdPartyLibs -I../../ThirdPartyLibs/Bullet/lib/src -I../../ThirdPartyLibs/FBX -I../../ThirdPartyLibs/FreeType/lib/include -I../../ThirdPartyLibs/Ogg/include -I../../ThirdPartyLibs/Opus/lib/include -I../../ThirdPartyLibs/Opus/file/include -I../../ThirdPartyLibs/Theora/include -I../../ThirdPartyLibs/Vorbis/include -I../../ThirdPartyLibs/VP/libvpx/third_party/libwebm -I../../ThirdPartyLibs/FDK-AAC/lib/libAACdec/include -I../../ThirdPartyLibs/FDK-AAC/lib/libAACenc/include -I../../ThirdPartyLibs/FDK-AAC/lib/libFDK/include -I../../ThirdPartyLibs/FDK-AAC/lib/libMpegTPDec/include -I../../ThirdPartyLibs/FDK-AAC/lib/libMpegTPEnc/include -I../../ThirdPartyLibs/FDK-AAC/lib/libPCMutils/include -I../../ThirdPartyLibs/FDK-AAC/lib/libSBRdec/include -I../../ThirdPartyLibs/FDK-AAC/lib/libSBRenc/include -I../../ThirdPartyLibs/FDK-AAC/lib/libSYS/include -I../../ThirdPartyLibs/PhysX/physx/include -I../../ThirdPartyLibs/PhysX/pxshared/include -I../../ThirdPartyLibs/PhysX/physx/source/common/src -I../../ThirdPartyLibs/PhysX/physx/source/foundation/include -I../../ThirdPartyLibs/PhysX/physx/source/geomutils/src -fms-extensions -std=c++17

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../stdafx.h.pch

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
