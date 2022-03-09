#include "UpdateManager.hpp"
#include "ui_UpdateManager.h"

#include "../global.hpp"

#include <QMessageBox>
#include <QFileInfo>

#include <QProcess>

namespace
{
inline
bool saveFromRc(const QString rcPath,const QString& filePath)
{
    //":/linux/updateScript/linux.sh"
    QFile scriptRc{rcPath};
    if(!scriptRc.open(QIODevice::ReadOnly)){
        throw std::runtime_error(__CURRENT_PLACE__.toStdString()+" :\nCannot open prog resource "+QString{"<%0>"}.arg(rcPath).toStdString());
    }
    auto scriptRcContent{scriptRc.readAll()};
    scriptRc.close();

    QFile script(filePath);
    if(!script.open(QIODevice::WriteOnly))
    {
        return false;
    }
    script.write(scriptRcContent);
    script.close();
    return true;
}
}

namespace updt {

UpdateManager::UpdateManager(bool checkUpdateOnInit,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateManager),
    m_apiRequest("",this),
    m_dlRequest("","",this),
    m_updtInfo{}
{
    ui->setupUi(this);
    this->setModal(true);

    ui->prgb_download->hide();
    ui->lbl_runningVersion->setText(PROJECT_VERSION);

    connect(&m_apiRequest,&utils::net::APIRequest::ready,this,&UpdateManager::onApiRequestFinished);

    connect(&m_dlRequest,&utils::net::DownloadRequest::downloaded,this,&UpdateManager::onUpdateDownloadFinished);
    connect(&m_dlRequest,&utils::net::DownloadRequest::progress,this,[&](int64_t current,int64_t size){
        ui->prgb_download->setMaximum(size);
        ui->prgb_download->setValue(current);
    });
    connect(&m_dlRequest,&utils::net::DownloadRequest::error,[&](){
        ui->prgb_download->hide();
        ui->lbl_feedback->setText(tr("An error occurred : Cannot download update"));
    });

    resetUpdateInfo();

    if(checkUpdateOnInit)
        on_pb_checkUpdt_clicked();
}

UpdateManager::~UpdateManager()
{
    delete ui;
}

void UpdateManager::onApiRequestFinished(const QJsonDocument& doc)
{
    if(!m_apiRequest.error().isEmpty())
    {
        QString errStr{QString{"ERROR - Impossible to check for update\n%1"}.arg(m_apiRequest.error())};
        qCritical() << errStr;
        QMessageBox::warning(this,tr("Error"),errStr);
        resetUpdateInfo();
        return;
    }
    auto currVersion{updt::Version{}};

    auto distVersionOpt{updt::getLatestReleaseInfo(doc)};
    if(!distVersionOpt)
    {
        QString errStr{QString{"ERROR\n%0 : Cannot retrieve dist version from JSON"}.arg(__CURRENT_PLACE__)};
        qCritical() << errStr;
        QMessageBox::warning(this,tr("Error"),errStr);
        resetUpdateInfo();
        return;
    }
    auto distVerion{distVersionOpt.value()};

    auto assetUrl{updt::getLatestReleaseUpdateFile(doc)};
    if(assetUrl.isEmpty())
    {
        QString errStr{QString{"ERROR\n%0 : Cannot retrieve update file"}.arg(__CURRENT_PLACE__)};
        qCritical() << errStr;
        QMessageBox::warning(this,tr("Error"),errStr);
        resetUpdateInfo();
        return;
    }
    m_updtInfo.assetUrl = assetUrl;

    qDebug() << "---------- Update info ---------";
    qDebug() << "Running version : " << currVersion;
    qDebug() << "Dist version : " << distVerion;
    qDebug() << "Asset url : " << assetUrl;

    ui->lbl_availableVersion->setText(QString{"%0.%1.%2"}.arg(distVerion.major).arg(distVerion.minor).arg(distVerion.patch));

    if(currVersion < distVerion)
    {
        ui->pb_update->show();
        ui->lbl_feedback->setText(tr("A newer version is available. Hit the download button to get it"));
        notifyAvailableUpdate();
    }
    else
    {
        ui->lbl_feedback->setText(tr("No newer version available"));
    }
}

void UpdateManager::onUpdateDownloadFinished()
{
    ui->lbl_feedback->setText(tr("Update successfully downloaded"));
    ui->prgb_download->hide();

    processUpdate();
}

void UpdateManager::resetUpdateInfo(){
    m_updtInfo = {{-1,-1,-1},""};

    ui->lbl_availableVersion->setText("-");
    ui->pb_update->hide();
    ui->prgb_download->hide();
}


void UpdateManager::on_pb_checkUpdt_clicked()
{
    m_apiRequest.sendRequest(PROJECT_GITHUB_RELEASE);
    ui->lbl_feedback->setText(tr("Searching for update ..."));
}


void UpdateManager::on_pb_update_clicked()
{
    if(m_updtInfo.assetUrl.isEmpty())
        return;
    ui->prgb_download->show();
    QFileInfo inf{m_updtInfo.assetUrl};;
    m_dlRequest.startDownload(m_updtInfo.assetUrl,inf.completeBaseName()+".update");
}

void UpdateManager::processUpdate()
{
    constexpr auto LOCAL_SCRIPT_NAME{"update.py"};
    constexpr auto UPDATER_SCRIPT_RC{":/update/updateScripts/update.py"};

    auto scriptPath{QApplication::applicationDirPath()+"/"+LOCAL_SCRIPT_NAME};

    if(!saveFromRc(UPDATER_SCRIPT_RC,scriptPath))
    {
        QMessageBox::critical(this,tr("Error"),tr("Cannot save updater script : update failed")+QString{"\n%0"}.arg(__CURRENT_PLACE__));
        return;
    }

    if(!QProcess::startDetached("python",{scriptPath},QApplication::applicationDirPath()))
    {
        QMessageBox::critical(this,tr("Error"),tr("Cannot update the program for an unknown reason")+QString{"\n%0"}.arg(__CURRENT_PLACE__));
        return;
    }

    exit(0);
}

} // namespace updt
