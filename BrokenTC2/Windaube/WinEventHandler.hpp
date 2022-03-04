#ifndef WINDAUBE_EVENTHANDLER_HPP
#define WINDAUBE_EVENTHANDLER_HPP

#include <windows.h>

namespace windaube {

using uint = unsigned int;

void sendKeyboardEvent(int code, bool pressed,int codeAlias = 0);
void delayedKeyboardEvent(int code, bool pressed, int delay);

} // namespace windaube

#endif // WINDAUBE_EVENTHANDLER_HPP
