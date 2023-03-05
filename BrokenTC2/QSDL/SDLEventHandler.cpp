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

#include "QSDL/SDLEventHandler.hpp"

#include "QSDL/GameController.hpp"
#include "QSDL/SDLGlobal.hpp"
#include "../global.hpp"

#include <chrono>
#include <iostream>
#include <future>
#include <unordered_map>

#include <SDL2/SDL.h>

#include <QDebug>

#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <unistd.h>
    #define Sleep(x) sleep(x)
#endif

namespace qsdl{

//-----------------------------------------------------------------------


void SDLEventThread::work(const QVector<GameController*>& controllerList,
                          std::shared_ptr<const EventHandlerSharedConfig> sharedConfig)
{
    std::ignore = controllerList;
    static SDL_Event e;
    m_continue = true;

    static bool firstCall{true};
    if(firstCall)
    {

    }

    auto deviceId{[&](){
            return e.jdevice.which;
        }};
    static std::unordered_map<int,bool> dpadDown{};//<Button ID,isDown>
    //Button ID : (button+OFFSET)*10+
    //isDown : true/false

    static std::unordered_map<int,bool> joyMove{};//<Button ID,isDown>
    //Button ID : (axis+OFFSET)*100+(value<0?1:2)
    //isDown : true/false

    auto lambdaSleep{[&](int ms){
        QThread::msleep(ms);
    }};

    while(m_continue)
    {
        m_continue = !(sharedConfig.get()->m_shouldStop);
        if(m_continue == false)
            qDebug() << __CURRENT_PLACE__ <<"  : WE SHOULD STOP NOW";

        bool skipToNextIter{false};
        if(sharedConfig.get()->lowPerfMode())
        {
            skipToNextIter = (SDL_PollEvent(&e) == 0);
            if(skipToNextIter)
            {
                lambdaSleep(15);
            }
        }
        else
        {
            skipToNextIter = (SDL_WaitEventTimeout(&e,100) == 0);
        }

        if(skipToNextIter)
            continue;

        switch (e.type)
        {
        case SDL_JOYAXISMOTION:{
            constexpr auto AXIS_BUTTON_OFFSET{1000};
            auto curPos{abs(e.jaxis.value) > sharedConfig->joyAxisthreshold()};
            auto axis{e.jaxis.axis};
            auto dictKey{(AXIS_BUTTON_OFFSET+axis)*1000+(e.jaxis.value<0?1:2)*100+deviceId()};

            if(!joyMove.contains(dictKey))
                joyMove[dictKey] = curPos;

            if(joyMove.at(dictKey) == false && curPos){
                emit buttonDown(deviceId(),AXIS_BUTTON_OFFSET+axis*10+(e.jaxis.value<0?1:2));
                joyMove[dictKey] = true;
                lambdaSleep(15);
            }
            else if(joyMove.at(dictKey) == true && !curPos){
                emit buttonUp(deviceId(),AXIS_BUTTON_OFFSET+axis*10+(e.jaxis.value<0?1:2));
                joyMove[dictKey] = false;
                lambdaSleep(15);
            }
            break;
        }case SDL_JOYBUTTONDOWN:
//            instance()->m_buttonsTimes[e.jbutton.button].setDown();
//            qDebug() << "Button DOWN : " << e.jbutton.button;
            emit buttonDown(deviceId(),e.jbutton.button);
            break;
        case SDL_JOYBUTTONUP:
//            instance()->m_buttonsTimes[e.jbutton.button].setUp();
//            qDebug() << "Button UP : " << e.jbutton.button;
            emit buttonUp(deviceId(),e.jbutton.button);
            break;
        case SDL_JOYHATMOTION:{
            constexpr auto HAT_BUTTON_OFFSET{100};
            auto curPos{e.jhat.value};

            auto checkPos{[&](const auto& hatVal){
                    auto dictKey{(HAT_BUTTON_OFFSET+hatVal)*100+deviceId()};
                    if(curPos & hatVal){
                        emit buttonDown(deviceId(),HAT_BUTTON_OFFSET+hatVal);
                        dpadDown[dictKey] = true;
                    }
                    else if(dpadDown[dictKey] == true){
                        emit buttonUp(deviceId(),HAT_BUTTON_OFFSET+hatVal);
                        dpadDown[dictKey] = false;
                    }
                }};
            checkPos(SDL_HAT_UP);
            checkPos(SDL_HAT_RIGHT);
            checkPos(SDL_HAT_DOWN);
            checkPos(SDL_HAT_LEFT);
            break;}

        case SDL_JOYDEVICEADDED:
            qInfo() << "New controller plugged in with id:" << e.jdevice.which << "emitting signal: newControllerPluggedIn(e.jdevice.which)";
            emit newControllerPluggedIn(e.jdevice.which);
            break;
        case SDL_JOYDEVICEREMOVED:
            qInfo() << "Controller unplugged with id:" << e.jdevice.which << "emitting signal: controllerUnplugged(e.jdevice.which)";
            emit controllerUnplugged(e.jdevice.which);
        default:
            break;
        }
    }
}

SDLEventHandler::SDLEventHandler() : QObject(),m_controllerList()
{
    if(!initSDL(SDL_INIT_JOYSTICK))
    {
        qCritical() << __CURRENT_PLACE_q_ << ": Cannot initialize SDL2_Joystick";
        throw std::runtime_error(__CURRENT_PLACE_std_+" : Cannot initialize SDL2_Joystick");
    }

    auto worker{new SDLEventThread};
    worker->moveToThread(&workerThread);
    connect(this,&SDLEventHandler::operate,worker,&SDLEventThread::work);

    connect(worker,&SDLEventThread::testSignal,[&](){qDebug() << "J'ai eu un test !";});


    connect(worker,&SDLEventThread::newControllerPluggedIn,this,[&](int id){emit gameControllerAdded(id);});
    connect(worker,&SDLEventThread::controllerUnplugged,this,[&](int id){emit gameControllerRemoved(id);});

    connect(worker,&SDLEventThread::buttonDown,this,&SDLEventHandler::onButtonDown);
    connect(worker,&SDLEventThread::buttonUp,this,&SDLEventHandler::onButtonUp);

    workerThread.start();

//    emit operate(instance()->m_controllerList);
}

SDLEventHandler::~SDLEventHandler(){
    m_sharedConfig.get()->m_shouldStop = true;//nicely exit event loop
    workerThread.quit();
    workerThread.wait();
}

//***********************************************************
//---------------------------------------------- Event slot
//***********************************************************


void SDLEventHandler::onButtonDown(int controllerId,int button)
{
    for(const auto& e : m_controllerList)
    {
        if(e->id() == controllerId)
        {
            e->notifyButtonDown(button);
        }
    }
    emit buttonDown(controllerId,controllerId);
}

void SDLEventHandler::onButtonUp(int controllerId,int button)
{
    for(const auto& e : m_controllerList)
    {
        if(e->id() == controllerId)
        {
            e->notifyButtonUp(button);
        }
    }
    emit buttonUp(controllerId,button);
}


//***********************************************************
//---------------------------------------------- Event slot - END
//***********************************************************

void SDLEventHandler::registerController(GameController* g)
{
    qInfo() << __PRETTY_FUNCTION__ << "Registering new controller";
    connect(g,&GameController::unregisterFromEventHandler,instance(),&SDLEventHandler::unregisterController);

    if(instance()->m_controllerList.indexOf(g) == -1)
        instance()->m_controllerList.append(g);
}

void SDLEventHandler::unregisterController(GameController* g)
{
    qInfo() << __PRETTY_FUNCTION__ << "Unregistering new controller";
    auto gIndex{instance()->m_controllerList.indexOf(g)};
//    qDebug() << gIndex;
//    qDebug() << g;
//    qDebug() << instance()->m_controllerList;
    if(gIndex != -1)
        instance()->m_controllerList.removeAt(gIndex);
}

} // namespace qsdl

