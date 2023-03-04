#pragma once

#include <QDialog>
#include <QMessageBox>

#include <QCloseEvent>

#include <SimpleUpdater.hpp>

namespace updt {

namespace Ui {
class UpdateHandler;
}

class UpdateHandler : public QWidget
{
    Q_OBJECT

public:
    struct States{
        enum State{
            kReset,
            kRetrievingReleaseInfo,
            kReleaseInfoRetrieved,
            kRetrievingManifest,
            kManifestRetrieved,
            kDownloading
        };
    };

    struct InfoBoxMsgType{
        enum Type{
            kUnknown,
            kWarning,
        };

        static constexpr QColor kUnknownColor{255,255,255,255};
        static constexpr QColor kUnknownTextColor{243,166,0};

        static constexpr QColor kWarningColor{250,243,216};
        static constexpr QColor kWarningTextColor{243,166,0};
    };

public:
    explicit UpdateHandler(Version progRunningVersion,QString githubReleaseApiAddress,
                           QString publicVerifierKeyFile, bool searchAvailableOnCreation,
                           bool showInstallPropositionOnNextOccasion, QWidget *parent);
    ~UpdateHandler();

    void showInstallPropositionOnNextOccasion(){m_showInstallPropositionOnNextOccasion = true;}

    void showInfoMessage(InfoBoxMsgType::Type type,const QString& message,int32_t timeout = 0);
    void hideInfoMessage();

private slots:
    void on_pb_close_clicked();
    void on_pb_checkAvailable_clicked();
    void on_pb_downloadAndInstall_clicked();

private:
    void doNotUpdate(const QString& errMsg = {});//use in case an error occurs. Reset everything
    void setReadyToUpdate(bool state = true){if(!state)doNotUpdate();else m_readyToUpdate = true;}

    void setState(States::State newState);

    void checkAvailableOnline(bool installAfterward);
    void onLatestUpdateRetrieved(std::optional<updt::ReleaseInfo> releaseInfoOpt, bool installAfterwards);
    void onManifestRetrieved(std::optional<QJsonDocument> docOpt);

private:
    Ui::UpdateHandler *ui;

    States::State m_state{States::kReset};
    bool m_askedToReset{false};

    const Version m_runningVersion;
    const QString m_githubReleaseApiAddress;
    const QString m_publicVerifierKeyFile;

    bool m_readyToUpdate{false};
    std::optional<ReleaseInfo> m_latestReleaseInfoOpt{};
    bool m_showInstallPropositionOnNextOccasion;/*!< Propose the user to install the updated  */
};

} // namespace updt
