cmake_minimum_required (VERSION 3.8)

set(LIB_NAME libSimpleUpdater)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED BUILD_SIGNING_KEYS_TOOLS)
    set(BUILD_SIGNING_KEYS_TOOLS OFF)
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network )

project(${LIB_NAME})

set(SOURCE_FILES
    SimpleUpdater.hpp
    Global.hpp Global.cpp
    JSONTools.hpp JSONTools.cpp

    CompressionTools.hpp CompressionTools.cpp
    DownloadManager.hpp DownloadManager.cpp
    ManifestTools.hpp ManifestTools.cpp
    Network.hpp Network.cpp

    Signing.hpp
    GeneralLibFunctions.hpp
)

# We declare the project as a static library and add all the source code files to it.
add_library(${LIB_NAME} STATIC ${SOURCE_FILES})
#Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(libSimpleUpdater::libSimpleUpdater ALIAS ${LIB_NAME})


add_subdirectory(cryptopp)

target_link_libraries(${LIB_NAME}
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    cryptopp
)

#Set target properties
target_include_directories(${LIB_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#    PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(${LIB_NAME} PRIVATE cxx_auto_type)
target_compile_options(${LIB_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Wextra -Wpedantic>)

if(BUILD_SIGNING_KEYS_TOOLS)
    IF(CMAKE_BUILD_TYPE MATCHES Debug)
        add_compile_definitions(CMAKE_DEBUG_MODE)
    ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

    add_executable(KeysToolsMain
      keysToolsMain.cpp

      LoggerHandler.hpp
      Signing.hpp
    )

    target_link_libraries(KeysToolsMain
        Qt${QT_VERSION_MAJOR}::Core
        cryptopp
    )
endif()
