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

#include <QDebug>
#include <QStringLiteral>
#include <QString>
#include <string_view>
#include <algorithm>
#include <stdexcept>
#include <QVector>

#ifdef __PRETTY_FUNCTION__
#define __CURRENT_PLACE__ __PRETTY_FUNCTION__
#else
#define __CURRENT_PLACE__ __FUNCTION__
#endif

#define __CURRENT_PLACE_std_ std::string{__CURRENT_PLACE__}
#define __CURRENT_PLACE_q_ QString{__CURRENT_PLACE__}

//namespace formatting{

//just keep it because I like it but should really not be used...

//template<typename T,typename... Ts>
//QString recursiveQStringFormat(const QString& out, const T& firstArg, const Ts&... rest) {
//    if constexpr (sizeof...(rest) > 0) {
//        // this line will only be instantiated if there are further
//        // arguments. if rest... is empty, there will be no recursive call
//        return recursiveQStringFormat(out.arg(firstArg),rest...);
//    }
//    return out.arg(firstArg);
//}

//}// namespace formatting

class ContextualRuntimeErrorq : public std::runtime_error{
public:
    ContextualRuntimeErrorq(const QString context, const QString errMessage):
        std::runtime_error{QString{"from: %0 -> %1"}.arg(context,errMessage).toStdString()}
    {}
};

inline
QString removeSpecialChars(QString str){
    auto it = std::remove_if(str.begin(), str.end(), [](const QChar& c){ return !c.isLetterOrNumber();});
    str.chop(std::distance(it, str.end()));
    return str;
}

template<typename T>
auto retrieveChildren(QObject* parent){
    QVector<T*> out{};
    for(auto* child : parent->children()){
        auto* castedChild{qobject_cast<T *>(child)};
        if(castedChild){
            out.append(castedChild);
        }
    }
    return out;
}

#endif // GLOBAL_HPP
