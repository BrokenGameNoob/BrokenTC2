#pragma once

#include <concepts>
#include <cstdint>
#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>

#include <QDebug>


namespace tc{

/* --- Global scope --- */

namespace constants{
    static constexpr int32_t MAX_GEAR{7};
};


/* --- Generic protobuf functions --- */


/* - Concepts / deleted functions - */

template <typename T>
concept FromPbMessage = std::is_base_of<google::protobuf::Message, T>::value;

/*!
 * \brief One must use this function to init a protobuf message
 * This function must be defined (template overload) for each
 * protobug message. See GameProfileHelper.hpp for an example
 *
 * Usage example: auto profile{tc::getDefault<tc::ControllerProfile>()};
 */
template<typename Proto_t>
Proto_t getDefault() = delete;

template<typename Proto_t>
Proto_t initNonInitilizedFields(const Proto_t& message, Proto_t def){
    qDebug() << "-------------------------- BEFIRE";
    qDebug() << def;
    def.MergeFrom(message);
    qDebug() << def;
    return def;
}


/* - File functions - */

bool dumpProtobufToFile(const google::protobuf::Message& message, const QString& filePath);

template<typename Proto_t>
std::optional<Proto_t> readFromFile(const QString& filePath){
    if(!QFileInfo::exists(filePath))
    {
        qCritical() << __PRETTY_FUNCTION__ << ": cannot read protobuf from file:" << filePath;
        return {};
    }

    QFile f{filePath};
    if(!f.open(QIODevice::ReadOnly))
    {
        qCritical() << __PRETTY_FUNCTION__ << ": failed to open file:" << filePath;
        return {};
    }
    QTextStream textStream{&f};
    auto tmpContent{textStream.readAll()};

    std::string jsonStr{tmpContent.toStdString()};
    Proto_t out;
    google::protobuf::util::Status success{google::protobuf::util::JsonStringToMessage(jsonStr,&out)};

    if(!success.ok())
    {
        qCritical() << __PRETTY_FUNCTION__ << ": cannot read protobuf JSON String from file:" << filePath;
        return {};
    }

    return initNonInitilizedFields(out,getDefault<Proto_t>());
}


/* - Debug - */

template<FromPbMessage Pb_t>
QString debug(const Pb_t& message){
    return QString{"%0{\n%1}"}.arg(message.descriptor()->full_name().c_str(),message.DebugString().c_str());
}
template<FromPbMessage Pb_t>
QDebug operator<<(QDebug dbg, const Pb_t& message){

    return dbg.noquote().nospace() << debug(message);
}

}
