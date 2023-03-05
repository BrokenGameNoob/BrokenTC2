#include "Network.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <unordered_map>
#include <QProgressDialog>

namespace updt {
namespace net {
void getJsonFromAPI(const QString& url, std::function<void(std::optional<QJsonDocument>)> callback)
{
    auto parent{new QObject{}};
    auto netManager{new QNetworkAccessManager(parent)};
    QObject::connect(netManager,&QNetworkAccessManager::finished,parent,[netManager,callback,url,parent](QNetworkReply* rep){
        if(rep->error() != QNetworkReply::NoError)
        {
            qCritical() << __PRETTY_FUNCTION__ << ": Cannot retrieve informations from"<<url;
            qCritical() << rep->errorString();
            callback({});//call callback with null optionnal = ERROR
        }
        else
        {
            auto repBytes{rep->readAll()};
            QJsonParseError converstionSuccess{};
            auto doc{QJsonDocument::fromJson(repBytes,&converstionSuccess)};
            if(converstionSuccess.error != QJsonParseError::NoError)
            {
                qCritical() << __PRETTY_FUNCTION__ << ": Cannot parse json retrieved from"<<url;
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
        parent->deleteLater();
    });
    netManager->get(QNetworkRequest{QUrl{url}});
}


void DownloadRequest::fileDownloaded(QNetworkReply* pReply)
{
    if(!m_error.isEmpty())
        return;
    auto refSize{pReply->size()};
    auto rawData = pReply->readAll();

    if(!QFileInfo{m_outputFile}.dir().mkpath("."))
    {
        m_error = QString{"%0 : Cannot download file <%1>: can't create folder <%2>"}.arg(__PRETTY_FUNCTION__,m_url,QFileInfo{m_outputFile}.dir().absolutePath());
        emit error();
        return;
    }

    QFile f(m_outputFile);
    if(!f.open(QIODevice::WriteOnly))
    {
        m_error = QString{"%0 : Cannot download file <%1> and save it as <%2>"}.arg(__PRETTY_FUNCTION__,m_url,m_outputFile);
        emit error();
        return;
    }

    auto writeCount{f.write(rawData)};

    f.close();

    if(writeCount != refSize)
    {
        m_error = QString{"%0 : Cannot flush downloaded file <%1> and save it as <%2>"}.arg(__PRETTY_FUNCTION__,m_url,m_outputFile);
        emit error();
        return;
    }

    if(pReply == m_tmpReply)
    {
        m_tmpReply->deleteLater();
        m_tmpReply = nullptr;
//        qDebug() << "------------> DELETE" << m_url;
    }

    //emit a signal
    emit downloaded();
}

void DownloadRequest::currentProgress(int64_t current,int64_t size){
    emit progress(current,size);
}

} // namespace net
} // namespace updt
