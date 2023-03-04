#pragma once

#include <QDialog>
#include <QMessageBox>

#include <QCloseEvent>

#include <SimpleUpdater.hpp>

namespace updt {

namespace Ui {
class UpdateHandler;
}

//singleton
class UpdateHandler : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateHandler(Version progRunningVersion,QString githubReleaseApiAddress,
                           bool searchAvailableOnCreation, bool showInstallPropositionOnNextOccasion,
                           QWidget *parent);
    ~UpdateHandler();

    void showInstallPropositionOnNextOccasion(){m_showInstallPropositionOnNextOccasion = true;}

private slots:
    void on_pb_checkAvailable_clicked();

    void on_pb_downloadAndInstall_clicked();

private:
    void doNotUpdate(const QString& errMsg = {});
    void setReadyToUpdate(bool state = true){if(!state)doNotUpdate();else m_readyToUpdate = true;}

    void checkAvailableOnline(bool installAfterward);
    void onLatestUpdateRetrieved(std::optional<updt::ReleaseInfo> releaseInfoOpt, bool installAfterwards);
    void onManifestRetrieved(std::optional<QJsonDocument> docOpt);

private:
    Ui::UpdateHandler *ui;

    const Version m_runningVersion;
    const QString m_githubReleaseApiAddress;

    bool m_readyToUpdate{false};
    std::optional<ReleaseInfo> m_latestReleaseInfoOpt{};
    bool m_showInstallPropositionOnNextOccasion;/*!< Propose the user to install the updated  */
};

} // namespace updt
