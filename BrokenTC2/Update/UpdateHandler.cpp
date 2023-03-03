#include "UpdateHandler.hpp"
#include "Utils/JSONTools.hpp"
#include "ui_UpdateHandler.h"

namespace updt {

UpdateHandler::UpdateHandler(Version progRunningVersion, QString githubReleaseApiAddress, bool searchAvailableOnCreation, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateHandler),
    m_runningVersion{std::move(progRunningVersion)},
    m_githubReleaseApiAddress{std::move(githubReleaseApiAddress)}
{
    ui->setupUi(this);

    if(searchAvailableOnCreation)
    {
        on_pb_checkAvailable_clicked();
    }
}

UpdateHandler::~UpdateHandler(){
    delete ui;
}


void UpdateHandler::on_pb_checkAvailable_clicked(){
    checkAvailableOnline(false);
}


void UpdateHandler::on_pb_downloadAndInstall_clicked()
{
    if(!m_latestReleaseInfoOpt)
    {
        doNotUpdate(tr("Failed to retrieve update information online.\nRetrying..."));
        on_pb_checkAvailable_clicked();
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

    net::getJsonFromAPI(this,manifestUrl,[this](std::optional<QJsonDocument> docOpt){
        this->onManifestRetrieved(std::move(docOpt));
    });
}


void UpdateHandler::checkAvailableOnline(bool installAfterward){
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
    const auto& releaseInfo{m_latestReleaseInfoOpt};
    qDebug() << "Github:";
    qDebug() << releaseInfo->assetsURLs;
    qDebug() << releaseInfo->versionAvailable;

    if(installAfterwards)
    {
        on_pb_downloadAndInstall_clicked();
    }
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
}

} // namespace updt
