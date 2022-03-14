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

#include "PostUpdate.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>

#include <QDialog>
#include <QTextBrowser>
#include <QString>
#include <QTextStream>
#include <QMessageBox>
#include <QVBoxLayout>

#include <QDebug>

namespace{

inline
void postUpdate()
{
    //v0.1.0 -> v0.1.1
}

}


namespace updt {

bool postUpdateFunction()
{
    if(!wasUpdated())
    {
        qDebug() << "Wasnt updated";
        return false;
    }

    postUpdate();

    markAsNotUpdated();
    qDebug() << "Was updated";
    return true;
}

} // namespace updt
