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

#include "GameController.hpp"

#include <QObject>
#include <QVector>
#include "global.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

#include "QSDL/SDLGlobal.hpp"

#include <QDebug>

namespace qsdl {

GameController::GameController(int controllerId, QObject *parent)
    : QObject{parent},
      m_controllerId{controllerId},
      m_controllerInstance{nullptr}
{
    if(controllerId >= 0)
    {
        connectController(controllerId);
    }
}

GameController::~GameController()
{
    disconnectController();
}

void GameController::connectController(int controllerId)
{
    if(m_controllerId >= 0)//if the controller is already connected
    {
        disconnectController(false);//disconnect it
    }

    auto joystickCount{getPluggedJoysticksCount()};
    if(controllerId >= joystickCount)
    {
        m_controllerId = -1;
        qCritical() << __CURRENT_PLACE__ << " : Error : " << QString{"Invalid controller ID (%0), there is only %1 devices plugged in"}.arg(controllerId).arg(getPluggedJoysticksCount());
    }
    else
    {
        m_controllerId = controllerId;
        m_controllerInstance = SDL_JoystickOpen(m_controllerId);
        if(!m_controllerInstance)
        {
            qCritical() << __CURRENT_PLACE__ << " : Error : " << QString{"Unable to open game controller device (id=%0)"}.arg(m_controllerId);
        }
    }
}

void GameController::disconnectController(bool unregisterEventHandler){
    if(m_controllerInstance)
    {
        SDL_JoystickClose(m_controllerInstance);
        m_controllerInstance = nullptr;
        m_controllerId = -1;
    }
    if(unregisterEventHandler)
        emit unregisterFromEventHandler(this);
}


void GameController::notifyButtonDown(int button)
{
    emit buttonDown(button);
}


void GameController::notifyButtonUp(int button)
{
    emit buttonUp(button);
}

} // namespace qsdl
