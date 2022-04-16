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

#include "TC/Widget_gearDisplay.hpp"
#include "ui_Widget_gearDisplay.h"

#include "TC/Profile.hpp"

#include <QDebug>
#include "../global.hpp"

Widget_gearDisplay::Widget_gearDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget_gearDisplay)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setAttribute(Qt::WA_OpaquePaintEvent,false);
//    this->setStyleSheet("background-color:transparent;color:rgb(255,255,255)");

    ui->label->setText("0");
//    this->showFullScreen();
}

Widget_gearDisplay::~Widget_gearDisplay()
{
    delete ui;
}

void Widget_gearDisplay::refreshGear(int value)
{
    ui->label->setText(QString::number(value));
}

void Widget_gearDisplay::onSwitchGearModeChanged(tc::GearSwitchMode newMode)
{
    auto toString{[&](QColor in)->QString{
            return QString{"rgb(%0,%1,%2)"}.arg(in.red()).arg(in.green()).arg(in.blue());
        }};
    if(newMode == tc::GearSwitchMode::CLUTCH)
    {
        ui->label->setStyleSheet(QString{"color:%0;"}.arg(toString(m_clutchColor)));
    }
    else
    {
        ui->label->setStyleSheet(QString{"color:%0;"}.arg(toString(m_seqColor)));
    }
}
