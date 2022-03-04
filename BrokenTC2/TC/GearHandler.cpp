#include "GearHandler.hpp"

#include <algorithm>

#include "../global.hpp"

#ifdef Q_OS_WIN
#include "Windaube/WinEventHandler.hpp"
#endif

#include <QTimer>

#include <chrono>
#include <thread>

#include <QDebug>

namespace tc {

int getKeyCode(Gear gear,const ProfileSettings& settings)
{
    switch(gear)
    {
    case Gear::R:
        return settings.reverse;
    case Gear::N_CLUTCH:
        return settings.clutch;
    case Gear::G1:
        return settings.g1;
    case Gear::G2:
        return settings.g2;
    case Gear::G3:
        return settings.g3;
    case Gear::G4:
        return settings.g4;
    case Gear::G5:
        return settings.g5;
    case Gear::G6:
        return settings.g6;
    case Gear::G7:
        return settings.g7;
    }
    return settings.g7;
}

GearHandler::GearHandler(QObject *parent, ProfileSettings settings) : QObject(parent),
    m_settings{settings},
    m_currentGear{}
{
    if(m_settings.maxGear == -9999)//if the maximum gear isn't initialized
        m_settings.maxGear = static_cast<int>(Gear::G7);
}

void GearHandler::setGear(int gear){
    gear = std::clamp(gear,static_cast<int>(Gear::R),m_settings.maxGear);
    m_currentGear = static_cast<Gear>(gear);

    auto keyCode{getKeyCode(m_currentGear,m_settings)};
#ifdef Q_OS_WIN
    using windaube::sendKeyboardEvent;
#else
    auto sendKeyboardEvent{
        [&](auto a,auto b){
            qDebug() << __CURRENT_PLACE__ << " - UNIX - " << "Sending keyboard event : " << a;
        }
    };
#endif

    auto lambdaSwitchClutch{
        [&](int gearKeyCode){
            auto t_starting{std::chrono::high_resolution_clock::now()};

            if(m_currentGear != Gear::N_CLUTCH)
            {
                sendKeyboardEvent(getKeyCode(Gear::N_CLUTCH,m_settings),true);//press clutch
            }
            else
                gearKeyCode = getKeyCode(Gear::G7,m_settings);

            sendKeyboardEvent(gearKeyCode,true);//press gear key
            std::this_thread::sleep_for(std::chrono::milliseconds(m_settings.keyDownTime));
            sendKeyboardEvent(gearKeyCode,false);//release gear Key

            sendKeyboardEvent(getKeyCode(Gear::N_CLUTCH,m_settings),false);//release clutch

            auto t_ending{std::chrono::high_resolution_clock::now()};
            auto duration{std::chrono::duration<double>(t_ending-t_starting).count()*1000};
            qDebug() << "Gear change took :" << duration << "ms";
        }
    };

    auto lambdaSwitchSeq{
        [&](int gearKeyCode){
            sendKeyboardEvent(gearKeyCode,true);//press gear key
            std::this_thread::sleep_for(std::chrono::milliseconds(m_settings.keyDownTime));
            sendKeyboardEvent(gearKeyCode,false);//release gear Key
        }
    };

    std::thread t{lambdaSwitchClutch,keyCode};
    t.detach();

    emit gearChanged(gear);
}

} // namespace tc
