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
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
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
