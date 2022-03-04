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

} // namespace qsdl
