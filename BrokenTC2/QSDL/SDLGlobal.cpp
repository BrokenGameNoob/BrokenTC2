/* Broken The Crew 2 sequential clutch assist
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

#include "QSDL/SDLGlobal.hpp"
#include "../global.hpp"

#include <SDL2/SDL.h>

#include <QStringList>

#include <QDebug>

namespace qsdl {

bool initSDL(uint64_t flags)
{
    if(SDL_Init(flags) < 0)
    {
        qCritical() << __CURRENT_PLACE__ << " : Cannot init SDL : error : " << SDL_GetError();
        return false;
    }
    return true;
}

QStringList getPluggedJoysticks()
{
    auto devicesCnt{getPluggedJoysticksCount()};
    QStringList out{};
    out.reserve(devicesCnt);

    for(int i{}; i < devicesCnt;++i)
    {
        out.append(SDL_JoystickNameForIndex(i));
    }
    return out;
}

int findJoystickByName(const QString name)
{
    auto joyList{getPluggedJoysticks()};
    int i{};
    for(const auto& curName : joyList)
    {
        if(name == curName)
            return i;
        ++i;
    }
    return -1;
}

} // namespace qsdl
