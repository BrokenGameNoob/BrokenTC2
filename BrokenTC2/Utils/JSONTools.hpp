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

#ifndef UTILS_JSONTOOLS_HPP
#define UTILS_JSONTOOLS_HPP

#include <QString>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace utils {

namespace json
{

bool save(const QJsonDocument& doc,const QString& fileName);

inline
bool save(const QJsonObject& obj,const QString& filename)
{
    return save(QJsonDocument(obj),filename);
}

std::optional<QJsonDocument> read(const QString& fileName);

}//namespace json

} // namespace utils

#endif // UTILS_JSONTOOLS_HPP
