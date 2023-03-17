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
            kDownloadingUpdatePackage,
            kUpdatePackageRetrieved,
        };
    };

    struct InfoBoxMsgType{
        enum Type{
            kUnknown,
            kWarning,
            kOk,
            kCritical
        };

        static constexpr QColor kUnknownColor{255,255,255,255};
        static constexpr QColor kUnknownTextColor{243,166,0};
        static const QString kUnknownTitleText;

        static constexpr QColor kWarningColor{250,243,216};
        static constexpr QColor kWarningTextColor{243,166,0};
        static const QString kWarningTitleText;

        static constexpr QColor kOkColor{216,241,227};
        static constexpr QColor kOkTextColor{68,184,111};
        static const QString kOkTitleText;

        static constexpr QColor kCriticalColor{248,209,204};
        static constexpr QColor kCriticalTextColor{182,24,37};
        static const QString kCriticalTitleText;
    };

public:
    explicit UpdateHandler(Version progRunningVersion,QString githubReleaseApiAddress,
                           QString publicVerifierKeyFile, bool searchAvailableOnCreation,
                           QString postUpdateCmd,
                           bool showInstallPropositionOnNextOccasion, QWidget *parent);
    ~UpdateHandler();

    void showInstallPropositionOnNextOccasion(){m_showInstallPropositionOnNextOccasion = true;}

    void showInfoMessage(InfoBoxMsgType::Type type,const QString& message,int32_t timeout = 0);
    void hideInfoMessage();

public:
    static std::optional<qint64> startDetachedUpdateProcess(const QString& packagePath,
                                                            const QString& verifierKeyPath,
                                                            const QString& updateManifest,
                                                            const QString& postUpdateCmd);

private slots:
    void on_pb_close_clicked();
    void on_pb_checkAvailable_clicked();
    void on_pb_downloadAndInstall_clicked();

    void on_pb_manualInstall_clicked();

private:
    void resetState(const QString& errMsg = {});//use in case an error occurs. Reset everything
    void setReadyToUpdate(bool state = true){if(!state)resetState();else m_readyToUpdate = true;}

    void setState(States::State newState,bool keepInfoBoxVisible = true);

    void downloadAndInstall(bool onlyRetrieveManifest);
    void checkAvailableOnline(bool installAfterward);
    void onLatestUpdateRetrieved(std::optional<updt::ReleaseInfo> releaseInfoOpt, bool installAfterwards);
    void onManifestRetrieved(std::optional<QJsonDocument> docOpt, bool install);
    void onUpdatePackageRetrieved();

private:
    Ui::UpdateHandler *ui;

    States::State m_state{States::kReset};
    bool m_askedToReset{false};

    const Version m_kRunningVersion;
    const QString m_kGithubReleaseApiAddress;
    const QString m_kPblicVerifierKeyFile;
    const QString m_kPostUpdateCmd;
    const QString m_kUpdatePackageName{"Update.pck"};
    const QString m_kDistantManifestName{"UpdateManifest.json"};
    const QString m_kDownloadDir{"tmpDownloads/"};

    bool m_readyToUpdate{false};
    std::optional<ReleaseInfo> m_latestReleaseInfoOpt{};
    bool m_showInstallPropositionOnNextOccasion;/*!< Propose the user to install the updated  */

    QString m_baseInfoMsg{};
    QString m_prevInfoMsgHash{};
    int32_t m_infoMsgRepetitionCount{};
};

} // namespace updt
