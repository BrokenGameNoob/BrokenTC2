#ifndef WIDGET_GEARDISPLAY_H
#define WIDGET_GEARDISPLAY_H

#include <QWidget>

#include "TC/Profile.hpp"

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

    void onSwitchGearModeChanged(tc::GearSwitchMode newMode);

private:
    Ui::Widget_gearDisplay *ui;

    QColor m_clutchColor{255,255,255};
    QColor m_seqColor{255,0,0};
};

#endif // WIDGET_GEARDISPLAY_H
