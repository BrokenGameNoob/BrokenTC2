#pragma once

#include <QObject>

#include "Network.hpp"

#include <QProgressDialog>

namespace updt {
namespace net {

//struct DownloadStatus{
//    bool completed{false};
//    bool success{false};
//};

struct FailedDownload{
    QString url{};
    QString errMsg{};
};

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QStringList urlList, QString outputPath, bool showProgress = false, QWidget *parent = nullptr);

    void startDownload(const QString& url);

    void startAllDownloads(){
        for(const auto& url : m_urlLists)
        {
            startDownload(url);
        }
        if(m_progress)
        {
            m_progress->exec();
        }
    }

signals:
    void allDlCompleted(int32_t successCount,const QVector<updt::net::FailedDownload>& fails);

public slots:
    void abortDownloads();
    void reset();

    void slot_onDownloadSucceeded(updt::net::DownloadRequest* rq);
    void slot_onDownloadError(updt::net::DownloadRequest* rq);
    void slot_onDownloadProgress();

    void slot_onAllDlCompleted();

private:
    int32_t completedDlCount()const{
        return m_successCount + m_failedDl.size();
    }

    int32_t currentProgress()const{
        int32_t out{};
//        qDebug() << __PRETTY_FUNCTION__ << "";
        for(const auto& [dlReq,prog] : m_progressList)
        {
//            qDebug().nospace()<< "\t" << prog << " " << dlReq->url();
            out += prog;
        }
        return out;
    }

    void checkAllDlCompleted(){
        if(completedDlCount() == m_urlLists.size())
            slot_onAllDlCompleted();
    }

    QVector<FailedDownload> failedDlsToVector(){
        QVector<FailedDownload> out{};
        out.reserve(m_failedDl.size());
        for(const auto& [key,val] : m_failedDl)
        {
            out.emplace_back(val);
        }
        return out;
    }

private:
    QVector<DownloadRequest*> m_rqList{};

    QProgressDialog* m_progress{};
    std::unordered_map<DownloadRequest*,int32_t> m_progressList{};

    int32_t m_successCount{};
    std::unordered_map<DownloadRequest*,FailedDownload> m_failedDl{};

    QStringList m_urlLists{};
    QString m_outputPath{};
};

} // namespace net
} // namespace updt

