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

#ifndef WIDGET_GEARDISPLAY_H
#define WIDGET_GEARDISPLAY_H

#include <QWidget>
#include <QTimer>

#include "TC/Profile.hpp"
#include <unordered_map>

namespace Ui {
class Widget_gearDisplay;
}

class Widget_gearDisplay : public QWidget
{
    Q_OBJECT

std::unordered_map<decltype(tc::GearSwitchMode::CLUTCH),QString> m_gearModeText{
    {tc::GearSwitchMode::CLUTCH,tr(" Sequential clutch ")},
    {tc::GearSwitchMode::SEQUENTIAL,tr(" Classic sequential ")}
};

public:
    explicit Widget_gearDisplay(QWidget *parent = nullptr);
    ~Widget_gearDisplay();

public slots:
    void refreshGear(int value);

    void onSwitchGearModeChanged(tc::GearSwitchMode newMode);

    void showOnScreen(int screenId);

    void setIndicatorVisible(bool visible);

private:
    Ui::Widget_gearDisplay *ui;

    QColor m_clutchColor{255,255,255};
    QColor m_seqColor{255,0,0};

    QTimer m_tmpShowTimer{this};
};

#endif // WIDGET_GEARDISPLAY_H
