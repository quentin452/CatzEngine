# Currently support Release DX11 X64 AND UNICODE ONLY (cannot compile without UNICODE IN SOME WAY)

cmake_minimum_required(VERSION 3.11.0)

# Définir la macro use_props
macro(use_props target config file)
    get_filename_component(PROP_DIR "${file}" PATH)
    include_directories("${PROP_DIR}/include")
    link_directories("${PROP_DIR}/lib")
endmacro()

# Compile using Parallel
add_compile_options(/MP)

set(NAME CatzEngineEditor)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_DX11 "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

project(${NAME} VERSION 0.1.0 LANGUAGES C CXX)
# Définition du standard C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DLL_DIR "${CMAKE_SOURCE_DIR}/!thirdParty/dlls")
set(LIB_DIR "${CMAKE_SOURCE_DIR}/!thirdParty/libs")
set(MANIFEST_DIR "${CMAKE_SOURCE_DIR}/!thirdParty/manifest")
set(LIBRAIRIES_DIR "${CMAKE_SOURCE_DIR}/ThirdPartyLibs")

file(GLOB DLL_LIST "${DLL_DIR}/*.dll")
file(GLOB_RECURSE MANIFEST_FILES "${MANIFEST_DIR}/*.manifest")

set(ALL_INCLUDE_DIR
    "${LIBRAIRIES_DIR}"
)

# Enable LTO Optimizations(Link-Time Optimization) 
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

message(STATUS "using ${CMAKE_GENERATOR}")

project(${NAME} VERSION 0.23.0)

include_directories(external)

file(GLOB ALL_LIBRARIES_DIR "${LIB_DIR}/*.lib")

if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Release_DX11" CACHE STRING "Configuration types" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release_DX11")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release_DX11")
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/Engine/Source/**/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}\\Engine\\Source\\*\\*\\*Platforms\\.*")
list(APPEND SOURCES "${PROJECT_SOURCE_DIR}/Engine/Source/Platforms/Windows/Windows Platform.cpp")
endif()

include(CTest)
enable_testing()

# Définition de la cible
add_executable(${NAME} ${SOURCES})

if(CMAKE_BUILD_TYPE STREQUAL "Release_DX11")
# Options de compilation
target_compile_options(${NAME} PRIVATE
  /O2                    # Optimisation complète
  /Oi                    # Activation de l'expansion de fonction en ligne
  /Ob1                   # Activé pour la taille ou la vitesse
  /Oy                    # Suppression des pointeurs de cadre
  /GT                    # Activation des optimisations sûres pour les fibres
  /Gy                    # Optimisation du programme entier
  /J                     # Multi-threading pour les fonctions intrinsèques
  /permissive-           # Respect strict des normes du langage C++
  /std:c++latest         # Norme C++ la plus récente
  /EHsc                  # Activation de la gestion des exceptions
  /MD                    # Utilisation de la bibliothèque multi-threaded
  /W3                    # Niveau de sensibilité aux avertissements 3
  /wd4018 /wd4244 /wd4312 /wd4521  # Désactiver des avertissements spécifiques
)

# Options de liaison
target_link_options(${NAME} PRIVATE
  /IGNORE:4078           # Ignorer certains avertissements de liaison
  /SUBSYSTEM:WINDOWS     # Sous-système cible : Windows
  /LARGEADDRESSAWARE     # Activer la prise en charge des adresses larges
)
# Dépendances supplémentaires
target_link_libraries(${NAME} PRIVATE
  Engine64DX11.lib
  d3d11.lib
  d3d12.lib
  d3dcompiler.lib
  dinput8.lib
  dsound.lib
  dxgi.lib
  dxguid.lib
  Xinput9_1_0.lib
  xaudio2.lib
  winmm.lib
  wininet.lib
  ws2_32.lib
  imm32.lib
  psapi.lib
  rpcrt4.lib
  version.lib
  libcmt.lib
  libcpmt.lib
  libucrt.lib
  libvcruntime.lib
  oldnames.lib
  iphlpapi.lib
  secur32.lib
  Setupapi.lib
  Dbghelp.lib
  comsupp.lib
  concrt.lib
  Mscms.lib
  Advapi32.lib
  User32.lib
  Shell32.lib
  Ole32.lib
  OleAut32.lib
  Gdi32.lib
  uuid.lib
  odbc32.lib
  Winspool.lib
  runtimeobject.lib
)

