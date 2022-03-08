#ifndef UTILS_UPDT_UPDATE_HPP
#define UTILS_UPDT_UPDATE_HPP



#include <QString>
#include <algorithm>

//--------------------------------------------------------------------------------------------------
#pragma clang diagnostic ignored "-Wstring-plus-int"
#define __FILENAME_PRIVATE__ (__FILE__ + SOURCE_PATH_SIZE)

#ifndef __LINE__
#error "Line macro used for debugging purpose is not defined"
#endif

#ifndef __FILE__
#error "File macro used for debugging purpose is not defined"
#else
#define _LINE_ QString::number(__LINE__)
#endif


#define __CURRENT_PLACE__ QString{"%0 : <%1> : l.%2"}.arg(__FILENAME_PRIVATE__,__func__,_LINE_)
//--------------------------------------------------------------------------------------------------


#include <functional>
#include <optional>

#include <QObject>

#include <QJsonDocument>
#include <QJsonObject>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QDebug>

namespace utils {

namespace net
{
//parent must not be null to avoid memory leaks
void getJsonFromAPI(QObject* parent, const QString &url, std::function<void (std::optional<QJsonDocument>)> callback);




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
            m_errStr = QString{"%0 : Cannot retrieve information from url <%1>"}.arg(__CURRENT_PLACE__,m_url);
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
    DownloadRequest(QString url = {},QString outputFile = {},QObject* parent = nullptr) : QObject(parent),
      m_error{},m_url{std::move(url)},m_outputFile{std::move(outputFile)},
      m_webCtrl{this},m_raw{}
    {
        connect(&m_webCtrl, SIGNAL(finished(QNetworkReply*)),this, SLOT(fileDownloaded(QNetworkReply*)));
        startDownload(url,outputFile);
    }

    void startDownload(QString url,QString outputFile){
        if(url.isEmpty() || outputFile.isEmpty())
            return;

        m_url = std::move(url); m_outputFile = std::move(outputFile);
        QNetworkRequest request(m_url);
        auto progressFollower{m_webCtrl.get(request)};
        connect(progressFollower,&QNetworkReply::downloadProgress,this,[this](int64_t current,int64_t size){
            emit progress(current,size);
        });
    }

signals:
    void progress(int currentValue,int size);
    void downloaded();
    void error();

private slots:
    void fileDownloaded(QNetworkReply* pReply);

private:
    QString m_error;
    QString m_url;
    QString m_outputFile;

    QNetworkAccessManager m_webCtrl;
    QByteArray m_raw;
};

}//namespace net
}//namespace utils


namespace updt {

struct Version
{
    int major{PROJECT_V_MAJOR};
    int minor{PROJECT_V_MINOR};
    int patch{PROJECT_V_PATCH};
};
bool operator<(const Version& v0,const Version& v1);
inline
QDebug operator<<(QDebug dbg,const Version& v){
    dbg.nospace() << v.major << "." << v.minor << "." << v.patch;
    return dbg << "\n";
}

struct Info
{
    Version available{};
    QString assetUrl{};
};

std::optional<Version> getLatestReleaseInfo(QJsonDocument doc);


QString getLatestReleaseUpdateFile(const QJsonDocument& doc);

//ask online and get it in callback
inline
void getLatestReleaseInfo(QObject* parent,std::function<void (std::optional<Version>)> callback){
    utils::net::getJsonFromAPI(parent,PROJECT_GITHUB_RELEASE,[callback](std::optional<QJsonDocument> optDoc){
        if(!optDoc)
        {
            qCritical() << __CURRENT_PLACE__ << ": Cannot retrieve latest release info from network";
            callback({});
        }
        else
        {
            callback(getLatestReleaseInfo(optDoc.value()));
            getLatestReleaseUpdateFile(optDoc.value());
        }
    });
};


} // namespace updt

#endif // UTILS_UPDT_UPDATE_HPP
