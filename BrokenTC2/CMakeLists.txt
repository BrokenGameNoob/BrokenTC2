cmake_minimum_required(VERSION 3.5)

#VERSION = MAJOR.MINOR.PATCH
set(VERSIONING_MAJOR 0)#incompatible update
set(VERSIONING_MINOR 1)#new features added
set(VERSIONING_PATCH 2)#bug fixed / minor change


add_definitions("-DPROJECT_V_MAJOR=${VERSIONING_MAJOR}")
add_definitions("-DPROJECT_V_MINOR=${VERSIONING_MINOR}")
add_definitions("-DPROJECT_V_PATCH=${VERSIONING_PATCH}")
add_definitions("-DPROJECT_GITHUB_RELEASE=\"https://api.github.com/repos/BrokenGameNoob/BrokenTC2/releases/latest\"")

set(EXE_NAME "BrokenTC2")


project(BrokenTC2 VERSION ${VERSIONING_MAJOR}.${VERSIONING_MINOR}.${VERSIONING_PATCH} LANGUAGES CXX)

add_definitions("-DPROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")

set(SDL2_PATH "C:\\Libs\\x86_64-w64-mingw32\\" CACHE BOOL "" FORCE)

if(UNIX)
    set(SDL2_INCLUDE_DIR "/usr/include/" CACHE BOOL "" FORCE)
    set(SDL2_LIBRARY "/usr/local/lib/libSDL2.so")
    message(STATUS "FOUND LINUX")
else()
    set(SDL2_PATH "C:\\Libs\\x86_64-w64-mingw32\\" CACHE BOOL "" FORCE)
    message(STATUS "Found Windows")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_ROOT}/Modules")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${SDL2_INCLUDE_DIR})

find_package(SDL2 REQUIRED)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network REQUIRED)


if(WIN32)
    SET(PLATEFORM_DEPENDANT_SOURCES
            Windaube/WinEventHandler.hpp Windaube/WinEventHandler.cpp
            Windaube/winextras.hpp Windaube/winextras.cpp
)
endif(WIN32)
if(UNIX)
    SET(PLATEFORM_DEPENDANT_SOURCES

)
endif(UNIX)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/assets/iconResource.rc")

set(PROJECT_SOURCES
        assets/resources.qrc
        Update/Update.hpp Update/Update.cpp
        Update/PostUpdate.hpp Update/PostUpdate.cpp
        Update/UpdateManager.hpp Update/UpdateManager.cpp Update/UpdateManager.ui

        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        global.hpp global.cpp

        SoftSettings.hpp SoftSettings.cpp

        TC/Profile.hpp TC/Profile.cpp
        TC/GearHandler.hpp TC/GearHandler.cpp
        TC/Widget_gearDisplay.hpp TC/Widget_gearDisplay.cpp TC/Widget_gearDisplay.ui

        QSDL/SDLGlobal.hpp QSDL/SDLGlobal.cpp
        QSDL/GameController.hpp QSDL/GameController.cpp
        QSDL/SDLEventHandler.hpp QSDL/SDLEventHandler.cpp

#        Windaube/WinEventHandler.hpp Windaube/WinEventHandler.cpp
#        Windaube/winextras.hpp Windaube/winextras.cpp

        Utils/Dialog_getKeyCode.hpp Utils/Dialog_getKeyCode.cpp Utils/Dialog_getKeyCode.ui
        Utils/Dialog_getGameControllerButton.hpp Utils/Dialog_getGameControllerButton.cpp Utils/Dialog_getGameControllerButton.ui
        Utils/Dialog_About.hpp Utils/Dialog_About.cpp Utils/Dialog_About.ui
        Utils/JSONTools.hpp Utils/JSONTools.cpp
        Utils/GUITools.hpp Utils/GUITools.cpp

        Debug/logConsole.cpp Debug/logConsole.hpp

        ${PLATEFORM_DEPENDANT_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${EXE_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET BrokenTC2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${EXE_NAME} SHARED
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${EXE_NAME}
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
        )
    endif()
endif()

target_link_libraries(${EXE_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    ${SDL2_LIBRARIES}
)

set_target_properties(${EXE_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${EXE_NAME})
endif()

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
