#include "UpdateHandler.hpp"
#include "ui_UpdateHandler.h"

#include <QPixmap>

#include <QTimer>
#include <QProcess>

namespace updt {

UpdateHandler::UpdateHandler(Version progRunningVersion,QString githubReleaseApiAddress,
                             QString publicVerifierKeyFile, bool searchAvailableOnCreation,
                             QString postUpdateCmd,
                             bool showInstallPropositionOnNextOccasion, QWidget *parent) :
    QWidget(),
    ui(new Ui::UpdateHandler),
    m_kRunningVersion{std::move(progRunningVersion)},
    m_kGithubReleaseApiAddress{std::move(githubReleaseApiAddress)},
    m_kPblicVerifierKeyFile{std::move(publicVerifierKeyFile)},
    m_kPostUpdateCmd{std::move(postUpdateCmd)},
    m_showInstallPropositionOnNextOccasion{showInstallPropositionOnNextOccasion}
{
    ui->setupUi(this);

    hideInfoMessage();

    ui->lbl_runningVersion->setText(to_string(m_kRunningVersion));

    if(searchAvailableOnCreation)
    {
        checkAvailableOnline(false);
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

    if(!ui->widget_infoBox->isVisible() && this->isVisible())
    {
        ui->widget_infoBox->show();
        ui->widget_infoBox->update();
    }

    switch (type) {
    case InfoBoxMsgType::Type::kWarning:
        setColor(InfoBoxMsgType::kWarningColor,InfoBoxMsgType::kWarningTextColor);
        break;
    case InfoBoxMsgType::Type::kOk:
        setColor(InfoBoxMsgType::kOkColor,InfoBoxMsgType::kOkTextColor);
        break;
    case InfoBoxMsgType::Type::kCritical:
        setColor(InfoBoxMsgType::kCriticalColor,InfoBoxMsgType::kCriticalTextColor);
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


void UpdateHandler::resetState(const QString& errMsg){
    m_readyToUpdate = false;
    m_latestReleaseInfoOpt = {};
    if(!errMsg.isEmpty())
    {
        QMessageBox::warning(this,tr("Error when trying to update"),errMsg);
    }
    setState(States::kReset);
}


void UpdateHandler::setState(States::State newState, bool keepInfoBoxVisible){
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
    if(newState != States::kReset && !keepInfoBoxVisible)
    {
        hideInfoMessage();
    }

    m_state = newState;
}


void UpdateHandler::downloadAndInstall(bool onlyRetrieveManifest){
    setState(States::kRetrievingManifest);
    if(!m_latestReleaseInfoOpt)
    {
        resetState();
        showInfoMessage(InfoBoxMsgType::kWarning,tr("Failed to retrieve update information online.\nRetrying..."));
        checkAvailableOnline(true);
        return;
    }
    QString manifestUrl{};
    for(const auto& asset: m_latestReleaseInfoOpt->assetsURLs)
    {
        if(asset.contains(m_kDistantManifestName))
        {
            manifestUrl = asset;
        }
    }
    if(manifestUrl.isEmpty())
    {
        resetState();
        qCritical() << __PRETTY_FUNCTION__ << ": Could not retrieve manifest url from github";
        showInfoMessage(InfoBoxMsgType::kWarning,tr("Could not retrieve manifest url from github"));
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
    updt::getLatestReleaseInfoRq(m_kGithubReleaseApiAddress,callback);
}

//NO dialog should be showed here
//unless installAfterwards=true as this should be the case ONLY when user requested it manually
void UpdateHandler::onLatestUpdateRetrieved(std::optional<ReleaseInfo> releaseInfoOpt, bool installAfterwards){
    if(!releaseInfoOpt)
    {
        showInfoMessage(InfoBoxMsgType::kCritical,tr("Could not retrieve info about the latest version available online."));
        qCritical() << "Could not retrieve info about the latest version available online.";
        resetState();
        return;
    }
    m_latestReleaseInfoOpt = std::move(releaseInfoOpt.value());
    const auto& releaseInfo{m_latestReleaseInfoOpt.value()};
    ui->lbl_distVersion->setText(to_string(releaseInfo.versionAvailable));
    qDebug() << "Github:";
    qDebug() << releaseInfo.assetsURLs;
    qDebug() << releaseInfo.versionAvailable;

    qInfo() << " --- Update status ---";
    qInfo().noquote().nospace() << "\tRunning version: " << m_kRunningVersion;
    qInfo().noquote().nospace() << "\tDistant version: " << releaseInfo.versionAvailable;
    if(m_kRunningVersion >= releaseInfo.versionAvailable)
    {
        qInfo().noquote().nospace() << "\tNo installation candidate found (distant version <= running version)";
        showInfoMessage(InfoBoxMsgType::kOk,tr("You are already running the latest available version"));
        resetState();

        if(m_showInstallPropositionOnNextOccasion)
            m_showInstallPropositionOnNextOccasion = false;

        return;
    }


    qInfo().noquote().nospace() << "\tInstallation candidate found (distant version > running version)";
    showInfoMessage(InfoBoxMsgType::kWarning,tr("A newer version is available, you should install it"));

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
        resetState();
        qCritical() << __PRETTY_FUNCTION__ << ": Could not retrieve manifest JSON document";
        return;
    }
    qInfo() << "Successfully retrieved manifest JSON document";
    if(!utils::json::save(docOpt.value(),m_kDistantManifestName))
    {
        resetState();
        qCritical() << "Could not save manifest file";
        return;
    }
    auto manifestOpt{getManifest(docOpt.value())};
    if(!manifestOpt)
    {
        resetState();
        qCritical() << "Could not read a valid manifest file from the retrieved manifest";
        return;
    }

    auto minVersion{manifestOpt.value().minVersionRequired};
//    minVersion = Version{3,10,2};
    if(minVersion > m_kRunningVersion) //if we can't install the update
    {
        resetState();
        qWarning() << "Can't install the update given the manifest file: reason: minimal version requirement is not met:";
        qWarning() << "Current version:" << m_kRunningVersion << " whereas minimum required is:" << minVersion;
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
        if(link.contains(m_kUpdatePackageName))
            updatePackageLink = link;
    }
    if(updatePackageLink.isEmpty())
    {
        qCritical() << "Could not find update archive link";
        showInfoMessage(InfoBoxMsgType::kWarning,tr("Sorry, we could not find the update package.\n"
                                                    "Please retry or contact support"));
        resetState();
        return;
    }

    qInfo() << "Downloading update package at:" << updatePackageLink;

    auto dlManager{new updt::net::DownloadManager({updatePackageLink},m_kDownloadDir,true,this)};
    connect(dlManager,&updt::net::DownloadManager::allDlCompleted,this,[&,this](int32_t successCount,const QVector<updt::net::FailedDownload>& fails){
        qDebug() << "Success: " << successCount << "  Fails:" << fails.size();
        for(const auto& e : fails)
        {
            qWarning().nospace() << "Failed to dl " << e.url << " because of " << e.errMsg;
        }
        if(fails.size() != 0)
        {
            this->resetState();
            this->showInfoMessage(InfoBoxMsgType::kWarning,tr("Failed to download the update package.\n"
                                                              "Retry or contact support"));
            return;
        }

        this->setState(States::kUpdatePackageRetrieved);
        this->showInfoMessage(InfoBoxMsgType::kOk,tr("Successfully downloaded the update package.\n"
                                                     "Starting installation..."));
        onUpdatePackageRetrieved();
    });
    dlManager->startAllDownloads();
}

void UpdateHandler::onUpdatePackageRetrieved(){
    constexpr auto kStartCommand{"SimpleUpdater.exe"};
//#ifdef Q_OS_WINDOWS
//    #define UPDATE_OPT_ARG "/c",
//#else
//    #define UPDATE_OPT_ARG
//#endif

    qint64 pid{};
    // -i
    auto success = QProcess::startDetached(kStartCommand,
    {"-i",m_kDownloadDir+m_kUpdatePackageName,
     "-v",m_kPblicVerifierKeyFile,
     "-m",m_kDistantManifestName,
     "-p",m_kPostUpdateCmd,
     "-o","."},QApplication::applicationDirPath(),&pid);
    if(pid == 0 || !success)
    {
        qCritical() << "Could not start the update process (" << __PRETTY_FUNCTION__ << ")";
        showInfoMessage(InfoBoxMsgType::kCritical,tr("Cannot start the update process for an unknown reason"));
        resetState();
        return;
    }
    qInfo() << "Started update with PID:" << pid;
    QApplication::exit(0);
}

} // namespace updt
