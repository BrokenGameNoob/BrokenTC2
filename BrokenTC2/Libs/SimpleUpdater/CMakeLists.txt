cmake_minimum_required(VERSION 3.5)

project(SimpleUpdater VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(CMAKE_DEBUG_MODE)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.hpp
        MainWindow.ui

        ProgArgs.hpp
        ProgFeatures.hpp
        Constants.hpp
        LoggerHandler.hpp
)

qt_add_executable(SimpleUpdater
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

set(BUILD_SIGNING_KEYS_TOOLS ON)
add_subdirectory(libSimpleUpdater)

target_link_libraries(SimpleUpdater PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

target_link_libraries(SimpleUpdater PRIVATE libSimpleUpdater)

set_target_properties(SimpleUpdater PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

qt_finalize_executable(SimpleUpdater)

add_definitions(${LOG_HANDLER_BASE_DIR_MACRO})

#add_custom_command(
#        TARGET SimpleUpdater POST_BUILD
#        COMMAND windeployqt.exe "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SimpleUpdater.exe")
