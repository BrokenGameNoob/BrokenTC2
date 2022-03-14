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

#include "GUITools.hpp"

#include <QString>
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QFile>
#include <QMessageBox>

namespace utils {
namespace gui {

void showMarkdown(QWidget* parent,const QString& changelogMdFile)
{
    auto dial{QDialog(parent)};
    dial.setWindowTitle(QObject::tr("Changelog"));
    dial.setModal(true);
    if(parent){
        dial.resize(parent->width()*0.9,parent->height()*0.9);
    }


    QVBoxLayout* lay{new QVBoxLayout(&dial)};

    auto tb{new QTextBrowser(&dial)};
    lay->addWidget(tb);

    auto buttonBox{new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok,&dial)};
    QObject::connect(buttonBox,&QDialogButtonBox::clicked,&dial,[&](auto bp){
        dial.done(0);
    });
    lay->addWidget(buttonBox);

    QFile f(changelogMdFile);
    if(!f.open(QIODevice::ReadOnly))
    {
        qCritical() << "Cannot open dialog file";
        QMessageBox::critical(parent,QObject::tr("Error"),QObject::tr("Cannot open changelog file"));
        return;
    }
    QString fContent{f.readAll()};

    tb->setMarkdown(fContent);

    dial.exec();
}

} // namespace gui
} // namespace utils
