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

#include "mainwindow.h"

#include <QApplication>
#include <SDL2/SDL.h>

#include <QMessageBox>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QSplashScreen>
#include <QPixmap>
#include <QThread>

#include <QCommandLineParser>

#include "Windows/WinUtils.hpp"

#include "LoggerHandler.hpp"

namespace{

static constexpr auto PROCESS_NAME{"BrokenTC2.exe"};

struct CanStart{
    enum Code{
        CAN_START,
        ALREADY_RUNNING
    };
};

void preStart(){
    auto pIdList{win::findProcessesId(PROCESS_NAME)};
    auto curPId{GetCurrentProcessId()};
    qDebug() << "Current pid:" << curPId;
    for(const auto& e : pIdList)
    {
        if(e != curPId)
        {
            qDebug() << "Should terminate " << e;
            while(win::isProcessRunning(e))
                win::terminateProcess(e);
        }
    }
}

CanStart::Code canStart(){
    CanStart::Code rVal{CanStart::CAN_START};

    if(win::processCount(PROCESS_NAME) > 1)
    {
        rVal = CanStart::ALREADY_RUNNING;
    }

    return rVal;
}

}


#ifdef WIN32
int SDL_main(int argc, char *argv[])
#else
int main(int argc,char* argv[])
#endif
{
    installCustomLogHandler(logHandler::GlobalLogInfo{.progLogFilePath="BrokeLog.log",.progName="BrokenTC2"});

    int rCode{0};
    QApplication a(argc, argv);

    QSplashScreen splash{QPixmap{":/img/img/spashScreen.png"}};
    splash.show();

//    ::preStart();

//    auto canStart{::canStart()};
//    if(canStart != CanStart::CAN_START)
//    {
//        switch(canStart)
//        {
//        case CanStart::ALREADY_RUNNING:
//            QMessageBox::information(nullptr,QObject::tr("Info"),QObject::tr("BrokenTC2 is already running."));
//            break;
//        default:
//            break;
//        }
//        splash.finish(nullptr);
//        return 0;
//    }

    QCommandLineParser parser;
    parser.addOptions({
                          // A boolean option with a single name (--hide)
                          {"hide",
                           QCoreApplication::translate("main", "Hide program on startup")}
                      });
    parser.process(a);

    try
    {
        std::cout << "Hello World";
        MainWindow w{parser.isSet("hide")};
        w.show();
        splash.finish(&w);
        rCode = a.exec();
    }
    catch (const std::exception& e)
    {
        qCritical() << "A FATAL ERROR OCCURED";
        qCritical() << e.what();

        rCode = 1;
    }
    catch(...)
    {
        qCritical() << "AN UNKNOWN FATAL ERROR OCCURED";
    }

    return rCode;
}
