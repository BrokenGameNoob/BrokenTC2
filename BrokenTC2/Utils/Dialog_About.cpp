#include "Dialog_About.hpp"
#include "ui_Dialog_About.h"

Dialog_About::Dialog_About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_About)
{
    ui->setupUi(this);
}

Dialog_About::~Dialog_About()
{
    delete ui;
}
