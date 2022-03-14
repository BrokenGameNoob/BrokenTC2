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

#ifndef WINDAUBE_EVENTHANDLER_HPP
#define WINDAUBE_EVENTHANDLER_HPP

#include <windows.h>

namespace windows {

using uint = unsigned int;

void sendKeyboardEvent(int code, bool pressed,int codeAlias = 0);
void delayedKeyboardEvent(int code, bool pressed, int delay);

} // namespace windows

#endif // WINDAUBE_EVENTHANDLER_HPP
