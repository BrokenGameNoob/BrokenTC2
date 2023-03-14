#include "ProtoHelp.hpp"

namespace tc{

/* - File functions - */

bool dumpProtobufToFile(const google::protobuf::Message& message, const QString& filePath){
    auto fileDir{QFileInfo{filePath}.absoluteDir()};
    if(!fileDir.exists())
    {
        if(!fileDir.mkpath("."))
        {
            qCritical() << __PRETTY_FUNCTION__ << ": Failed to create dir: "<< fileDir.absolutePath();
            return false;
        }
    }

    std::string json{};

    auto success = google::protobuf::util::MessageToJsonString(message,&json);
    if(!success.ok())
    {
        std::string tmp{success.message()};
        qCritical() << __PRETTY_FUNCTION__ << ": Failed to export protobuf json:" << QString::fromStdString(tmp);
        return false;
    }

    QFile f{filePath};
    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << __PRETTY_FUNCTION__ << ": Failed to open file:" << filePath;
        return false;
    }
    QTextStream fStream{&f};
    fStream << QString::fromStdString(json);
    f.close();
    return true;
}

}//namespace tc
