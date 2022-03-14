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

#include "Update.hpp"

#include <functional>
#include <optional>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QRegularExpression>
#include <QFile>
#include <QFileInfo>

#include <QDebug>
#include <QFile>

namespace utils {

namespace net
{

void getJsonFromAPI(QObject* parent,const QString& url,std::function<void(std::optional<QJsonDocument>)> callback)
{
    auto netManager{new QNetworkAccessManager(parent)};
    QObject::connect(netManager,&QNetworkAccessManager::finished,parent,[netManager,callback,url](QNetworkReply* rep){
        if(rep->error() != QNetworkReply::NoError)
        {
            qCritical() << __CURRENT_PLACE__ << ": Cannot retrieve informations from"<<url;
            callback({});//call callback with null optionnal = ERROR
        }
        else
        {
            auto repBytes{rep->readAll()};
            QJsonParseError converstionSuccess{};
            auto doc{QJsonDocument::fromJson(repBytes,&converstionSuccess)};
            if(converstionSuccess.error != QJsonParseError::NoError)
            {
                qCritical() << __CURRENT_PLACE__ << ": Cannot parse json retrieved from"<<url;
                qCritical() << "Error : " << converstionSuccess.errorString();
                callback({});
            }
            else
            {

                callback({doc});
            }
        }
        netManager->deleteLater();
        rep->deleteLater();
    });
    netManager->get(QNetworkRequest{QUrl{url}});
}

void DownloadRequest::fileDownloaded(QNetworkReply* pReply)
{
    m_raw = pReply->readAll();

    QFile f(m_outputFile);
    if(!f.open(QIODevice::WriteOnly))
    {
        m_error = QString{"%0 : Cannot download file <%1> and save it as <%2>"}.arg(__CURRENT_PLACE__,m_url,m_outputFile);
        emit error();
        return;
    }

    f.write(m_raw);

    f.close();

    //emit a signal
    pReply->deleteLater();
    emit downloaded();
}

}//namespace net
}//namespace utils

namespace updt {

bool operator<(const Version& r,const Version& d)
{
    if(r.major < d.major)
        return true;
    if(r.minor < d.minor)
        return true;
    if(r.patch < d.patch)
        return true;
    return false;
}


std::optional<Version> getLatestReleaseInfo(QJsonDocument doc)
{
//    auto jsonFormatted(QString(doc.toJson()));
//    QFile f{"APIResults.json"};
//    if(!f.open(QIODevice::WriteOnly))
//        qCritical() << __CURRENT_PLACE__ << " : Cannot save file json";
//    else
//    {
//        f.write(doc.toJson());
//        f.close();
//    }

    auto jsonObj{doc.object()};
    QString releaseTagName{jsonObj.value("tag_name").toString()};

    const QRegularExpression tagNameRegex{"^v([0-9]{1,}).([0-9]{1,}).([0-9]{1,})$"};
    //one group per version number (major, minor, patch)

    auto match{tagNameRegex.match(releaseTagName)};
    //check for format : <vX.X.X>
    if(!match.hasMatch())
    {
        qCritical() << __CURRENT_PLACE__ << ": Cannot find proper <tag_name> in given JsonDocument";
        return {};
    }

    Version out{};
    out.major = match.captured(1).toInt();
    out.minor = match.captured(2).toInt();
    out.patch = match.captured(3).toInt();

    return {out};
}

QString getLatestReleaseUpdateFile(const QJsonDocument& doc)
{
    const QRegularExpression updateFileNameRegex{"/(?!.*/).*\\.update$"};

    auto jsonObj{doc.object()};
    auto assets(jsonObj["assets"].toArray());

    for(const auto& e : assets)
    {
        auto dlUrl{e.toObject()["browser_download_url"].toString()};
//        qDebug() << dlUrl;
        auto match{updateFileNameRegex.match(dlUrl)};
        //check for finding at the end of the string : </XXXXXXvX.X.X.update>
        if(match.hasMatch())
        {
            return {dlUrl};
        }
    }
    qCritical() << __CURRENT_PLACE__ << " : " << "Cannot find suitable update file";

    return {};
}

} // namespace updt
