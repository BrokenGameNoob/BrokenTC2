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

#ifndef QSDL_GLOBAL_HPP
#define QSDL_GLOBAL_HPP

#include <cstdint>

#include <SDL2/SDL.h>

#include <QStringList>

namespace qsdl {

//this struct contains data shared between the EventHandler and the EventThread
class EventHandlerSharedConfig
{
public:
    EventHandlerSharedConfig(bool lowPerfMode = false):
        m_joyAxisthreshold{20000},
        m_lowPerfMode{lowPerfMode}
    {

    }

    void setJoyAxisthreshold(int16_t threshold){
        m_joyAxisthreshold = threshold;
    }
    auto joyAxisthreshold()const{
        return m_joyAxisthreshold;
    }

    bool lowPerfMode()const{
        return m_lowPerfMode;
    }
    void setLowPerfMode(bool enable){
        m_lowPerfMode = enable;
    }

private:
    friend class SDLEventThread;
    friend class SDLEventHandler;

    int16_t m_joyAxisthreshold;

    bool m_lowPerfMode;


    bool m_shouldStop{false};
};

bool initSDL(uint64_t flags);

inline
int getPluggedJoysticksCount(){
    return SDL_NumJoysticks();
}

QStringList getPluggedJoysticks();

inline
QString getPluggedJoystick(int id)
{
    return SDL_JoystickNameForIndex(id);
}

/*!
 * \brief findJoystickByName search for a plugged in joystick by it's name
 * \return joystick open ID if found, -1 otherwise
 */
int findJoystickByName(const QString name);

} // namespace qsdl

#endif // QSDL_GLOBAL_HPP
