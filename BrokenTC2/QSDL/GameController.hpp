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

#ifndef QSDL_GAMECONTROLLER_H
#define QSDL_GAMECONTROLLER_H

#include <QObject>

#include <QVector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

#include "../global.hpp"
#include <QDebug>

namespace qsdl {


using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
class ButtonTime{
    using hrc = std::chrono::high_resolution_clock;
public:
    ButtonTime():m_buttonDown{std::chrono::high_resolution_clock::now()}
    {

    }
    int pressed(){//return -1 if button is up else return the time since the button was pressed down (ms)
        if(m_buttonDown > m_buttonUp)//if the button is still not released
            return std::chrono::duration_cast<std::chrono::milliseconds>(hrc::now() - m_buttonDown).count();
        else
            return -1;
    }

    void setDown(){m_buttonDown = hrc::now();}
    void setUp(){m_buttonDown = hrc::now();}
private:
    TimePoint m_buttonDown{};
    TimePoint m_buttonUp{};
};

//---------------------------------------------------
//*
//*         GameController
//*
//---------------------------------------------------


class GameController : public QObject
{
Q_OBJECT

public:
    explicit GameController(int controllerId = -1,QObject *parent = nullptr);
    ~GameController();

    int id(){
        return (m_controllerId == -1)?-1:SDL_JoystickInstanceID(m_controllerInstance);
    }



public slots:
    void connectController(int controllerId);
    void disconnectController(bool unregisterEventHandler = true);

    void notifyButtonDown(int button);
    void notifyButtonUp(int button);

signals:
    void unregisterFromEventHandler(qsdl::GameController*);

    void buttonDown(int button);
    void buttonUp(int button);

private:
    int m_controllerId;
    SDL_Joystick* m_controllerInstance;
};

//-------------------------------------
//*         GameController inline funcs
//-------------------------------------




} // namespace qsdl

#endif // QSDL_GAMECONTROLLER_H
