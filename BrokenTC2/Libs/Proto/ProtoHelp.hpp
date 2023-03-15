#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
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

using PbCppType = google::protobuf::FieldDescriptor::CppType;
using PbReflection = google::protobuf::Reflection;

/* - Concepts / deleted functions - */

template <typename T>
concept FromPbMessage = std::is_base_of<google::protobuf::Message, T>::value;


/* --- Generic protobuf functions --- */

template<FromPbMessage Proto_t>
Proto_t initNonInitilizedFields(const Proto_t& message, Proto_t def){
    def.MergeFrom(message);
    return def;
}

//see setFieldTypeToValue for usage example
template<FromPbMessage Proto_t>
void applyOnPbFields(Proto_t* msg,
                     std::function<bool(const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)> toApply){
    const auto* desc = msg->GetDescriptor();
    auto* ref = msg->GetReflection();
    for (int i = 0; i < desc->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field_desc = desc->field(i);
        if(toApply(*field_desc,*ref))
        {
            break;
        }
    }
}

//function used for generic types
// ex:
// fieldCount = setFieldTypeToValue<PbCppType::CPPTYPE_INT32>(&out,&PbReflection::SetInt32,-1);
template<google::protobuf::FieldDescriptor::CppType targetType,FromPbMessage Proto_t,typename ... Args>
int32_t setFieldTypeToValue(Proto_t* msg,
                            void(google::protobuf::Reflection::*pbSetter)(google::protobuf::Message*,const google::protobuf::FieldDescriptor*,Args...)const,
                            Args... args){
    int32_t modifiedFieldsCount{};
    std::function<bool(const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)> toApply =
    [&](const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)->bool{
        if(desc.cpp_type() == targetType) {
            ++modifiedFieldsCount;
            (reflection.*pbSetter)(msg,&desc,args...);
        }
        return false;
    };
    applyOnPbFields(msg,toApply);
    return modifiedFieldsCount;
}

//function to set submessage types
template<FromPbMessage Proto_t>
int32_t setFieldTypeToValue(Proto_t* msg,
                            void(google::protobuf::Reflection::*pbSetter)(google::protobuf::Message*,google::protobuf::Message*,const google::protobuf::FieldDescriptor*),
                            google::protobuf::Message* subMessage){
    int32_t modifiedFieldsCount{};
    std::function<bool(const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)> toApply =
    [&](const google::protobuf::FieldDescriptor& desc, const google::protobuf::Reflection& reflection)->bool{
        if(desc.cpp_type() == google::protobuf::FieldDescriptor::CppType::CPPTYPE_MESSAGE) {
            ++modifiedFieldsCount;
            (reflection.*pbSetter)(msg,subMessage,desc);
        }
        return false;
    };
    return modifiedFieldsCount;
}

/* - Deleted functions - */

/*!
 * \brief One must use this function to init a protobuf message
 * This function must be defined (template overload) for each
 * protobug message. See GameProfileHelper.hpp for an example
 *
 * Usage example: auto profile{tc::getDefault<tc::ControllerProfile>()};
 */
template<FromPbMessage Proto_t>
Proto_t getDefault() = delete;


/* - File functions - */

bool dumpProtobufToFile(const google::protobuf::Message& message, const QString& filePath);

template<FromPbMessage Proto_t>
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
