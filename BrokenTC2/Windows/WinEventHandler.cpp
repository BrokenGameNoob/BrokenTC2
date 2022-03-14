#include "WinEventHandler.hpp"

/*!
 *  This file is based on the antimicroX project
 *  https://github.com/AntiMicroX/antimicrox
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
