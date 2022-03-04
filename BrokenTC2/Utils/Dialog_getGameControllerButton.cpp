#include "Dialog_getGameControllerButton.hpp"
#include "ui_Dialog_getGameControllerButton.h"

#include "QSDL/GameController.hpp"

#include "../global.hpp"

Dialog_getGameControllerButton::Dialog_getGameControllerButton(qsdl::GameController *controller, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_getGameControllerButton)
{
    ui->setupUi(this);

    connect(controller,&qsdl::GameController::buttonDown,this,&Dialog_getGameControllerButton::done);

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

    auto rval{dial->exec()};//0 means cancelled | -1 means unbind

    qDebug() << __CURRENT_PLACE__ << "  : got : " << rval;

    return rval;
}
