/* Broken The Crew 2 sequential clutch assist
 * Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

public slots:
    void checkUpdate(){
        on_pb_checkUpdt_clicked();
    }

signals:
    void hiddenUpdateAvailable();//signal emitted when an update is found when window invisible
    void hiddenNoUpdateAvailable();

private:
    void resetUpdateInfo();

private slots:
    void notifyAvailableUpdate(){
        if(!this->isVisible())
            emit hiddenUpdateAvailable();
    }
    void notifyNoAvailableUpdate(){
        if(!this->isVisible())
            emit hiddenNoUpdateAvailable();
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
