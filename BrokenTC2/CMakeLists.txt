cmake_minimum_required(VERSION 3.5)

project(BrokenTC2 VERSION 0.1 LANGUAGES CXX)

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

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)


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


set(PROJECT_SOURCES
        assets/resources.qrc

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

        Debug/logConsole.cpp Debug/logConsole.hpp

        ${PLATEFORM_DEPENDANT_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(BrokenTC2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET BrokenTC2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(BrokenTC2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(BrokenTC2
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(BrokenTC2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    ${SDL2_LIBRARIES}
)

set_target_properties(BrokenTC2 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BrokenTC2)
endif()

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
