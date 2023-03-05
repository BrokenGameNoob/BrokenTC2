#include "UpdtJSONTools.hpp"

#include <QFile>
#include <QTextStream>

#include <QDebug>

namespace {

constexpr bool throwErrorOnFileFail{false};

}

namespace updt::utils {
namespace json {

bool save(const QJsonDocument& doc,const QString& fileName)
{
    auto jsonFormatted(QString(doc.toJson()));
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        if constexpr(throwErrorOnFileFail)
            throw std::runtime_error{std::string{__PRETTY_FUNCTION__}+" : Cannot open file (w) "+fileName.toStdString()};
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

QStringList strListFromJsonArray(const QJsonArray& arr,bool skipEmpty){
    QStringList rVal{};
    rVal.reserve(arr.size());
    for(const auto& e : arr)
    {
        auto tmp{e.toString("")};
        if(tmp == "" && skipEmpty)
            continue;
        rVal.append(std::move(tmp));
    }
    return rVal;
}

}//namespace json
} // namespace updt::utils
