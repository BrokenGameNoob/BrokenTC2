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
#include <QPixmap>
#include <QSystemTrayIcon>

#include "QSDL/GameController.hpp"
#include "QSDL/SDLEventHandler.hpp"
#include "TC/GearHandler.hpp"
#include "TC/Profile.hpp"
#include "TC/Widget_gearDisplay.hpp"
#include "Update/UpdateHandler.hpp"
#include "Utils/Dialog_getKeyCode.hpp"
#include "Utils/JSONTools.hpp"
#include "global.hpp"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

  struct Settings {
    bool isInit{false};

    //    bool launchOnComputerStartup{false};
    bool displayAboutOnStartup{true};
    QString currentDeviceName{};
    bool gearDisplayed{false};  // whether the current gear be displayed on screen or not
    bool enableNotification{false};
    QString displayGearScreen{};
    bool ignoreVJoyProfile{false};

    bool exitOnCloseEvent{true};

    int openedTab{2};

    int preferredCoreCount{2};

    QString backgroundImagePath{};

    void setLowPerfMode(bool enable) {
      m_lowPerfModeEnabled = enable;
      qsdl::SDLEventHandler::setLowPerfMode(m_lowPerfModeEnabled);
    }
    bool lowPerfMode() const {
      return m_lowPerfModeEnabled;
    }

    void setBgHUDColor(QColor c, Widget_gearDisplay *m_gearDisplay);
    QColor bgHUDColor() const {
      return m_bgHUDColor;
    }

    void setJoyAxisThreshold(int16_t threshold) {
      m_joyAxisthreshold = threshold;
      qsdl::SDLEventHandler::setJoyAxisThreshold(threshold);
    }
    int16_t joyAxisThreshold() const {
      return m_joyAxisthreshold;
    }

   private:
    bool m_lowPerfModeEnabled{true};
    int16_t m_joyAxisthreshold{std::numeric_limits<int16_t>::max() / 2};
    QColor m_bgHUDColor{79, 79, 79, 120};
  };

 public:
  //    LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM
  //    lParam);
  MainWindow(bool hideOnStartup, QWidget *parent = nullptr);
  ~MainWindow();

 protected:
  void closeEvent(QCloseEvent *event) override;
  void showEvent(QShowEvent *event) override;

 private slots:
  void UpdateConflicts();

  void onControllerPluggedIn(int id);
  void onControllerUnplugged(int id);

  void onControllerButtonPressed(int id);
  void onControllerButtonReleased(int id);

  void onKeyboardPressed(int key);

  void on_cb_selectDevice_currentIndexChanged(int index);
  void on_cb_showCurrentGear_stateChanged(int checked);
  void on_action_about_triggered();
  void on_cb_lowPerfMode_currentIndexChanged(int arg1);
  void on_pb_bgHUDColor_clicked();
  void on_sb_bgHUDColorAlpha_valueChanged(int arg1);
  void on_pb_ezConf_clicked();
  void on_action_checkUpdates_triggered();
  void on_actionOpen_logs_folder_triggered();
  void on_pb_changeBackground_clicked();

 protected:
  void paintEvent(QPaintEvent *pe) override;

 private:
  int32_t GetKey();
  bool setBackgroungImage(const QString &newPath);
  void updateSoftSettings();
  bool saveSoftSettings();
  bool loadSoftSettings();

  QString getCurrentProfileFilePath();
  QString getProfileFilePath(const QString &deviceName);
  void saveProfileSettings();
  bool loadProfileSettings();
  bool loadProfile(QString gamePadName);

  void refreshFromSettings();
  void populateDevicesComboBox();
  void touchMissingProfiles();
  void refreshDisplayFromGearHandlerSettings();

  void cycleGamepadProfile();

 private:
  Ui::MainWindow *ui;
  QSystemTrayIcon m_trayIcon;

  bool m_ignore_kb_events{false};
  bool m_has_keyboard_conflict{false};

  bool m_wasUpdated;
  updt::UpdateHandler *m_updateHandler;

  Widget_gearDisplay *m_gearDisplay;

  Settings m_softSettings;
  const QString c_appDataFolder;
  const QString c_softSettingsFile;

  tc::GearHandler m_gearHandler;
  qsdl::GameController m_controller;

  QPixmap m_pixmapBg;
  static constexpr auto kDefaultBgPath{":/img/img/background.jpeg"};
};

inline QString MainWindow::getProfileFilePath(const QString &deviceName) {
  return ::getProfileFilePath(c_appDataFolder, deviceName);
}
inline QString MainWindow::getCurrentProfileFilePath() {
  return getProfileFilePath(m_gearHandler.settings().profileName);
}

#endif  // MAINWINDOW_H
