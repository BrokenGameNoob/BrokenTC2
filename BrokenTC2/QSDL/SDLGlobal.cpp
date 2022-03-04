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
