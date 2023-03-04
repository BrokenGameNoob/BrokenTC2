#include "DownloadManager.hpp"

#include <QFileInfo>
#include <QDir>

#include <QDebug>

namespace updt {
namespace net {

DownloadManager::DownloadManager(QStringList urlList, QString outputPath, bool showProgress, QWidget *parent)
    :   QObject{parent},
      m_progress{((showProgress)?new QProgressDialog(QObject::tr("Download progress"), QObject::tr("Abort download"), 0, urlList.size()*100, parent):nullptr)},
      m_urlLists{std::move(urlList)},
      m_outputPath{std::move(outputPath)}
{
    if(m_progress)
    {
        m_progress->setWindowModality(Qt::WindowModal);
        QProgressDialog::connect(m_progress,&QProgressDialog::canceled,this,[this](){
            qDebug() << "Canceled download";
            abortDownloads();
        });
    }
}

void DownloadManager::startDownload(const QString& file){
    QFileInfo fInfo{file};
    QDir outFilePath{m_outputPath};
    auto tmpDlRequest{new DownloadRequest(file,outFilePath.absoluteFilePath(fInfo.fileName()),this,false)};

    m_progressList[tmpDlRequest] = 0;
    m_rqList.emplace_back(tmpDlRequest);

    DownloadRequest::connect(tmpDlRequest,&DownloadRequest::error,this,[this,tmpDlRequest](){
        slot_onDownloadError(tmpDlRequest);
    });
    DownloadRequest::connect(tmpDlRequest,&DownloadRequest::downloaded,this,[this,tmpDlRequest](){
        slot_onDownloadSucceeded(tmpDlRequest);
    });
    DownloadRequest::connect(tmpDlRequest,&DownloadRequest::progress,this,[this,tmpDlRequest](int64_t curVal,int64_t size){
        if(m_progressList[tmpDlRequest] == 100)//if we update an already finished dl
            return;
        int32_t percentage{size != 0?static_cast<int32_t>(100.*(double)curVal/(double)size):0};
        m_progressList[tmpDlRequest] = percentage;
        slot_onDownloadProgress();
    });
    tmpDlRequest->startDownload();
}

void DownloadManager::abortDownloads(){
    for(auto& rq : m_rqList)
    {
        rq->stopDownload();
    }
    m_progress->close();
}

void DownloadManager::reset(){
    abortDownloads();
    for(auto& rq : m_rqList)
    {
        rq->stopDownload();
        rq->deleteLater();
    }
    m_progressList.clear();
}

void DownloadManager::slot_onDownloadSucceeded(DownloadRequest* rq){
    qDebug().nospace() << __PRETTY_FUNCTION__ << ": Successfull download of:" << rq->url();
    m_progressList[rq] = 100;
    ++m_successCount;
    checkAllDlCompleted();
}

void DownloadManager::slot_onDownloadError(updt::net::DownloadRequest* rq){
    if(m_failedDl.find(rq) != m_failedDl.end())//Download already failed once...
        return;
    qDebug().nospace() << __PRETTY_FUNCTION__ << ": Failed download of:" << rq->url();
    m_failedDl[rq] = FailedDownload{rq->url(),rq->getError()};
    m_progressList[rq] = 100;
    checkAllDlCompleted();
}

void DownloadManager::slot_onDownloadProgress(){
    if(m_progress)
    {
        m_progress->setValue(currentProgress());
    }
}

void DownloadManager::slot_onAllDlCompleted(){
    if(m_progress)
    {
//        m_progress->close();
        slot_onDownloadProgress();
    }
    emit allDlCompleted(m_successCount,failedDlsToVector());
}

} // namespace net
} // namespace updt
