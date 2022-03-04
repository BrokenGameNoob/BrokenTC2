#ifndef WIDGET_GEARDISPLAY_H
#define WIDGET_GEARDISPLAY_H

#include <QWidget>

namespace Ui {
class Widget_gearDisplay;
}

class Widget_gearDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit Widget_gearDisplay(QWidget *parent = nullptr);
    ~Widget_gearDisplay();

public slots:
    void refreshGear(int value);

private:
    Ui::Widget_gearDisplay *ui;
};

#endif // WIDGET_GEARDISPLAY_H
