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

#include "Dialog_getGameControllerButton.hpp"
#include "ui_Dialog_getGameControllerButton.h"

#include "QSDL/GameController.hpp"

#include "../global.hpp"

Dialog_getGameControllerButton::Dialog_getGameControllerButton(qsdl::GameController *controller, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_getGameControllerButton)
{
    ui->setupUi(this);

    connect(controller,&qsdl::GameController::buttonDown,this,[&](int code){
        m_buttonSelected=true;
        done(code);
    });

    connect(ui->pb_unbind,&QPushButton::clicked,this,[&](){
        this->done(-1);
    });

    setModal(true);
}

Dialog_getGameControllerButton::~Dialog_getGameControllerButton()
{
    delete ui;
}


int Dialog_getGameControllerButton::getButton(qsdl::GameController* controller,QWidget *parent)
{
    auto dial{new Dialog_getGameControllerButton{controller,parent}};

    //some buttons might have value 0
    //which is the value used when closing the dialog
    //So if the value is 0 and no button was selected, this means cancel
    auto rval{dial->exec()};//-2 means cancelled | -1 means unbind
    if(rval == 0 && !dial->buttonSelected())
    {
        rval = -2;
    }

    qDebug() << __CURRENT_PLACE__ << "  : got : " << rval;

    return rval;
}
