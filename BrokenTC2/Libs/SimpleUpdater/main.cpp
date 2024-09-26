//#include "MainWindow.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "ProgArgs.hpp"
#include "ProgFeatures.hpp"
#include "LoggerHandler.hpp"
#include "Constants.hpp"
#include "libSimpleUpdater/SimpleUpdater.hpp"

#include "MainWindow.hpp"

#include <QFileInfo>

#include <QDebug>

int main(int argc, char *argv[])
{
    auto logConf{logHandler::GlobalLogInfo{.progLogFilePath="SimpleUpdater.log",.progName="SimpleUpdater"}};
    installCustomLogHandler(std::move(logConf));
    QApplication a(argc, argv);

    QCommandLineParser parser{};
    parser.addHelpOption();
//    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
//    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    addArgsOption(parser);

    parser.process(a);

    auto parsedArgs{parseArgs(parser)};

    qDebug() << parsedArgs;

    if(parsedArgs.progGoal == ProgArgs::INVALID)
    {
        qCritical() << "Invalid CLI option combination";
        parser.showHelp(1);//exit the program
    }

    MainWindow w{parsedArgs};
//    w.show();
    QMetaObject::invokeMethod(&w, "exit", Qt::QueuedConnection);
    return a.exec();
}
