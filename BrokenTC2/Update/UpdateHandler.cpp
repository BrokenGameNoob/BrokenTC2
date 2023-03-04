#include "UpdateHandler.hpp"
#include "ui_UpdateHandler.h"

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
}

UpdateHandler::~UpdateHandler(){
    delete ui;
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

    net::getJsonFromAPI(manifestUrl,[this](std::optional<QJsonDocument> docOpt){
        this->onManifestRetrieved(std::move(docOpt));
    });
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
    if(newState == m_state)
        return;

    auto lambdaSetAllPBEnabled{[&](bool state){
            ui->pb_checkAvailable->setEnabled(state);
            ui->pb_downloadAndInstall->setEnabled(state);
        }};

    switch (newState) {
    case States::kReset://error occured
        lambdaSetAllPBEnabled(true);
        break;
    case States::kRetrievingManifest://downloading something that we can't cancel
    case States::kRetrievingReleaseInfo:
        lambdaSetAllPBEnabled(false);
        break;
    default:
        qWarning() << "Unhandled UpdateHandler state:" << newState;
        break;
    }
    m_state = newState;
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
    qDebug() << "Github:";
    qDebug() << releaseInfo.assetsURLs;
    qDebug() << releaseInfo.versionAvailable;

    if(m_showInstallPropositionOnNextOccasion)
    {
        auto ans{QMessageBox::question(this,tr("Update installation"),
                                       tr("A new update is available (%0). Would you like to install it?")
                                       .arg(to_string(releaseInfo.versionAvailable)))};
        if(ans == QMessageBox::Yes)
        {
            qInfo() << "User chose to install the new version:" << releaseInfo.versionAvailable;
            on_pb_downloadAndInstall_clicked();
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
        on_pb_downloadAndInstall_clicked();
        return;
    }

    //if we are finished here
    setState(States::kReset);
}

void UpdateHandler::onManifestRetrieved(std::optional<QJsonDocument> docOpt){
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
    if(minVersion > m_runningVersion) //if we can't install the update
    {
        doNotUpdate();
        qInfo() << "Can't install the update given the manifest file: reason: minimal version requirement is not met:";
        qInfo() << "Current version:" << m_runningVersion << " whereas minimum required is:" << minVersion;
    }


    //download & install update

    setState(States::kReset);
}

} // namespace updt
