#ifndef UPDT_UPDATEMANAGER_HPP
#define UPDT_UPDATEMANAGER_HPP

#include <QDialog>
#include "Update/Update.hpp"

#include <QDebug>

namespace updt {

namespace Ui {
class UpdateManager;
}

class UpdateManager : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateManager(bool checkUpdateOnInit = false, QWidget *parent = nullptr);
    ~UpdateManager();

    void checkUpdate(){
        on_pb_checkUpdt_clicked();
    }

signals:
    void hiddenUpdateAvailable();//signal emitted when an update is found when window invisible

private:
    void resetUpdateInfo();

private slots:
    void notifyAvailableUpdate(){
        if(!this->isVisible())
        {
            emit hiddenUpdateAvailable();
        }
    }
    void onApiRequestFinished(const QJsonDocument& doc);
    void onUpdateDownloadFinished();

    void on_pb_checkUpdt_clicked();

    void on_pb_update_clicked();

    void processUpdate();

private:
    Ui::UpdateManager *ui;
    utils::net::APIRequest m_apiRequest;
    utils::net::DownloadRequest m_dlRequest;

    Info m_updtInfo;
};


} // namespace updt
#endif // UPDT_UPDATEMANAGER_HPP
