#ifndef DIALOG_GETGAMECONTROLLERBUTTON_HPP
#define DIALOG_GETGAMECONTROLLERBUTTON_HPP

#include <QDialog>

#include "QSDL/GameController.hpp"

namespace Ui {
class Dialog_getGameControllerButton;
}

class Dialog_getGameControllerButton : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_getGameControllerButton(qsdl::GameController* controller,QWidget *parent = nullptr);
    ~Dialog_getGameControllerButton();

    static int getButton(qsdl::GameController* controller,QWidget *parent = nullptr);

private:
    Ui::Dialog_getGameControllerButton *ui;
};

#endif // DIALOG_GETGAMECONTROLLERBUTTON_HPP
