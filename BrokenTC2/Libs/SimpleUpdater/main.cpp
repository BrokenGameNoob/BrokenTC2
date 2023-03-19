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
    auto logConf{logHandler::GlobalLogInfo{.progLogFilePath="SimpleUpdater.log",.progName="BrokenTC2SimpleUpdater"}};
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


//    updt::Manifest m{{0,0,0},{"../../SimpleUpdater/ExampleFiles/Qt6Xml.dll","../../SimpleUpdater/ExampleFiles/BrokenTC2.exe"},
//                    "","UPDATED.TAG"};
//    updt::Manifest m{.minVersionRequired={0,0,0},
//                     .updateFileList={"Qt6Xml.dll","BrokenTC2.exe","ExampleDir"},
//                     .finalizeUpdateCommand="",
//                     .updatedTagFile="UPDATED.TAG"};
//    qDebug() << "Saving manifest test:" <<  updt::save(m,"../../SimpleUpdater/ExampleFiles/manifest.json");


    MainWindow w{parsedArgs};
//    w.show();
    QMetaObject::invokeMethod(&w, "exit", Qt::QueuedConnection);
    return a.exec();
}
