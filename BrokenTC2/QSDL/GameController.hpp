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

inline
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

inline
void GameController::notifyButtonDown(int button)
{
    emit buttonDown(button);
}
inline
void GameController::notifyButtonUp(int button)
{
    emit buttonUp(button);
}


} // namespace qsdl

#endif // QSDL_GAMECONTROLLER_H
