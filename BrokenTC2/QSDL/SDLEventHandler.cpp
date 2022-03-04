#include "QSDL/SDLEventHandler.hpp"

#include "QSDL/GameController.hpp"
#include "QSDL/SDLGlobal.hpp"
#include "../global.hpp"

#include <chrono>
#include <iostream>
#include <future>

#include <SDL2/SDL.h>

#include <QDebug>

namespace qsdl{

//-----------------------------------------------------------------------


void SDLEventThread::work(const QVector<GameController*>& controllerList,std::shared_ptr<bool> shouldStop)
{
    static SDL_Event e;
    int i{};
    m_continue = true;

    static bool firstCall{true};
    if(firstCall)
    {

    }

    while(m_continue)
    {
//        if(controllerList.size() != i)
//        {
//            if(controllerList.size() > i)
//                qDebug() << "NEW CONTROLLER";
//            else
//                qDebug() << "Removed CONTROLLER";
//            i = controllerList.size();
//        }
        m_continue = !(*(shouldStop.get()));
        if(m_continue == false)
            qDebug() << __CURRENT_PLACE__ <<"  : WE SHOULD STOP NOW";

        if(SDL_WaitEventTimeout(&e,100) == 0)
            continue; //wait next iteration
        switch (e.type)
        {
        case SDL_JOYBUTTONDOWN:
//            instance()->m_buttonsTimes[e.jbutton.button].setDown();
//            qDebug() << "Button DOWN : " << e.jbutton.button;
            emit buttonDown(e.jdevice.which,e.jbutton.button);
            break;
        case SDL_JOYBUTTONUP:
//            instance()->m_buttonsTimes[e.jbutton.button].setUp();
//            qDebug() << "Button UP : " << e.jbutton.button;
            emit buttonUp(e.jdevice.which,e.jbutton.button);
            break;
        case SDL_JOYDEVICEADDED:
            emit newControllerPluggedIn(e.jdevice.which);
//            qDebug() << "PLUGGED IN CONTROLLER";
            break;
        case SDL_JOYDEVICEREMOVED:
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
        throw std::runtime_error(__CURRENT_PLACE__.toStdString()+" : Cannot initialize SDL2_Joystick");
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
    *(m_shouldStop.get()) = true;//nicely exit event loop
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
    connect(g,&GameController::unregisterFromEventHandler,instance(),&SDLEventHandler::unregisterController);

    if(instance()->m_controllerList.indexOf(g) == -1)
        instance()->m_controllerList.append(g);
}

void SDLEventHandler::unregisterController(GameController* g)
{
    auto gIndex{instance()->m_controllerList.indexOf(g)};
//    qDebug() << gIndex;
//    qDebug() << g;
//    qDebug() << instance()->m_controllerList;
    if(gIndex != -1)
        instance()->m_controllerList.removeAt(gIndex);
}

} // namespace qsdl

