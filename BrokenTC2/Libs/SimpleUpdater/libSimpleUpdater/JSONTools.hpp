#ifndef UTILS_JSONTOOLS_HPP
#define UTILS_JSONTOOLS_HPP

#include <QString>
#include <QStringList>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

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




QStringList strListFromJsonArray(const QJsonArray& arr,bool skipEmpty = true);

}//namespace json

} // namespace utils

#endif // UTILS_JSONTOOLS_HPP
