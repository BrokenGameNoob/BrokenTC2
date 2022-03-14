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

#ifndef DIALOG_GETKEYCODE_HPP
#define DIALOG_GETKEYCODE_HPP

#include <QDialog>

#include <QHash>

namespace Ui {
class Dialog_getKeyCode;
}

class Dialog_getKeyCode : public QDialog
{
    Q_OBJECT

public:
    struct KeyCode{
        int code;
        int scanCode;
    };

public:
    explicit Dialog_getKeyCode(QWidget *parent = nullptr);
    ~Dialog_getKeyCode();

    static KeyCode getKey(QWidget* parent);

    static constexpr int customQtKeyPrefix = 0x10000000;
    static constexpr int customKeyPrefix = 0x20000000;
    static constexpr int nativeKeyPrefix = 0x60000000;

    enum
    {
        AntKey_Shift_R = Qt::Key_Shift | customQtKeyPrefix,
        AntKey_Control_R = Qt::Key_Control | customQtKeyPrefix,
        AntKey_Shift_Lock = 0xffe6 | customKeyPrefix, // XK_Shift_Lock | 0x20000000
        AntKey_Meta_R = Qt::Key_Meta | customQtKeyPrefix,
        AntKey_Alt_R = Qt::Key_Alt | customQtKeyPrefix,
        AntKey_KP_Divide = Qt::Key_Slash | customQtKeyPrefix,
        AntKey_KP_Multiply = Qt::Key_Asterisk | customQtKeyPrefix,
        AntKey_KP_Subtract = Qt::Key_Minus | customQtKeyPrefix,
        AntKey_KP_Add = Qt::Key_Plus | customQtKeyPrefix,
        AntKey_KP_Decimal = Qt::Key_Period | customQtKeyPrefix,
        AntKey_KP_Insert = Qt::Key_Insert | customQtKeyPrefix,
        AntKey_Delete = Qt::Key_Delete | customQtKeyPrefix,
        AntKey_KP_Delete = 0xff9f | customKeyPrefix,
        AntKey_KP_End = Qt::Key_End | customQtKeyPrefix,
        AntKey_KP_Down = Qt::Key_Down | customQtKeyPrefix,
        AntKey_KP_Prior = Qt::Key_PageUp | customQtKeyPrefix,
        AntKey_KP_Left = Qt::Key_Left | customQtKeyPrefix,
        AntKey_KP_Begin = Qt::Key_Clear | customQtKeyPrefix,
        AntKey_KP_Right = Qt::Key_Right | customQtKeyPrefix,
        AntKey_KP_Home = Qt::Key_Home | customQtKeyPrefix,
        AntKey_KP_Up = Qt::Key_Up | customQtKeyPrefix,
        AntKey_KP_Next = Qt::Key_PageDown | customQtKeyPrefix,
        AntKey_KP_Enter = 0xff8d | customKeyPrefix,
        AntKey_KP_0 = 0xffb0 | customKeyPrefix,
        AntKey_KP_1 = 0xffb1 | customKeyPrefix,
        AntKey_KP_2 = 0xffb2 | customKeyPrefix,
        AntKey_KP_3 = 0xffb3 | customKeyPrefix,
        AntKey_KP_4 = 0xffb4 | customKeyPrefix,
        AntKey_KP_5 = 0xffb5 | customKeyPrefix,
        AntKey_KP_6 = 0xffb6 | customKeyPrefix,
        AntKey_KP_7 = 0xffb7 | customKeyPrefix,
        AntKey_KP_8 = 0xffb8 | customKeyPrefix,
        AntKey_KP_9 = 0xffb9 | customKeyPrefix
    };

signals:
    void key(int code,int scancode);

protected:
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent* event);


    void populateMappingHashes();

private:
    Ui::Dialog_getKeyCode *ui;


    QHash<int, int> virtKeyToQtKeyHash;
    QHash<int, int> qtKeyToVirtKeyHash;
};

#endif // DIALOG_GETKEYCODE_HPP
