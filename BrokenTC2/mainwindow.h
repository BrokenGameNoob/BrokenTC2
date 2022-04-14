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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "global.hpp"

#include "TC/Profile.hpp"
#include "TC/GearHandler.hpp"
#include "TC/Widget_gearDisplay.hpp"

#include "QSDL/GameController.hpp"
#include "QSDL/SDLEventHandler.hpp"

#include "Update/UpdateManager.hpp"

#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

struct Settings{
    bool isInit{false};

    bool displayAboutOnStartup{true};
    QString currentDeviceName{};
    bool gearDisplayed{false};
    QString displayGearScreen{};

    void setLowPerfMode(bool enable){
        m_lowPerfModeEnabled = enable;
        qsdl::SDLEventHandler::setLowPerfMode(m_lowPerfModeEnabled);
    }
    bool lowPerfMode()const{
        return m_lowPerfModeEnabled;
    }

private:
    bool m_lowPerfModeEnabled{false};
};

public:
//    LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onControllerPluggedIn(int id);
    void onControllerUnplugged(int id);

    void onControllerButtonPressed(int id);

    void onKeyboardPressed(int key);



    void on_pb_selectKey_resetDefault_clicked();

    void on_cb_selectDevice_currentIndexChanged(int index);

    void on_cb_showCurrentGear_stateChanged(int checked);

    void on_action_about_triggered();

    void on_cb_lowPerfMode_stateChanged(int arg1);

private:
    void updateSoftSettings();
    bool saveSoftSettings();
    bool loadSoftSettings();

    QString getCurrentProfileFilePath();
    QString getProfileFilePath(const QString& deviceName);
    void saveProfileSettings();
    bool loadProfileSettings();
    bool loadProfile(QString gamePadName);

    void refreshFromSettings();
    void populateDevicesComboBox();
    void refreshDisplayFromGearHandlerSettings();

private:
    Ui::MainWindow *ui;
    updt::UpdateManager m_updateManager;
    bool m_wasUpdated;

    Settings m_softSettings;
    const QString c_appDataFolder;
    const QString c_softSettingsFile;

    Widget_gearDisplay* m_gearDisplay;

    tc::GearHandler m_gearHandler;
    qsdl::GameController m_controller;
};

inline
QString MainWindow::getProfileFilePath(const QString& deviceName){
    return c_appDataFolder+removeSpecialChars(deviceName)+".json";
}
inline
QString MainWindow::getCurrentProfileFilePath(){
    return getProfileFilePath(m_gearHandler.settings().profileName);
}

#endif // MAINWINDOW_H
