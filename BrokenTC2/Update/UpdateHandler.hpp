#ifndef UPDT_UPDATEHANDLER_HPP
#define UPDT_UPDATEHANDLER_HPP

#include <QDialog>
#include <QMessageBox>

#include <SimpleUpdater.hpp>

namespace updt {

namespace Ui {
class UpdateHandler;
}

class UpdateHandler : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateHandler(Version progRunningVersion,QString githubReleaseApiAddress,bool searchAvailableOnCreation = true,QWidget *parent = nullptr);
    ~UpdateHandler();

private slots:
    void on_pb_checkAvailable_clicked();

    void on_pb_downloadAndInstall_clicked();

private:
    void doNotUpdate(const QString& errMsg = {}){
        m_readyToUpdate = false;
        m_latestReleaseInfoOpt = {};
        if(!errMsg.isEmpty())
        {
            QMessageBox::warning(this,tr("Error when trying to update"),errMsg);
        }
    }
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
};


} // namespace updt
#endif // UPDT_UPDATEHANDLER_HPP
