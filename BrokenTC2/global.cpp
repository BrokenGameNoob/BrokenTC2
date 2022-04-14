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

#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <QString>
#include <algorithm>


inline
QString removeSpecialChars(QString str){
    auto it = std::remove_if(str.begin(), str.end(), [](const QChar& c){ return !c.isLetterOrNumber();});
    str.chop(std::distance(it, str.end()));
    return str;
}

#endif // GLOBAL_HPP
