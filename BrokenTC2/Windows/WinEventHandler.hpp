#ifndef WINDAUBE_EVENTHANDLER_HPP
#define WINDAUBE_EVENTHANDLER_HPP

#include <windows.h>

namespace windows {

using uint = unsigned int;

void sendKeyboardEvent(int code, bool pressed,int codeAlias = 0);
void delayedKeyboardEvent(int code, bool pressed, int delay);

} // namespace windows

#endif // WINDAUBE_EVENTHANDLER_HPP