# Autres configurations
set_target_properties(${NAME} PROPERTIES
  LINK_FLAGS "/ENTRY:wWinMainCRTStartup"
)

# Fichier manifeste
#configure_file(Assets/WindowsManifest.xml WindowsManifest.xml COPYONLY)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

if(SMAA_HLSL_4_1)
    add_definitions(-DSMAA_HLSL_4_1=1)
endif()

if(DX11)
    add_definitions(-DDX11=1)
endif()

if(UNICODE)
    add_definitions(-DUNICODE=1)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/Engine"
    ${ALL_INCLUDE_DIR}
    "${ALL_INCLUDE_DIR}/PhysX/pxshared/include"
    "${ALL_INCLUDE_DIR}/PhysX/physx/include"
    "${ALL_INCLUDE_DIR}/PhysX/physx/source/foundation/include"
    "${ALL_INCLUDE_DIR}/PhysX/physx/source/common/src"
    "${ALL_INCLUDE_DIR}/PhysX/physx/source/geomutils/src"
    "${ALL_INCLUDE_DIR}/JpegXL/lib/lib/include"
)

# Copy DLLs after build if they don't exist already
foreach(DLL ${DLL_LIST})
    file(TO_NATIVE_PATH "${DLL}" NATIVE_DLL_PATH)
    get_filename_component(DLL_NAME "${DLL}" NAME)
    set(DESTINATION_PATH "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${DLL_NAME}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${NATIVE_DLL_PATH}"
        "${DESTINATION_PATH}"
        VERBATIM
        )
endforeach()

set(MANIFEST_DIR "${CMAKE_SOURCE_DIR}/!libsanddlls/manifest")

# Récupérer une liste de tous les fichiers manifest
file(GLOB_RECURSE MANIFEST_FILES "${MANIFEST_DIR}/*.manifest")

# Copier chaque fichier manifest dans le répertoire de build correspondant au niveau de build
foreach(MANIFEST_FILE ${MANIFEST_FILES})
    get_filename_component(MANIFEST_NAME ${MANIFEST_FILE} NAME)
    set(DESTINATION "${CMAKE_BINARY_DIR}/$<CONFIG>/${MANIFEST_NAME}")
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${MANIFEST_FILE}
        ${DESTINATION}
        COMMENT "Copying ${MANIFEST_NAME} to $<CONFIG> directory"
    )
endforeach()

set(dest_dir "${CMAKE_BINARY_DIR}/$<CONFIG>/resources")

add_custom_target(copy_resources ALL
    COMMAND echo start copying models and shaders at && date /T && echo The current time is: %TIME%
    COMMAND ${CMAKE_COMMAND} -E make_directory ${dest_dir}/models
    COMMAND ${CMAKE_COMMAND} -E make_directory ${dest_dir}/shaders
    COMMAND ${CMAKE_COMMAND} -E make_directory ${dest_dir}/tex
    COMMAND for /R ${CMAKE_SOURCE_DIR}/resources/models %%G in (*.obj *.png *.meta *.mtl *.png.meta) do (
        ${CMAKE_COMMAND} -E copy_if_different %%G ${dest_dir}/models/%%~nxG
    )
    COMMAND for /R ${CMAKE_SOURCE_DIR}/resources/shaders %%G in (*.frag *.spv *.vert) do (
        ${CMAKE_COMMAND} -E copy_if_different %%G ${dest_dir}/shaders/%%~nxG
    )
    COMMAND for /R ${CMAKE_SOURCE_DIR}/resources/tex %%G in (*.jpg *.png) do (
      ${CMAKE_COMMAND} -E copy_if_different %%G ${dest_dir}/tex/%%~nxG
  )
    COMMAND echo Copy completed at && date /T && echo The current time is: %TIME%
    COMMAND echo Start removing files at && date /T && echo The current time is: %TIME%
    COMMAND for /R ${dest_dir}/models %%G in (*.obj *.png *.meta *.mtl *.png.meta) do @if not exist ${CMAKE_SOURCE_DIR}/resources/models/%%~nxG (
        echo Deleting %%G & del /Q %%G
    )
    COMMAND for /R ${dest_dir}/shaders %%G in (*.frag *.spv *.vert) do @if not exist ${CMAKE_SOURCE_DIR}/resources/shaders/%%~nxG (
        echo Deleting %%G & del /Q %%G
    )
    COMMAND for /R ${dest_dir}/tex %%G in (*.jpg *.png) do @if not exist ${CMAKE_SOURCE_DIR}/resources/tex/%%~nxG (
      echo Deleting %%G & del /Q %%G
  )
    COMMAND echo Remove completed at && date /T && echo The current time is: %TIME%
)

