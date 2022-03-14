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

#include "JSONTools.hpp"
#include "../global.hpp"

#include <QString>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <QFile>
#include <QTextStream>

#include <QDebug>

namespace {

constexpr bool throwErrorOnFileFail{false};

}

namespace utils {
namespace json {

bool save(const QJsonDocument& doc,const QString& fileName)
{
    auto jsonFormatted(QString(doc.toJson()));
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        if constexpr(throwErrorOnFileFail)
            throw std::runtime_error{__CURRENT_PLACE__.toStdString()+" : Cannot open file (w) "+fileName.toStdString()};
        else
            return false;
    }

    QTextStream stream(&f);
    stream << jsonFormatted;
    f.close();
    return true;
}

std::optional<QJsonDocument> read(const QString& fileName)
{
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly)){
        if constexpr(throwErrorOnFileFail)
            throw std::runtime_error{"Cannot open file (r) "+fileName.toStdString()};
        else
        return {};
    }

    auto jsonBytes{f.readAll()};
    f.close();
    auto docR{QJsonDocument::fromJson(jsonBytes)};
    return {docR};
}

}//namespace json
} // namespace utils
