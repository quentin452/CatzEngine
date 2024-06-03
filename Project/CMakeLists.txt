set(PROJECT_NAME Project)

################################################################################
# Source groups
################################################################################
set(H
    "stdafx.cpp"
    "stdafx.h"
)
source_group("H" FILES ${H})

set(Src
    "Source/Engine Builder.cpp"
)
source_group("Src" FILES ${Src})

set(ALL_FILES
    ${H}
    ${Src}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    use_props(${PROJECT_NAME} DebugDX11            "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} DebugGL              "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} DebugUniversalDX11   "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} ReleaseDX11          "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} ReleaseGL            "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} ReleaseUniversalDX11 "ImportNintendoSdk.cmake")
endif()

set(ROOT_NAMESPACE Project)

################################################################################
# Target name
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUGDX11            "libProject"
        TARGET_NAME_DEBUGGL              "libProject"
        TARGET_NAME_DEBUGUNIVERSALDX11   "libProject"
        TARGET_NAME_RELEASEDX11          "libProject"
        TARGET_NAME_RELEASEGL            "libProject"
        TARGET_NAME_RELEASEUNIVERSALDX11 "libProject"
    )
endif()
################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/$ENV{MSBuildProjectName}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/$ENV{MSBuildProjectName}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/$ENV{MSBuildProjectName}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/Android/Debug/$ENV{PlatformTarget}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/Android/Debug/$ENV{PlatformTarget}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/Android/Debug/$ENV{PlatformTarget}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/Android/Release/$ENV{PlatformTarget}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/Android/Release/$ENV{PlatformTarget}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/Android/Release/$ENV{PlatformTarget}/"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        PDB_OUTPUT_DIRECTORY                      "${OUTPUT_DIRECTORY}"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION                      "FALSE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
endif()
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        MultiThreaded
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm;"
        "$ENV{VS_NdkRoot}/sources/android"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /W3;
            /fp:fast;
            /GR;
            /wd4244;
            /wd4312;
            /wd4521;
            /J;
            /permissive;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:wchar_t
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /ZI
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /ZI
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /ZI
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /arch:SSE2;
            /Gd;
            /W3;
            /fp:fast;
            /GR;
            /wd4244;
            /wd4312;
            /wd4521;
            /J;
            /permissive;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:wchar_t
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /W3;
            /fp:fast;
            /GR;
            /wd4244;
            /wd4312;
            /wd4521;
            /J;
            /permissive;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:wchar_t
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /Od;
                /Ob0;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /Od;
                /Ob0;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /Od;
                /Ob0;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /O2;
                /Ob1;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            $<$<CONFIG:ReleaseGL>:
                /O2;
                /Ob1;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /O2;
                /Ob1;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            /MP;
            /GL-;
            /Oi;
            /GF;
            /GT;
            /Oy;
            /Gd;
            /W3;
            /Ot;
            /fp:fast;
            /GR;
            -Wno-invalid-offsetof;
            -Wno-comment;
            -Wno-parentheses;
            -Wno-switch;
            -Wno-empty-body;
            -Wno-address-of-temporary;
            -Wno-constant-logical-operand;
            -fms-extensions;
            -std=c++17;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:wchar_t
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /W3;
            /fp:fast;
            /GR;
            /wdcomment;
            /wdparentheses;
            /wdswitch;
            /wdaddress-of-temporary;
            /wdnull-dereference;
            /wdint-to-void-pointer-cast;
            /wdoverloaded-virtual;
            /wdunused-local-typedef;
            /wdmisleading-indentation;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:wchar_t
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /W3;
            /fp:fast;
            /GR;
            /wd4244;
            /wd4312;
            /wd4521;
            -fno-delete-null-pointer-checks;
            -Wno-comment;
            -Wno-parentheses;
            -Wno-misleading-indentation;
            -Wno-int-to-void-pointer-cast;
            -Wno-unused-local-typedef;
            -Wno-overloaded-virtual;
            -Wno-switch;
            -Wno-undefined-bool-conversion;
            -Wno-null-dereference;
            -Wno-missing-braces;
            -Wno-invalid-noreturn;
            -Wno-unused-function;
            -Wno-unused-variable;
            -Wno-unused-const-variable;
            -Wno-address-of-temporary;
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Zc:wchar_t
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugGL>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /DEBUG;
                /SAFESEH:NO;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:ReleaseDX11>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseGL>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /OPT:REF;
                /OPT:ICF
            >
            /NOLOGO;
            /MACHINE:X64;
            /INCREMENTAL:NO;
            /IGNORE:4078
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF;
                /INCREMENTAL
            >
            $<$<CONFIG:DebugGL>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF;
                /INCREMENTAL
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /DEBUG;
                /SAFESEH:NO;
                /OPT:NOREF;
                /OPT:NOICF;
                /INCREMENTAL
            >
            $<$<CONFIG:ReleaseDX11>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ReleaseGL>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /NOLOGO;
            /MACHINE:X86;
            /IGNORE:4078
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF;
                /INCREMENTAL
            >
            $<$<CONFIG:DebugGL>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF;
                /INCREMENTAL
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /DEBUG;
                /SAFESEH:NO;
                /OPT:NOREF;
                /OPT:NOICF;
                /INCREMENTAL
            >
            $<$<CONFIG:ReleaseDX11>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ReleaseGL>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /OPT:REF;
                /OPT:ICF;
                /INCREMENTAL:NO
            >
            /NOLOGO;
            /IGNORE:4078
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /DEBUG;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugGL>:
                /DEBUG;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /DEBUG;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:ReleaseDX11>:
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseGL>:
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /OPT:REF;
                /OPT:ICF
            >
            /NOLOGO;
            /SUBSYSTEM:WINDOWS;
            /NXCOMPAT:NO;
            /DYNAMICBASE;
            /INCREMENTAL:NO;
            /IGNORE:4078
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugGL>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /DEBUG;
                /SAFESEH:NO;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:ReleaseDX11>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseGL>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /NODEFAULTLIB:libcmt;
                /NODEFAULTLIB:libcpmt;
                /OPT:REF;
                /OPT:ICF
            >
            /NOLOGO;
            /INCREMENTAL:NO
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugGL>:
                /DEBUG;
                /SAFESEH:NO;
                /SUBSYSTEM:WINDOWS;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /DEBUG;
                /SAFESEH:NO;
                /OPT:NOREF;
                /OPT:NOICF
            >
            $<$<CONFIG:ReleaseDX11>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseGL>:
                /SUBSYSTEM:WINDOWS;
                /OPT:REF;
                /OPT:ICF
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /OPT:REF;
                /OPT:ICF
            >
            /NOLOGO;
            /INCREMENTAL:NO
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
# Link with other targets.
target_link_libraries(${PROJECT_NAME} PRIVATE
    Engine
)

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:DebugDX11>:"
            "../Engine/EngineDebug64DX11;"
            "d3d11;"
            "d3d12;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "../Engine/EngineDebug64GL;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject;"
            "opengl32"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "../Engine/EngineDebugUniversal64DX11;"
            "WindowsApp"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "../Engine/Engine64DX11;"
            "d3d11;"
            "d3d12;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "../Engine/Engine64GL;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject;"
            "opengl32"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "../Engine/EngineUniversal64DX11;"
            "WindowsApp"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:DebugDX11>:"
            "../Engine/EngineDebug32DX11;"
            "d3d11;"
            "d3d12;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "../Engine/EngineDebug32GL;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject;"
            "opengl32"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "../Engine/EngineDebugUniversal32DX11;"
            "WindowsApp"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "../Engine/Engine32DX11;"
            "d3d11;"
            "d3d12;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "../Engine/Engine32GL;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject;"
            "opengl32"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "../Engine/EngineUniversal32DX11;"
            "WindowsApp"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "$<$<CONFIG:DebugDX11>:"
            "../Engine/EngineDebugArm32DX11;"
            "d3d11;"
            "d3d12;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "../Engine/EngineDebugArm32GL;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject;"
            "opengl32"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "../Engine/EngineDebugUniversalArm32DX11;"
            "WindowsApp"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "../Engine/EngineArm32DX11;"
            "d3d11;"
            "d3d12;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "../Engine/EngineArm32GL;"
            "d3dcompiler;"
            "dinput8;"
            "dsound;"
            "dxgi;"
            "dxguid;"
            "Xinput9_1_0;"
            "xaudio2;"
            "imm32;"
            "wininet;"
            "winmm;"
            "libcmt;"
            "libcpmt;"
            "libucrt;"
            "libvcruntime;"
            "psapi;"
            "ws2_32;"
            "oldnames;"
            "rpcrt4;"
            "version;"
            "iphlpapi;"
            "Secur32;"
            "Setupapi;"
            "Dbghelp;"
            "comsupp;"
            "concrt;"
            "Mscms;"
            "Advapi32;"
            "User32;"
            "Shell32;"
            "Ole32;"
            "OleAut32;"
            "Gdi32;"
            "uuid;"
            "odbc32;"
            "Winspool;"
            "runtimeobject;"
            "opengl32"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "../Engine/EngineUniversalArm32DX11;"
            "WindowsApp"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "libz.a;"
        "libnn_gll.a"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugUniversalDX11>:"
            "$ENV{VCInstallDir}/lib/store/amd64;"
            "$ENV{VCInstallDir}/lib/amd64"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "$ENV{VCInstallDir}/lib/store/amd64;"
            "$ENV{VCInstallDir}/lib/amd64"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugUniversalDX11>:"
            "$ENV{VCInstallDir}/lib/store;"
            "$ENV{VCInstallDir}/lib"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "$ENV{VCInstallDir}/lib/store;"
            "$ENV{VCInstallDir}/lib"
        ">"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    target_link_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugUniversalDX11>:"
            "$ENV{VCInstallDir}/lib/store/arm;"
            "$ENV{VCInstallDir}/lib/arm"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "$ENV{VCInstallDir}/lib/store/arm;"
            "$ENV{VCInstallDir}/lib/arm"
        ">"
    )
endif()

