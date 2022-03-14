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
