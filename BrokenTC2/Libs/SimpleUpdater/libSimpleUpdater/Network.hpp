#ifndef UPDT_NET_NETWORK_HPP
#define UPDT_NET_NETWORK_HPP

#include "Global.hpp"
#include <QString>
#include <algorithm>


#include <functional>
#include <optional>

#include <QObject>

#include <QJsonDocument>
#include <QJsonObject>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QDebug>

namespace updt {

////ask online and get it in callback
//void getLatestReleaseInfo(std::function<void (std::optional<ReleaseInfo>)> callback,
//                          const QString& projectGithubRelease);

namespace net {

//parent must not be null to avoid memory leaks
void getJsonFromAPI(QObject* parent, const QString &url, std::function<void (std::optional<QJsonDocument>)> callback);




struct DownloadStatus{
    bool completed{false};
    bool success{false};
};

//the "onFinished" callback tells whether everything was a success or not
void downloadFileList(QWidget *parent, const QStringList& files, const QString &outputPath, bool showProgressBar = false, std::function<void(bool)> onFinished = [](bool success){std::ignore = success;});


class APIRequest : public QObject
{
Q_OBJECT

public:
    explicit APIRequest(QString url = {},QObject* parent = nullptr) : QObject(parent),m_errStr{},m_url{},m_doc{},m_jsonObject{}
    {
        sendRequest(std::move(url));
    }

    void sendRequest(QString url){
        m_url = std::move(url);
        if(m_url.isEmpty())
            return;
        getJsonFromAPI(this,m_url,[this](std::optional<QJsonDocument> doc){
            this->onAPIRequestFinished(doc);
        });
    }

    const QString& error(){return m_errStr;}
    const QString& url(){return m_url;}
    const QJsonDocument& doc(){return m_doc;}
    const QJsonObject& obj(){return m_jsonObject;}

signals:
    void ready(const QJsonDocument&);

private:
    void onAPIRequestFinished(std::optional<QJsonDocument> docOpt){
        if(!docOpt)
        {
            m_errStr = QString{"%0 : Cannot retrieve information from url <%1>"}.arg(__PRETTY_FUNCTION__,m_url);
            m_doc = {};
            m_jsonObject = {};
        }
        else
        {
            m_errStr = {};
            m_doc = docOpt.value();
            m_jsonObject = m_doc.object();
        }
        emit ready(m_doc);
    }


private:
    QString m_errStr;
    QString m_url;
    QJsonDocument m_doc;
    QJsonObject m_jsonObject;
};

class DownloadRequest : public QObject
{
Q_OBJECT
public:
    DownloadRequest(QString url = {},QString outputFile = {},QObject* parent = nullptr,bool autoStart = true) : QObject(parent),
      m_error{},m_url{std::move(url)},m_outputFile{std::move(outputFile)},
      m_webCtrl{this}
    {
        connect(&m_webCtrl, SIGNAL(finished(QNetworkReply*)),this, SLOT(fileDownloaded(QNetworkReply*)));
        if(autoStart)
            startDownload(m_url,m_outputFile);
    }

    void startDownload(QString url = {},QString outputFile = {}){
        if(url.isEmpty())
            url = m_url;
        if(outputFile.isEmpty())
            outputFile = m_outputFile;

        if(url.isEmpty() || outputFile.isEmpty())
            return;

        m_url = std::move(url); m_outputFile = std::move(outputFile);
        QNetworkRequest request(m_url);
        m_tmpReply = m_webCtrl.get(request);
        connect(m_tmpReply,&QNetworkReply::downloadProgress,this,&DownloadRequest::currentProgress);
        connect(m_tmpReply,&QNetworkReply::errorOccurred,this,[&](const QNetworkReply::NetworkError& netError){
            m_error = QString{"Network error: {%0}"}.arg(netError);
            emit error();
        });
        connect(m_tmpReply,&QNetworkReply::aboutToClose,this,[&](){
            if(m_tmpReply)
            {
                m_tmpReply->deleteLater();
                m_tmpReply = nullptr;
//                qDebug() << "------------> DELETE" << m_url;
            }
        });
    }

    void stopDownload(){
        if(m_tmpReply)
        {
            m_tmpReply->abort();
            m_error = "Aborted";
            emit error();
        }
    }

    const QString& getError()const{
        return m_error;
    }
    const QString& url()const{
        return m_url;
    }

signals:
    void progress(int64_t currentValue,int64_t size);
    void downloaded();
    void error();

private slots:
    void fileDownloaded(QNetworkReply* pReply);
    void currentProgress(int64_t current,int64_t size);

private:
    QString m_error;
    QString m_url;
    QString m_outputFile;

    QNetworkAccessManager m_webCtrl;
    QNetworkReply* m_tmpReply{};
};

} // namespace net


//inline
//void getLatestReleaseInfo(std::function<void (std::optional<ReleaseInfo>)> callback,
//                          const QString& projectGithubRelease){
//    auto parent{new QObject{}};
//    net::getJsonFromAPI(parent,projectGithubRelease,[callback](std::optional<QJsonDocument> optDoc){
//        if(!optDoc)
//        {
//            qCritical() << __PRETTY_FUNCTION__ << ": Cannot retrieve latest release info from network";
//            callback({});
//        }
//        else //we retrieved the Github API json containing everything
//        {

//            callback(getLatestReleaseInfo(optDoc.value()));
////            getLatestReleaseUpdateFile(optDoc.value());
//        }
//    });
//};


} // namespace updt

#endif // UPDT_NET_NETWORK_HPP
