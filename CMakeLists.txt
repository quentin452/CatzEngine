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

if(SMAA_HLSL_4_1)
    add_definitions(-DSMAA_HLSL_4_1=1)
endif()

if(DX11)
    add_definitions(-DDX11=1)
endif()

if(UNICODE)
    add_definitions(-DUNICODE=1)
endif()

# Enable LTO Optimizations(Link-Time Optimization) 
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

message(STATUS "using ${CMAKE_GENERATOR}")

project(${NAME} VERSION 0.23.0)

include_directories(external)

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

################################################################################
# Sub-projects
################################################################################
add_subdirectory(Engine)
add_subdirectory(Project)