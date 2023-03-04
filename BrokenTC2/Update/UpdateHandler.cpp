#include "UpdateHandler.hpp"
#include "ui_UpdateHandler.h"

#include <QPixmap>

#include <QTimer>

namespace updt {

UpdateHandler::UpdateHandler(Version progRunningVersion,QString githubReleaseApiAddress,
                             QString publicVerifierKeyFile, bool searchAvailableOnCreation,
                             bool showInstallPropositionOnNextOccasion, QWidget *parent) :
    QWidget(),
    ui(new Ui::UpdateHandler),
    m_runningVersion{std::move(progRunningVersion)},
    m_githubReleaseApiAddress{std::move(githubReleaseApiAddress)},
    m_publicVerifierKeyFile{std::move(publicVerifierKeyFile)},
    m_showInstallPropositionOnNextOccasion{showInstallPropositionOnNextOccasion}
{
    ui->setupUi(this);

    hideInfoMessage();

    ui->lbl_runningVersion->setText(to_string(m_runningVersion));

    if(searchAvailableOnCreation)
    {
        on_pb_checkAvailable_clicked();
    }

    if(parent)
    {
        connect(parent,&QObject::destroyed,this,[this](){
            delete this;
        });
    }

    ui->lbl_disp_infoPic->setPixmap(QPixmap{":/img/img/warning.png"});
    ui->lbl_disp_infoPic->hide();
}

UpdateHandler::~UpdateHandler(){
    delete ui;
}

void UpdateHandler::showInfoMessage(InfoBoxMsgType::Type type,const QString& message,int32_t timeout){
    auto getColorStrA{[&](const QColor& in){
            return in.name(QColor::HexArgb);
        }};
    auto getColorStr{[&](const QColor& in){
            return in.name(QColor::HexRgb);
        }};

    auto setColor{[&](const QColor& bg,const QColor& txtColor){
            ui->widget_infoBox->setStyleSheet(QString{"background-color: %0;"}.arg(getColorStrA(bg)));
            ui->lbl_disp_infoBox->setStyleSheet(QString{"QLabel{color: %0;}"}.arg(getColorStr(txtColor)));
        }};

    if(!ui->widget_infoBox->isVisible())
    {
        ui->widget_infoBox->show();
    }

    switch (type) {
    case InfoBoxMsgType::Type::kWarning:
        setColor(InfoBoxMsgType::kWarningColor,InfoBoxMsgType::kWarningTextColor);
        break;
    case InfoBoxMsgType::Type::kOk:
        setColor(InfoBoxMsgType::kOkColor,InfoBoxMsgType::kOkTextColor);
        break;
    case InfoBoxMsgType::Type::kUnknown:
    default:
        setColor(InfoBoxMsgType::kUnknownColor,InfoBoxMsgType::kUnknownTextColor);
        qCritical() << "Unknown InfoBoxMsgType::Type:" << type;
        break;
    }
    ui->lbl_infoBox->setText(message);

    if(timeout > 0)
    {
        QTimer::singleShot(timeout,this,[this](){
            this->hideInfoMessage();
        });
    }
}
void UpdateHandler::hideInfoMessage(){
    ui->widget_infoBox->hide();
}


void UpdateHandler::on_pb_close_clicked()
{
    this->close();

}


void UpdateHandler::on_pb_checkAvailable_clicked(){
    checkAvailableOnline(false);
}


void UpdateHandler::on_pb_downloadAndInstall_clicked()
{
    downloadAndInstall(false);
}


void UpdateHandler::doNotUpdate(const QString& errMsg){
    m_readyToUpdate = false;
    m_latestReleaseInfoOpt = {};
    if(!errMsg.isEmpty())
    {
        QMessageBox::warning(this,tr("Error when trying to update"),errMsg);
    }
    setState(States::kReset);
}


void UpdateHandler::setState(States::State newState){
    qDebug() << newState;

    if(newState == m_state)
        return;

    auto lambdaSetAllPBEnabled{[&](bool state){
            ui->pb_checkAvailable->setEnabled(state);
            ui->pb_downloadAndInstall->setEnabled(state);
        }};

    switch (newState) {
    case States::kReset://error occured
        lambdaSetAllPBEnabled(true);
        ui->pb_downloadAndInstall->setEnabled(false);
        break;
    case States::kRetrievingManifest://downloading something that we can't cancel
    case States::kRetrievingReleaseInfo:
        lambdaSetAllPBEnabled(false);
        break;
    case States::kReleaseInfoRetrieved:
        // fall through
    case States::kManifestRetrieved:
        lambdaSetAllPBEnabled(true);
        break;
    case States::kDownloadingUpdatePackage:
    case States::kUpdatePackageRetrieved:
        lambdaSetAllPBEnabled(false);
        break;
    default:
        qWarning() << "Unhandled UpdateHandler state:" << newState;
        break;
    }
    if(newState != States::kReset)
    {
        hideInfoMessage();
    }

    m_state = newState;
}


void UpdateHandler::downloadAndInstall(bool onlyRetrieveManifest){
    setState(States::kRetrievingManifest);
    if(!m_latestReleaseInfoOpt)
    {
        doNotUpdate(tr("Failed to retrieve update information online.\nRetrying..."));
        checkAvailableOnline(true);
        return;
    }
    QString manifestUrl{};
    for(const auto& asset: m_latestReleaseInfoOpt->assetsURLs)
    {
        if(asset.contains("manifest.json"))
        {
            manifestUrl = asset;
        }
    }
    if(manifestUrl.isEmpty())
    {
        doNotUpdate();
        qCritical() << __PRETTY_FUNCTION__ << ": Could not retrieve manifest url from github";
        return;
    }

    qInfo() << __PRETTY_FUNCTION__ << ": Manifest url:" << manifestUrl;

    net::getJsonFromAPI(manifestUrl,[this,onlyRetrieveManifest](std::optional<QJsonDocument> docOpt){
        this->onManifestRetrieved(std::move(docOpt),!onlyRetrieveManifest);
    });
}

void UpdateHandler::checkAvailableOnline(bool installAfterward){
    setState(States::kRetrievingReleaseInfo);
    std::function<void(std::optional<updt::ReleaseInfo>)> callback{[=,this](std::optional<updt::ReleaseInfo> releaseInfoOpt){
            this->onLatestUpdateRetrieved(releaseInfoOpt,installAfterward);
        }};
    updt::getLatestReleaseInfoRq(m_githubReleaseApiAddress,callback);
}

//NO dialog should be showed here
//unless installAfterwards=true as this should be the case ONLY when user requested it manually
void UpdateHandler::onLatestUpdateRetrieved(std::optional<ReleaseInfo> releaseInfoOpt, bool installAfterwards){
    if(!releaseInfoOpt)
    {
        doNotUpdate();
        return;
    }
    m_latestReleaseInfoOpt = std::move(releaseInfoOpt.value());
    const auto& releaseInfo{m_latestReleaseInfoOpt.value()};
    ui->lbl_distVersion->setText(to_string(releaseInfo.versionAvailable));
    qDebug() << "Github:";
    qDebug() << releaseInfo.assetsURLs;
    qDebug() << releaseInfo.versionAvailable;

    setState(States::kReleaseInfoRetrieved);

    if(m_showInstallPropositionOnNextOccasion)
    {
        auto ans{QMessageBox::question(this,tr("Update installation"),
                                       tr("A new update is available (%0). Would you like to install it?")
                                       .arg(to_string(releaseInfo.versionAvailable)))};
        if(ans == QMessageBox::Yes)
        {
            qInfo() << "User chose to install the new version:" << releaseInfo.versionAvailable;
            downloadAndInstall(false);
            if(!this->isVisible())
            {
                this->show();
            }
        }
        else
        {
            qInfo() << "User chose NOT to install the new version:" << releaseInfo.versionAvailable;
        }
        m_showInstallPropositionOnNextOccasion = false;
        return;//give priority to user reply
    }

    if(installAfterwards)
    {
        downloadAndInstall(false);
        return;
    }

    //if we are finished here
//    setState(States::kReset);
}

void UpdateHandler::onManifestRetrieved(std::optional<QJsonDocument> docOpt,bool install){
    //we must enter this function only when m_latestReleaseInfoOpt has a value
    if(!docOpt)
    {
        doNotUpdate();
        qCritical() << __PRETTY_FUNCTION__ << ": Could not retrieve manifest JSON document";
        return;
    }
    qInfo() << "Successfully retrieved manifest JSON document";
    if(!utils::json::save(docOpt.value(),"manifest.json"))
    {
        doNotUpdate();
        qCritical() << "Could not save manifest file";
        return;
    }
    auto manifestOpt{getManifest(docOpt.value())};
    if(!manifestOpt)
    {
        doNotUpdate();
        qCritical() << "Could not read a valid manifest file from the retrieved manifest";
        return;
    }

    auto minVersion{manifestOpt.value().minVersionRequired};
//    minVersion = Version{3,10,2};
    if(minVersion > m_runningVersion) //if we can't install the update
    {
        doNotUpdate();
        qWarning() << "Can't install the update given the manifest file: reason: minimal version requirement is not met:";
        qWarning() << "Current version:" << m_runningVersion << " whereas minimum required is:" << minVersion;
//        QMessageBox::warning(this,tr("Could not update"),);
        showInfoMessage(InfoBoxMsgType::kWarning,tr("Sorry, your running version is not suitable for an automatic update.\n"
                                                    "(Min required running version is: %1)\n\n"
                                                    "Please reinstall the software manually.").arg(to_string(minVersion)));
        return;
    }

    setState(States::kManifestRetrieved);

    //download & install update

    if(!install)
    {
        return;
    }

    if(!m_latestReleaseInfoOpt)
    {
        qCritical() << "Tried to download an archive package without having release information (m_latestReleaseInfoOpt doesn't hold a value)";
    }

    QString updatePackageLink{};
    for(const auto& link : m_latestReleaseInfoOpt.value().assetsURLs)
    {
        if(link.contains(m_updatePackageName))
            updatePackageLink = link;
    }
    if(updatePackageLink.isEmpty())
    {
        qCritical() << "Could not find update archive link";
        showInfoMessage(InfoBoxMsgType::kWarning,tr("Sorry, we could not find the update package.\n"
                                                    "Please retry or contact support"));
        doNotUpdate();
        return;
    }

    qInfo() << "Downloading update package at:" << updatePackageLink;

    auto dlManager{new updt::net::DownloadManager({updatePackageLink},"tmpDownloads/",true,this)};
    connect(dlManager,&updt::net::DownloadManager::allDlCompleted,this,[&,this](int32_t successCount,const QVector<updt::net::FailedDownload>& fails){
        qDebug() << "Success: " << successCount << "  Fails:" << fails.size();
        for(const auto& e : fails)
        {
            qWarning().nospace() << "Failed to dl " << e.url << " because of " << e.errMsg;
        }
        if(fails.size() != 0)
        {
            this->doNotUpdate();
            this->showInfoMessage(InfoBoxMsgType::kWarning,tr("Failed to download the update package.\n"
                                                              "Retry or contact support"));
            return;
        }

        this->setState(States::kUpdatePackageRetrieved);
        this->showInfoMessage(InfoBoxMsgType::kOk,tr("Successfully downloaded the update package.\n"
                                                     "Starting installation..."));
    });
    dlManager->startAllDownloads();

//    setState(States::kReset);
}

void UpdateHandler::onUpdatePackageRetrieved(){

}

} // namespace updt
