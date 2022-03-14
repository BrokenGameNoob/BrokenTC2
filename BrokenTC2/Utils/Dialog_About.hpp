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

#ifndef DIALOG_ABOUT_HPP
#define DIALOG_ABOUT_HPP

#include <QDialog>

namespace Ui {
class Dialog_About;
}

class Dialog_About : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_About(QWidget *parent = nullptr);
    ~Dialog_About();

private:
    Ui::Dialog_About *ui;
};

#endif // DIALOG_ABOUT_HPP
