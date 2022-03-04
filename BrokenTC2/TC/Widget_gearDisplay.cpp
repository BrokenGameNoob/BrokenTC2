#include "TC/Widget_gearDisplay.hpp"
#include "ui_Widget_gearDisplay.h"

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
