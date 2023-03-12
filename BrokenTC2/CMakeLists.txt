## Broken The Crew 2 sequential clutch assist
## Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.5)

function(SetIfNotSet varName value)
    if(NOT DEFINED ${varName})
        set(${varName} "${value}" PARENT_SCOPE)
    endif()
endfunction()


if(UNIX)
    message( FATAL_ERROR "Linux is not supported, CMake will exit." )
endif(UNIX)

#VERSION = MAJOR.MINOR.PATCH
set(VERSIONING_MAJOR 1)#incompatible update
set(VERSIONING_MINOR 1)#new features added
set(VERSIONING_PATCH 1)#bug fixed / minor change


set(EXE_NAME "BrokenTC2")


project(BrokenTC2 VERSION ${VERSIONING_MAJOR}.${VERSIONING_MINOR}.${VERSIONING_PATCH} LANGUAGES CXX)




###############################
#                             #
#        CMake config         #
#                             #
###############################

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_ROOT}/Modules")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)




###############################
#                             #
#            LIBS             #
#                             #
###############################

#SDL: Place the x86_64-w64-mingw32 included in the SDL2-devel-{version}-mingw package into C:/Libs/
set(SDL2_PATH "C:\\Libs\\x86_64-w64-mingw32\\" CACHE BOOL "" FORCE)
set(SDL2_INCLUDE_DIR "C:\\Libs\\x86_64-w64-mingw32\\include\\" CACHE BOOL "" FORCE)



# --- SDL2 ---
include_directories(${SDL2_INCLUDE_DIR})
find_package(SDL2 REQUIRED)


# --- Qt ---
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Network Xml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Network Xml REQUIRED)

# --- Sudirectories ---
add_subdirectory(Libs)




###############################
#                             #
#          Sources            #
#                             #
###############################

SET(PLATEFORM_DEPENDANT_SOURCES
        Windows/WinEventHandler.hpp Windows/WinEventHandler.cpp
        Windows/winextras.hpp Windows/winextras.cpp
        Windows/WinUtils.hpp Windows/WinUtils.cpp
)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/assets/iconResource.rc")

set(PROJECT_SOURCES
        LoggerHandler.hpp
        assets/resources.qrc
        Utils/JSONTools.hpp Utils/JSONTools.cpp
        Update/Update.hpp Update/Update.cpp
        Update/PostUpdate.hpp Update/PostUpdate.cpp
        Update/UpdateHandler.hpp Update/UpdateHandler.cpp Update/UpdateHandler.ui

        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui

        global.hpp global.cpp
        Constants.hpp

        TC/Profile.hpp TC/Profile.cpp
        TC/GearHandler.hpp TC/GearHandler.cpp
        TC/Widget_gearDisplay.hpp TC/Widget_gearDisplay.cpp TC/Widget_gearDisplay.ui
        TC/Dialog_ConfigureGame.hpp TC/Dialog_ConfigureGame.cpp TC/Dialog_ConfigureGame.ui
        TC/ConfigEditor.hpp TC/ConfigEditor.cpp

        QSDL/SDLGlobal.hpp QSDL/SDLGlobal.cpp
        QSDL/GameController.hpp QSDL/GameController.cpp
        QSDL/SDLEventHandler.hpp QSDL/SDLEventHandler.cpp

        Utils/Dialog_getKeyCode.hpp Utils/Dialog_getKeyCode.cpp Utils/Dialog_getKeyCode.ui
        Utils/Dialog_getGameControllerButton.hpp Utils/Dialog_getGameControllerButton.cpp Utils/Dialog_getGameControllerButton.ui
        Utils/Dialog_About.hpp Utils/Dialog_About.cpp Utils/Dialog_About.ui
        Utils/XmlTools.hpp Utils/XmlTools.cpp
        Utils/GUITools.hpp Utils/GUITools.cpp

        ${PLATEFORM_DEPENDANT_SOURCES}
)




###############################
#                             #
#        Compilation          #
#                             #
###############################

add_compile_options(-Wall -Wextra -Wpedantic)

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
    add_executable(${EXE_NAME}
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
    )
endif()

target_link_libraries(${EXE_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Xml
    ${SDL2_LIBRARIES}
    libSimpleUpdater::libSimpleUpdater
)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${EXE_NAME})
endif()




###############################
#                             #
# Code macros (definitions)   #
#                             #
###############################

add_definitions("-DPROJECT_V_MAJOR=${VERSIONING_MAJOR}")
add_definitions("-DPROJECT_V_MINOR=${VERSIONING_MINOR}")
add_definitions("-DPROJECT_V_PATCH=${VERSIONING_PATCH}")
add_definitions("-DPROJECT_GITHUB_RELEASE_DEFINE=\"https://api.github.com/repos/BrokenGameNoob/BrokenTC2/releases/latest\"")

add_definitions("-DPROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")
add_definitions("-DPROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")
add_definitions("-DWPROJECT_NAME=L\"${CMAKE_PROJECT_NAME}\"")

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(CMAKE_DEBUG_MODE)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