add_dependencies(${NAME} copy_resources)

add_custom_command(TARGET ${NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E echo "Post build step completed.")

################################################################################
# Sub-projects
################################################################################
add_subdirectory(Engine)
add_subdirectory(Project)

# Common compilation options

set(common_compile_options
   -std=c++${CMAKE_CXX_STANDARD}
   -Wall 
   -compilerandversion=${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}
   -Wmicrosoft 
   -Wno-invalid-token-paste 
   -Wno-unknown-pragmas 
   -Wno-unused-value 
   -fsyntax-only 
   "\\\"-D_MT\\\"" 
   "\\\"-D_DLL\\\"" 
   "\\\"-DWIN32\\\"" 
   "\\\"-D_WINDOWS\\\"" 
   "\\\"-DCMAKE_INTDIR=/\\\"Debug/\\\"\\\"" 
   "\\\"-D_DEBUG_FUNCTIONAL_MACHINERY\\\""   
)

# Add each include directory to the compilation options
foreach(INCLUDE_DIR ${ALL_INCLUDE_DIR})
    list(APPEND common_compile_options -I\\\"${INCLUDE_DIR}\\\")
endforeach()

# Add an extra quote to the last element
list(LENGTH ALL_INCLUDE_DIR INCLUDE_DIR_COUNT)
math(EXPR LAST_INDEX "${INCLUDE_DIR_COUNT} - 1")
list(GET ALL_INCLUDE_DIR ${LAST_INDEX} LAST_INCLUDE_DIR)
string(CONCAT LAST_INCLUDE_DIR_WITH_QUOTE ${LAST_INCLUDE_DIR} "\"")
list(POP_BACK common_compile_options)
list(APPEND common_compile_options ${LAST_INCLUDE_DIR_WITH_QUOTE})

# Convert the list to a string with spaces between the options
string(REPLACE ";" " " common_compile_options_string "${common_compile_options}")

# Filter the source files to keep only .cpp and .hpp files
list(FILTER SOURCES INCLUDE REGEX "\\.(cpp|hpp)$")

set(compile_commands "")
foreach(SOURCE_FILE ${SOURCES})
    # Create a JSON string for each .cpp or .hpp source file
    string(APPEND compile_commands
        "    {\n"
        "        \"directory\": \"${CMAKE_CURRENT_BINARY_DIR}\",\n"
        "        \"command\": \"\\\"${CMAKE_CXX_COMPILER}\\\" -x c++ \\\"${SOURCE_FILE}\\\""
    )

    # Add the common compile options
    foreach(OPTION ${common_compile_options})
        string(APPEND compile_commands " ${OPTION}") 
    endforeach()

    # Add the rest of the command and JSON metadata
    string(APPEND compile_commands
        ",\n"
        "        \"file\": \"${SOURCE_FILE}\"\n"
        "    },\n"
    )
endforeach()

# Remove the final comma
string(REGEX REPLACE ",\n$" "" compile_commands "${compile_commands}")

# Add brackets to form a JSON list
set(compile_commands "[\n${compile_commands}\n]")

# Write the entries to a JSON file
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json "${compile_commands}")

# Add the custom target to generate compile_commands.json
add_custom_target(generate_compile_commands
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    COMMENT "Generating compile_commands.json"
)
