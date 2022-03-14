/* reuse of antimicro Gamepad to KB+M event mapper https://github.com/AntiMicroX/antimicrox
 * Copyright (C) 2015 Travis Nickles <nickles.travis@gmail.com>
 * Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "WinEventHandler.hpp"

/*!
 *  This file is based on the antimicroX project
 *  
*/

#include "Windows/winextras.hpp"

#include <QTimer>

namespace windows {

void sendKeyboardEvent(int code, bool pressed,int codeAlias)
{
//    int code = slot->getSlotCode();
    INPUT temp[1] = {};

    unsigned int scancode = WinExtras::scancodeFromVirtualKey(code, codeAlias);
    int extended = (scancode & WinExtras::EXTENDED_FLAG) != 0;
    int tempflags = extended ? KEYEVENTF_EXTENDEDKEY : 0;

    temp[0].type = INPUT_KEYBOARD;
     temp[0].ki.wScan = MapVirtualKey(code, MAPVK_VK_TO_VSC);
    temp[0].ki.wScan = scancode;
    temp[0].ki.time = 0;
    temp[0].ki.dwExtraInfo = 0;

    temp[0].ki.wVk = code;
    temp[0].ki.dwFlags = pressed ? tempflags : (tempflags | KEYEVENTF_KEYUP); // 0 for key press
    SendInput(1, temp, sizeof(INPUT));
}

void delayedKeyboardEvent(int code, bool pressed, int delay)
{
    QTimer::singleShot(delay,[=](){
        windows::sendKeyboardEvent(code,pressed);
    });
}

} // namespace windows
