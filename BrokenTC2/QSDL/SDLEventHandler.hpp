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

#ifndef QSDL_SDLEVENTHANDLER_HPP
#define QSDL_SDLEVENTHANDLER_HPP

#include <QThread>
#include <unordered_map>
#include <chrono>
#include <QVector>

#include <QDebug>

#include "GameController.hpp"
#include "QSDL/SDLGlobal.hpp"

namespace qsdl {

class GameController;

class SDLEventThread : public QObject
{
Q_OBJECT

public slots:
    void work(const QVector<qsdl::GameController*>& controllerList,
              std::shared_ptr<const EventHandlerSharedConfig> shouldStop);

signals:
    void testSignal();
    void buttonDown(int controllerId,int button);
    void buttonUp(int controllerId,int button);

    void newControllerPluggedIn(int controllerId);
    void controllerUnplugged(int controllerId);

private:
    bool m_continue{true};
};


//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------


//Singleton

class SDLEventHandler : public QObject
{
Q_OBJECT

QThread workerThread;

public:
    static SDLEventHandler* instance()
    {
        static SDLEventHandler m_instance{};
        return &m_instance;
    }

    static void start(){
        emit instance()->operate(instance()->m_controllerList,instance()->m_sharedConfig);
    }

    static void setLowPerfMode(bool enable){
        instance()->m_sharedConfig.get()->setLowPerfMode(enable);
    }

signals:
    void operate(const QVector<qsdl::GameController*>&,std::shared_ptr<EventHandlerSharedConfig>);

    void gameControllerAdded(int id);
    void gameControllerRemoved(int id);
    void testSignal();


    void buttonDown(int controllerId,int button);
    void buttonUp(int controllerId,int button);

private slots:
    void onButtonDown(int controllerId,int button);
    void onButtonUp(int controllerId,int button);

public slots:
    static void registerController(qsdl::GameController* g);
    static void unregisterController(qsdl:: GameController* g);

private:
    SDLEventHandler();
    ~SDLEventHandler();
    SDLEventHandler(const SDLEventHandler&)= delete;
    SDLEventHandler& operator=(const SDLEventHandler&)= delete;

    std::shared_ptr<EventHandlerSharedConfig> m_sharedConfig{
        std::make_shared<EventHandlerSharedConfig>(EventHandlerSharedConfig{false})};

    QVector<GameController*> m_controllerList;

    std::unordered_map<int,ButtonTime> m_buttonsTimes;
};

} // namespace qsdl

#endif // QSDL_SDLEVENTHANDLER_HPP
