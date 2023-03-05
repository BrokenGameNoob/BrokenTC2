#pragma once

#include <QString>
#include <QStringList>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

namespace updt::utils {
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
} // namespace updt::utils
