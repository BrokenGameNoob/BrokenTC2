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
