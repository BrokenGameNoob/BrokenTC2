#include "mainwindow.h"

#include <QApplication>
#include <SDL2/SDL.h>

#include <QMessageBox>
#include <QString>


#ifdef WIN32
int SDL_main(int argc, char *argv[])
#else
int main(int argc,char* argv[])
#endif
{
    int rCode{0};
    QApplication a(argc, argv);

    try
    {
        MainWindow w;
        w.show();
        rCode = a.exec();
    }
    catch (const std::runtime_error& e)
    {
        QMessageBox::critical(nullptr,"Fatal error",QString::fromStdString(e.what()));

        rCode = 1;
    }

    return rCode;
}
