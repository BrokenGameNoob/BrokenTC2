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

#include "mainwindow.h"

#include <QTimer>
#include <SimpleUpdater.hpp>

#include "./ui_mainwindow.h"
#include "Constants.hpp"
#include "Update/PostUpdate.hpp"
#include "Update/Update.hpp"
#include "Utils/Dialog_About.hpp"
#include "Utils/Dialog_getGameControllerButton.hpp"
#include "Utils/Dialog_getKeyCode.hpp"
#include "global.hpp"

#ifdef Q_OS_WIN
#include <windows.h>

#include "Windows/WinEventHandler.hpp"
#include "Windows/WinUtils.hpp"
#endif

#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QStandardPaths>
#include <QWindow>
#include <limits>

#include "LoggerHandler.hpp"
#include "QSDL/SDLEventHandler.hpp"
#include "QSDL/SDLGlobal.hpp"
#include "TC/Dialog_ConfigureGame.hpp"
#include "TC/GearHandler.hpp"
#include "TC/Profile.hpp"
#include "Utils/JSONTools.hpp"

namespace {

void RefreshCheckBoxText(QCheckBox *cb) {
  cb->setText(cb->isChecked() ? QObject::tr("Yes") : QObject::tr("No"));
}

QString getKeyOrButtonText(int keyCode, bool useVkCodeChar) {
  if (keyCode < 0) return "- -";

  QString text{" "};
  if (useVkCodeChar)
    text = win::vkCodeToStr(keyCode).toUpper();
  else {
    if (keyCode >= 1000)  // gamepad axis
    {
      text += "Axis: ";
      text += QString::number((keyCode - 1000) / 10);
      text += " Dir: ";
      text += QString::number(keyCode % 10);
    } else {
      text += QString::number(keyCode);
    }
  }
  return text;
}

void setKey(int keyCode, QLabel *lblDisp, tc::ProfileSettings::Key &settingsKeyToChange) {
  if (keyCode == 0)  // means do nothing
  {
    return;
  }
  lblDisp->setText(getKeyOrButtonText(keyCode, true));
  settingsKeyToChange = keyCode;  // if key == -1, it will never be matched = Unbind
}

inline void setButton(int button, QLabel *lblDisp, tc::ProfileSettings::Key &settingsBtnToChange) {
  if (button < -1)  // means do nothing
  {
    return;
  }
  lblDisp->setText(getKeyOrButtonText(button, false));
  settingsBtnToChange = button;  // if button == -1, it will never be matched = Unbind
}

inline QString screenName(const QScreen *screen) {
  static QRegularExpression reToRemove{"(\\.)|(/)|(\\\\)"};
  return screen->name().remove(reToRemove);
}

// inline
// std::unique_ptr<wchar_t[]> toWchar(const std::string& str){
//     // required size
//     int nChars = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
//     // allocate it
//     std::unique_ptr<wchar_t[]> out{new WCHAR[nChars]};
//     MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (LPWSTR)(out.get()),
//     nChars); return out;
// }

#ifdef Q_OS_WIN

bool reg_startOnStartupExist() {
  constexpr auto cRunPath{"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"};
  HKEY hKey{};
  auto lResult{RegOpenKeyExA(HKEY_CURRENT_USER, cRunPath, 0, KEY_READ, &hKey)};
  if (lResult == ERROR_SUCCESS) {
    lResult = RegQueryValueExA(hKey, PROJECT_NAME, NULL, NULL, NULL, NULL);
    if (lResult == ERROR_SUCCESS) {
      RegCloseKey(hKey);
      return true;
    }
  }
  return false;
}
bool reg_startOnStartup(bool enableAutoStart) {
  constexpr auto runPath{L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"};
  constexpr auto cRunPath{"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"};
  HKEY hKey{};
  auto lResult{RegOpenKeyEx(HKEY_CURRENT_USER, runPath, 0, KEY_READ | KEY_WRITE, &hKey)};

  if (lResult != ERROR_SUCCESS) {
    if (lResult == ERROR_FILE_NOT_FOUND) {
      qCritical() << __CURRENT_PLACE__ << ": Registry key not found : " << cRunPath;
    } else {
      qCritical() << __CURRENT_PLACE__ << ": Error occured when opening registry key : " << cRunPath;
    }
    return false;
  }

  auto stdStrExePath{QCoreApplication::applicationFilePath().replace("/", "\\").toStdString() + " --hide"};
  auto cExePath{stdStrExePath.c_str()};

  if (enableAutoStart) {
    lResult = RegSetValueExA(
        hKey, PROJECT_NAME, 0, REG_SZ, reinterpret_cast<const BYTE *>(cExePath), size(stdStrExePath) + 1);
  } else {
    lResult = RegDeleteKeyValueA(hKey, NULL, PROJECT_NAME);
  }

  if (lResult != ERROR_SUCCESS) {
    if (lResult == ERROR_FILE_NOT_FOUND) {
      qCritical() << __CURRENT_PLACE__ << ": Registry key not found : " << cRunPath;
    } else {
      qCritical() << __CURRENT_PLACE__ << ": Error occured when writing registry key : " << cRunPath;
      qCritical() << "To : " << QCoreApplication::applicationFilePath();
    }
    RegCloseKey(hKey);
    return false;
  }

  RegCloseKey(hKey);
  return true;
}
#endif

void SetLabelHasConflict(QLabel *lbl, const QString &conflict_with) {
  lbl->setStyleSheet("background-color:#ffcccc");
  lbl->setToolTip(QObject::tr("This key is in conflict with <%1>").arg(conflict_with));
}

void ClearLabelConflict(QLabel *lbl) {
  lbl->setStyleSheet("");
  lbl->setToolTip("");
}

}  // namespace

void MainWindow::Settings::setBgHUDColor(QColor c, Widget_gearDisplay *m_gearDisplay) {
  m_bgHUDColor = std::move(c);
  m_gearDisplay->setBgHUDColor(m_bgHUDColor);
}

MainWindow::MainWindow(bool hideOnStartup, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_trayIcon{QIcon{":/img/img/softPic.png"}, this},
      m_updateHandler{new updt::UpdateHandler(consts::CURRENT_VERSION, consts::PROJECT_GITHUB_RELEASE,
                                              consts::PUBLIC_VERIFIER_KEY_FILE, true, consts::POST_UPDATE_CMD, true,
                                              this)},
      m_gearDisplay{new Widget_gearDisplay()},
      m_softSettings{},
      c_appDataFolder{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/"},
      c_softSettingsFile{c_appDataFolder + "settings.conf"},
      m_controller{},
      m_pixmapBg{} {
  m_pixmapBg.load(kDefaultBgPath);
  if (m_pixmapBg.isNull()) {
    qCritical() << "Could not load background";
  }
  // UI Init
  ui->setupUi(this);
  ui->statusbar->addPermanentWidget(new QLabel{PROJECT_VERSION, this});

  // Correct line appearances
  auto lineList{this->findChildren<QFrame *>(QRegularExpression{"^line_[0-9]*$"})};
  for (auto &e : lineList) {
    e->setMidLineWidth(1);
  }

  // tray icon
  auto trayIconMenu{new QMenu(this)};
  auto tmpAction = trayIconMenu->addAction(tr("Show window"));
  connect(tmpAction, &QAction::triggered, this, [&]() { this->show(); });
  tmpAction = trayIconMenu->addAction(tr("Exit program"));
  connect(tmpAction, &QAction::triggered, this, [&]() { this->close(); });
  m_trayIcon.setContextMenu(trayIconMenu);

  connect(&m_trayIcon, &QSystemTrayIcon::activated, this, [&](auto reason) {
    if (reason == QSystemTrayIcon::ActivationReason::Trigger) {
      this->show();
    }
  });
  if (hideOnStartup)  // through command line option
  {
    m_trayIcon.hide();
    m_trayIcon.show();
  }

  connect(ui->toolB_help, &QToolButton::clicked, this, [&]() {
    constexpr auto helpLink{"https://github.com/BrokenGameNoob/BrokenTC2"};
    qInfo() << __CURRENT_PLACE__ << " : open " << helpLink;
    if (!QDesktopServices::openUrl(QUrl{helpLink})) {
      QMessageBox::warning(
          this, tr("Can't help"), tr("Sorry, we can't open help. Go to this page :\n%0").arg(helpLink));
    }
  });
  connect(ui->pb_userManual, &QToolButton::clicked, this, [&]() {
    constexpr auto helpLink{"https://github.com/BrokenGameNoob/BrokenTC2#quick-guide"};
    qInfo() << __CURRENT_PLACE__ << " : open " << helpLink;
    if (!QDesktopServices::openUrl(QUrl{helpLink})) {
      QMessageBox::warning(
          this, tr("Can't help"), tr("Sorry, we can't open help. Go to this page :\n%0").arg(helpLink));
    }
  });

#ifdef Q_OS_WIN

  // start keyboard event handler
  connect(
      windows::WindowsEventThread::ins(), &windows::WindowsEventThread::keyDown, this, &MainWindow::onKeyboardPressed);
#endif

  qsdl::SDLEventHandler::start();
  qsdl::SDLEventHandler::registerController(&m_controller);

  /* Keyboard inputs */

  connect(ui->pb_selectKey_GReverse, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_GReverse, m_gearHandler.settings().reverse);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_GClutch, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_GClutch, m_gearHandler.settings().clutch);
    saveProfileSettings();
    UpdateConflicts();
  });
  connect(ui->pb_selectKey_G1, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_G1, m_gearHandler.settings().g1);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_G2, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_G2, m_gearHandler.settings().g2);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_G3, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_G3, m_gearHandler.settings().g3);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_G4, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_G4, m_gearHandler.settings().g4);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_G5, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_G5, m_gearHandler.settings().g5);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_G6, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_G6, m_gearHandler.settings().g6);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_G7, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_G7, m_gearHandler.settings().g7);
    saveProfileSettings();
  });

  connect(ui->pb_selectKey_SeqUp, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_seqUp, m_gearHandler.settings().seqGearUp);
    saveProfileSettings();
    UpdateConflicts();
  });
  connect(ui->pb_selectKey_SeqDown, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_seqDown, m_gearHandler.settings().seqGearDown);
    saveProfileSettings();
    UpdateConflicts();
  });

  connect(ui->pb_selectButton_GUp, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_GUp, m_gearHandler.settings().gearUp);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_GDown, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_GDown, m_gearHandler.settings().gearDown);
    saveProfileSettings();
  });

  connect(ui->pb_selectKey_kSwitchMode, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_kSwitchMode, m_gearHandler.settings().kSwitchMode);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_kCycleProfile, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_kCycleProfile, m_gearHandler.settings().kCycleProfile);
    saveProfileSettings();
  });
  connect(ui->pb_selectKey_keyboardGearUp, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_keyboardGearUp, m_gearHandler.settings().keyboardSeqGearUp);
    saveProfileSettings();
    UpdateConflicts();
  });
  connect(ui->pb_selectKey_keyboardGearDown, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_keyboardGearDown, m_gearHandler.settings().keyboardSeqGearDown);
    saveProfileSettings();
    UpdateConflicts();
  });
  connect(ui->pb_selectKey_disableSoftware, &QPushButton::clicked, this, [&]() {
    auto key{GetKey()};
    setKey(key, ui->lbl_keyboardDisableSoftware, m_gearHandler.settings().keyboardDisableSoftware);
    saveProfileSettings();
  });

  /* CONTROLLER */

  connect(ui->pb_selectButton_switchMode, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_switchMode, m_gearHandler.settings().switchMode);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_cycleProfile, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_cycleProfile, m_gearHandler.settings().cycleProfile);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_disableSoftware, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_disableSoftware, m_gearHandler.settings().disableSoftware);
    saveProfileSettings();
  });

  connect(ui->pb_selectButton_gearR, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gearR, m_gearHandler.settings().setReverseGear);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_gear1, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gear1, m_gearHandler.settings().setFirstGear);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_gear2, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gear2, m_gearHandler.settings().setSecondGear);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_gear3, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gear3, m_gearHandler.settings().setThirdGear);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_gear4, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gear4, m_gearHandler.settings().setFourthGear);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_gear5, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gear5, m_gearHandler.settings().setFifthGear);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_gear6, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gear6, m_gearHandler.settings().setSixthGear);
    saveProfileSettings();
  });
  connect(ui->pb_selectButton_gear7, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_gear7, m_gearHandler.settings().setSeventhGear);
    saveProfileSettings();
  });
  connect(ui->pb_holdFirstGear, &QPushButton::clicked, this, [&]() {
    auto btn{Dialog_getGameControllerButton::getButton(&m_controller, this)};
    setButton(btn, ui->lbl_btn_holdFirstGear, m_gearHandler.settings().setHoldFirstGear);
    saveProfileSettings();
  });
  connect(ui->cb_holdFirstGearWithClutch, &QCheckBox::stateChanged, this, [&](int val) {
    m_gearHandler.settings().holdFirstGearWithClutch = val;
    saveProfileSettings();
    RefreshCheckBoxText(this->ui->cb_holdFirstGearWithClutch);
  });

  /* MISC options */

  connect(ui->sb_gearDelay, &QSpinBox::valueChanged, this, [&](int val) {
    m_gearHandler.settings().keyDownTime = val;
    saveProfileSettings();
  });

  connect(ui->cb_skip_neutral, &QCheckBox::stateChanged, this, [&](int val) {
    m_gearHandler.settings().skipNeutral = val;
    saveProfileSettings();
    RefreshCheckBoxText(this->ui->cb_skip_neutral);
  });

  connect(ui->cb_use_seq_after_clutch, &QCheckBox::stateChanged, this, [&](int val) {
    m_gearHandler.settings().useSequentialAfterClutch = val;
    saveProfileSettings();
    RefreshCheckBoxText(this->ui->cb_use_seq_after_clutch);
  });

  // init threshold slider (set max value to int16_t max val)
  ui->hs_joyAxisThreshold->setMaximum(std::numeric_limits<decltype(m_softSettings.joyAxisThreshold())>::max() -
                                      1);  // int16_t
  connect(ui->hs_joyAxisThreshold, &QSlider::valueChanged, this, [&](int val) {
    m_softSettings.setJoyAxisThreshold(static_cast<int16_t>(val));
    saveSoftSettings();
  });
  connect(ui->cb_gearDisplayScreen, &QComboBox::currentIndexChanged, this, [&](int) {
    m_softSettings.displayGearScreen = ui->cb_gearDisplayScreen->currentText();

    m_gearDisplay->showOnScreen(ui->cb_gearDisplayScreen->currentData().toInt());
    saveSoftSettings();
  });
  connect(ui->cb_enableNotification, &QCheckBox::stateChanged, this, [&](int) {
    m_softSettings.enableNotification = ui->cb_enableNotification->isChecked();
    saveSoftSettings();
  });

  connect(ui->cb_ignoreVJoy, &QCheckBox::stateChanged, this, [&](int val) {
    m_softSettings.ignoreVJoyProfile = val;
    qInfo() << "Ignore VJoy profile: " << m_softSettings.ignoreVJoyProfile;
    saveSoftSettings();
    RefreshCheckBoxText(this->ui->cb_ignoreVJoy);
    populateDevicesComboBox();
  });

  connect(ui->cb_exitOnCloseEvent, &QComboBox::currentIndexChanged, this, [&](int valI) {
    m_softSettings.exitOnCloseEvent = bool(valI);
    saveSoftSettings();
  });
#ifdef Q_OS_WIN
  connect(ui->cb_launchOnStartup, &QComboBox::currentIndexChanged, this, [&](int valI) {
    reg_startOnStartup(bool(valI));
    saveSoftSettings();
  });
#endif
  connect(ui->tb_settings, &QTabWidget::currentChanged, this, [&](int index) { m_softSettings.openedTab = index; });

  connect(&m_gearHandler, &tc::GearHandler::gearChanged, m_gearDisplay, &Widget_gearDisplay::refreshGear);
  connect(&m_gearHandler,
          &tc::GearHandler::gearSwitchModeChanged,
          m_gearDisplay,
          &Widget_gearDisplay::onSwitchGearModeChanged);
  connect(&m_gearHandler, &tc::GearHandler::gearSwitchModeChanged, this, [&](auto) { saveProfileSettings(); });
  connect(&m_gearHandler,
          &tc::GearHandler::softwareEnabledChanged,
          m_gearDisplay,
          &Widget_gearDisplay::onSoftwareEnabledChanged);
  auto lambda_updateSoftwareEnabledChanged{
      [&](bool enabled) { ui->lbl_softEnabled->setText(enabled ? tr("Enabled") : tr("Disabled")); }};
  connect(&m_gearHandler, &tc::GearHandler::softwareEnabledChanged, this, lambda_updateSoftwareEnabledChanged);
  lambda_updateSoftwareEnabledChanged(m_gearHandler.isEnabled());

  using qsdl::GameController;
  using qsdl::SDLEventHandler;
  connect(SDLEventHandler::instance(), &SDLEventHandler::gameControllerAdded, this, &MainWindow::onControllerPluggedIn);
  connect(
      SDLEventHandler::instance(), &SDLEventHandler::gameControllerRemoved, this, &MainWindow::onControllerUnplugged);

  connect(&m_controller, &GameController::buttonDown, this, &MainWindow::onControllerButtonPressed);
  connect(&m_controller, &GameController::buttonUp, this, &MainWindow::onControllerButtonReleased);  // New Feature

  //--------------------------------------------------------------------

  qDebug() << __CURRENT_PLACE__ << "   " << getCurrentProfileFilePath();
  if (!QFileInfo::exists(c_appDataFolder))  // if the appdata folder doesn't exist
  {
    if (!QDir::root().mkpath(c_appDataFolder)) {
      throw std::runtime_error{__CURRENT_PLACE_std_ + " : Cannot create appData folder : <" +
                               c_appDataFolder.toStdString() + ">"};
    }
  }

  populateDevicesComboBox();

  auto availableScreens{QApplication::screens()};
  int i{};
  std::map<QString, int32_t> screenCount{};
  qInfo() << "Found " << availableScreens.size() << " screens";
  for (const auto &e : availableScreens) {
    qInfo() << "\t-> " << screenName(e);
  }
  for (const auto &e : availableScreens) {
    const auto kTmpScreenName{screenName(e)};
    screenCount[kTmpScreenName]++;
    const auto kDisplayName{screenCount[kTmpScreenName] > 1
                                ? QString{"%0_%1"}.arg(kTmpScreenName).arg(screenCount[kTmpScreenName])
                                : std::move(kTmpScreenName)};
    ui->cb_gearDisplayScreen->addItem(kDisplayName, i);
    ++i;
  }

  m_softSettings.setBgHUDColor(Settings{}.bgHUDColor(), m_gearDisplay);
  loadSoftSettings();

  if (!win::setCoreCountAffinity(m_softSettings.preferredCoreCount, false, true)) {
    qCritical() << __PRETTY_FUNCTION__ << " Can't set cpu cores affinity";
  }

  // try to read settings profile
  if (!QFileInfo::exists(getCurrentProfileFilePath())) {
    qDebug() << "Saving initial configuration";
    saveProfileSettings();
  } else {
    loadProfileSettings();
  }

  refreshDisplayFromGearHandlerSettings();

  m_softSettings.isInit = true;
  on_cb_showCurrentGear_stateChanged(ui->cb_showCurrentGear->isChecked());
  updateSoftSettings();
  refreshFromSettings();

  setBackgroungImage(m_softSettings.backgroundImagePath);

  // UPDATES

  std::function<void(MainWindow *)> toCallIfUpdated = [](MainWindow *) { qDebug() << "Updated!"; };
  m_wasUpdated = updt::acquireUpdated(toCallIfUpdated, lupdt::UPDATED_TAG_FILENAME, this);

  qInfo() << "------------"
          << "Program state:";
  qInfo() << "\tAppData folder (storing profiles):" << c_appDataFolder;
  qInfo() << "\tCurrently loaded settings file:" << c_softSettingsFile;
  qInfo() << "\tWas updated?" << m_wasUpdated;
}

MainWindow::~MainWindow() {
  m_softSettings.openedTab = ui->tb_settings->currentIndex();
  saveSoftSettings();

  m_gearDisplay->deleteLater();
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
  __asm("nop");
  if (event->spontaneous() && !(m_softSettings.exitOnCloseEvent))  // if the window is closed by the top
                                                                   // right closing icon
  {
    this->hide();
    m_trayIcon.show();
    event->setAccepted(false);
  } else  // closed programmatically (or expected to be so?)
  {
    m_gearDisplay->close();
    QMainWindow::closeEvent(event);
    QApplication::exit(0);
  }
}

void MainWindow::showEvent(QShowEvent *event)  // when the window is shown
{
  QMainWindow::showEvent(event);

  if (m_softSettings.displayAboutOnStartup) { /*
                                                 on_action_about_triggered();*/
    m_softSettings.displayAboutOnStartup = false;
    saveSoftSettings();

    QMetaObject::invokeMethod(this,
                              "on_action_about_triggered",
                              Qt::ConnectionType::QueuedConnection);  // call it after function
                                                                      // terminated

    on_pb_ezConf_clicked();
  }

  if (m_wasUpdated) {
    QMetaObject::invokeMethod(
        this, [this]() { updt::showChangelog(this); }, Qt::ConnectionType::QueuedConnection);
  }

  m_trayIcon.hide();
}

int32_t MainWindow::GetKey() {
  m_ignore_kb_events = true;
  Dialog_getKeyCode::KeyCode key{Dialog_getKeyCode::getKey(this)};
  m_ignore_kb_events = false;
  return key.code;
}

//------------------------------------------------------------------
//
//--------   Soft settings
//
//------------------------------------------------------------------

bool MainWindow::setBackgroungImage(const QString &newPath) {
  if (!QFileInfo::exists(newPath) && !newPath.isEmpty()) {
    qWarning() << "Background image not found: " << newPath;
    QMessageBox::warning(this, tr("Warning"), tr("Background image not found") + QString{"\n%0"}.arg(newPath));
    setBackgroungImage(kDefaultBgPath);
    return false;
  }

  m_pixmapBg.load(newPath);
  if (m_pixmapBg.isNull() && !newPath.isEmpty()) {
    qWarning() << "Invalid background image: " << newPath;
    QMessageBox::warning(this, tr("Warning"), tr("Invalid background image") + QString{"\n%0"}.arg(newPath));
    return setBackgroungImage(kDefaultBgPath);
  }
  // valid
  if (newPath.isEmpty()) {
    setBackgroungImage(kDefaultBgPath);
  }

  ui->pb_changeBackground->setText(newPath == kDefaultBgPath || newPath.isEmpty() ? tr("Click to select") : newPath);
  m_softSettings.backgroundImagePath = newPath;
  saveSoftSettings();
  return true;
}

void MainWindow::updateSoftSettings() {
  // refresh settings from UI
  m_softSettings.currentDeviceName =
      ((ui->cb_selectDevice->currentText().isEmpty()) ? m_softSettings.currentDeviceName
                                                      : ui->cb_selectDevice->currentText());
  m_softSettings.gearDisplayed = ui->cb_showCurrentGear->isChecked();
  m_softSettings.enableNotification = ui->cb_enableNotification->isChecked();
  m_softSettings.displayGearScreen = ui->cb_gearDisplayScreen->currentText();

  saveSoftSettings();
}

bool MainWindow::saveSoftSettings() {
  if (!m_softSettings.isInit)  // if settings still not initialized
    return true;               // they won't be saved

  QJsonObject globObj{};

  QJsonObject settings{};
  //    settings.insert("launchOnComputerStartup",m_softSettings.launchOnComputerStartup);
  settings.insert("displayAboutOnStartup", m_softSettings.displayAboutOnStartup);
  settings.insert("lastProfile", m_softSettings.currentDeviceName);
  settings.insert("displayGear", m_softSettings.gearDisplayed);
  settings.insert("enableNotification", m_softSettings.enableNotification);
  settings.insert("displayGearScreen", m_softSettings.displayGearScreen);
  settings.insert("lowPerfMode", m_softSettings.lowPerfMode());
  settings.insert("exitOnCloseEvent", m_softSettings.exitOnCloseEvent);
  settings.insert("openedTab", m_softSettings.openedTab);
  settings.insert("preferredCoreCount", m_softSettings.preferredCoreCount);
  settings.insert("bgHUDColor", tc::colorToString(m_softSettings.bgHUDColor()));
  settings.insert("joyAxisThreshold", m_softSettings.joyAxisThreshold());
  settings.insert("backgroundImagePath", m_softSettings.backgroundImagePath);
  settings.insert("ignoreVJoyProfile", m_softSettings.ignoreVJoyProfile);

  globObj.insert("settings", settings);

  return utils::json::save(globObj, c_softSettingsFile);
}

bool MainWindow::loadSoftSettings() {
  utils::json::read(c_softSettingsFile);
  auto docOpt{utils::json::read(c_softSettingsFile)};

  if (!docOpt)  // if we could not read the settings file
  {
    if (QFileInfo::exists(c_softSettingsFile))  // if the config file exist
      throw std::runtime_error{__CURRENT_PLACE_std_ + " : " + std::string{"Cannot read settings file "} +
                               c_softSettingsFile.toStdString()};
    if (!saveSoftSettings())  // if we fail to create a settings file
    {
      throw std::runtime_error{__CURRENT_PLACE_std_ + " : " + std::string{"Cannot create settings file "} +
                               c_softSettingsFile.toStdString()};
    } else  // if we saved new settings
    {
      return true;
    }
  }

  auto docObj{docOpt.value().object()};

  Settings def{};

  auto settings{docObj.value("settings").toObject()};
  //    out.launchOnComputerStartup =
  //    settings.value("launchOnComputerStartup").toBool(false);
  m_softSettings.displayAboutOnStartup = settings.value("displayAboutOnStartup").toBool(true);
  m_softSettings.currentDeviceName = settings.value("lastProfile").toString();
  m_softSettings.gearDisplayed = settings.value("displayGear").toBool();
  m_softSettings.enableNotification = settings.value("enableNotification").toBool();
  m_softSettings.setLowPerfMode(settings.value("lowPerfMode").toBool());
  m_softSettings.displayGearScreen = settings.value("displayGearScreen").toString();
  m_softSettings.exitOnCloseEvent = settings.value("exitOnCloseEvent").toBool(false);
  m_softSettings.openedTab = settings.value("openedTab").toInt(def.openedTab);
  m_softSettings.preferredCoreCount = settings.value("preferredCoreCount").toInt(def.preferredCoreCount);
  auto bgHUDColorStr{settings.value("bgHUDColor").toString()};
  m_softSettings.setBgHUDColor(bgHUDColorStr.isEmpty() ? QColor{79, 79, 79, 120} : tc::stringToColor(bgHUDColorStr),
                               m_gearDisplay);
  m_softSettings.setJoyAxisThreshold(
      static_cast<int16_t>(settings.value("joyAxisThreshold")
                               .toInt(std::numeric_limits<decltype(m_softSettings.joyAxisThreshold())>::max() - 1)));
  m_softSettings.backgroundImagePath = settings.value("backgroundImagePath").toString();
  m_softSettings.ignoreVJoyProfile = settings.value("ignoreVJoyProfile").toBool();

  refreshFromSettings();

  return true;
}

//------------------------------------------------------------------
//
//--------   TC profiles settings save/load
//
//------------------------------------------------------------------

void MainWindow::saveProfileSettings() {
  if (m_gearHandler.settings().profileName.isEmpty()) return;

  auto fileName{getCurrentProfileFilePath()};

  if (!tc::saveSettings(m_gearHandler.settings(), fileName)) {
    qCritical() << "Could not save gearHandler settings as " << fileName;
    QMessageBox::critical(this, tr("Error"), tr("Cannot save gear keys and settings to the file\n%0").arg(fileName));
  }
}

bool MainWindow::loadProfileSettings() {
  qDebug() << "Loading profile: " << getCurrentProfileFilePath();
  try {
    m_gearHandler.settings() = tc::readProfileSettings(getCurrentProfileFilePath());
    m_gearHandler.setGearSwitchMode(m_gearHandler.settings().gearSwitchMode);
    UpdateConflicts();
  } catch (const std::runtime_error &e) {
    qCritical() << "Cannot read gearHandler settings file";
    QMessageBox::critical(
        this, tr("Error"), tr("Cannot read gear keys and settings from the file\n%0").arg(getCurrentProfileFilePath()));
    return false;
  }
  return true;
}

bool MainWindow::loadProfile(QString gamePadName) {
  auto settingsSave{m_gearHandler.settings()};

  m_gearHandler.settings().profileName = gamePadName;

  if (!loadProfileSettings()) {
    m_gearHandler.settings() = settingsSave;  // restore old profile name
    return false;
  }
  return true;
}

//------------------------------------------------------------------
//
//--------   Display
//
//------------------------------------------------------------------

void MainWindow::refreshFromSettings() {
#ifdef Q_OS_WIN
  auto launchOnStartup{reg_startOnStartupExist()};
  ui->cb_launchOnStartup->setCurrentIndex(int(launchOnStartup));
#endif
  ui->cb_showCurrentGear->setChecked(m_softSettings.gearDisplayed);
  ui->cb_enableNotification->setChecked(m_softSettings.enableNotification);
  ui->cb_lowPerfMode->setCurrentIndex(int(m_softSettings.lowPerfMode()));

  auto deviceIndex{ui->cb_selectDevice->findText(m_softSettings.currentDeviceName)};
  if (deviceIndex >= 0) {
    ui->cb_selectDevice->setCurrentIndex(deviceIndex);
  }

  auto screenIndex{ui->cb_gearDisplayScreen->findText(m_softSettings.displayGearScreen)};
  if (screenIndex >= 0) {
    ui->cb_gearDisplayScreen->setCurrentIndex(screenIndex);
  } else {
    ui->cb_gearDisplayScreen->setCurrentIndex(-1);
  }

  ui->cb_ignoreVJoy->setChecked(m_softSettings.ignoreVJoyProfile);
  ::RefreshCheckBoxText(ui->cb_ignoreVJoy);

  ui->cb_exitOnCloseEvent->setCurrentIndex(int(m_softSettings.exitOnCloseEvent));

  ui->tb_settings->setCurrentIndex(m_softSettings.openedTab);

  ui->hs_joyAxisThreshold->setValue(m_softSettings.joyAxisThreshold());

  ui->lbl_bgHUDColor->setStyleSheet(QString{"background-color:%0"}.arg(tc::colorToString(m_softSettings.bgHUDColor())));
  ui->sb_bgHUDColorAlpha->setValue(m_softSettings.bgHUDColor().alpha());

  ui->pb_changeBackground->setText(m_softSettings.backgroundImagePath);
}

void MainWindow::populateDevicesComboBox() {
  auto curDevice{ui->cb_selectDevice->currentText()};
  if (curDevice.isEmpty()) curDevice = m_softSettings.currentDeviceName;

  ui->cb_selectDevice->clear();

  auto deviceList{qsdl::getPluggedJoysticks()};

  auto newDeviceIndex{-1};
  int i{};
  for (const auto &e : deviceList) {
    if (m_softSettings.ignoreVJoyProfile) {
      if (e.contains("vJoy", Qt::CaseSensitivity::CaseInsensitive)) {
        continue;
      }
    }
    if (curDevice == e) newDeviceIndex = i;
    ui->cb_selectDevice->addItem(e, i);  // store device id as data. Even though it should match cb index
    ++i;
  }
  ui->cb_selectDevice->setCurrentIndex(newDeviceIndex);
}

void MainWindow::refreshDisplayFromGearHandlerSettings() {
  auto lambdaUpdateText{[&](QLabel *lbl, auto newCode, bool useVkCodeChar = true) {
    lbl->setText(getKeyOrButtonText(newCode, useVkCodeChar));
  }};

  lambdaUpdateText(ui->lbl_GReverse, m_gearHandler.settings().reverse);
  lambdaUpdateText(ui->lbl_GClutch, m_gearHandler.settings().clutch);
  lambdaUpdateText(ui->lbl_G1, m_gearHandler.settings().g1);
  lambdaUpdateText(ui->lbl_G2, m_gearHandler.settings().g2);
  lambdaUpdateText(ui->lbl_G3, m_gearHandler.settings().g3);
  lambdaUpdateText(ui->lbl_G4, m_gearHandler.settings().g4);
  lambdaUpdateText(ui->lbl_G5, m_gearHandler.settings().g5);
  lambdaUpdateText(ui->lbl_G6, m_gearHandler.settings().g6);
  lambdaUpdateText(ui->lbl_G7, m_gearHandler.settings().g7);
  ui->cb_holdFirstGearWithClutch->setChecked(m_gearHandler.settings().holdFirstGearWithClutch);
  ::RefreshCheckBoxText(ui->cb_holdFirstGearWithClutch);

  lambdaUpdateText(ui->lbl_seqUp, m_gearHandler.settings().seqGearUp);
  lambdaUpdateText(ui->lbl_seqDown, m_gearHandler.settings().seqGearDown);

  lambdaUpdateText(ui->lbl_kSwitchMode, m_gearHandler.settings().kSwitchMode);
  lambdaUpdateText(ui->lbl_kCycleProfile, m_gearHandler.settings().kCycleProfile);
  lambdaUpdateText(ui->lbl_keyboardGearUp, m_gearHandler.settings().keyboardSeqGearUp);
  lambdaUpdateText(ui->lbl_keyboardGearDown, m_gearHandler.settings().keyboardSeqGearDown);
  lambdaUpdateText(ui->lbl_keyboardDisableSoftware, m_gearHandler.settings().keyboardDisableSoftware);
  ui->sb_gearDelay->setValue(m_gearHandler.settings().keyDownTime);

  ui->cb_skip_neutral->setChecked(m_gearHandler.settings().skipNeutral);
  ::RefreshCheckBoxText(ui->cb_skip_neutral);
  ui->cb_use_seq_after_clutch->setChecked(m_gearHandler.settings().useSequentialAfterClutch);
  ::RefreshCheckBoxText(ui->cb_use_seq_after_clutch);

  lambdaUpdateText(ui->lbl_btn_GUp, m_gearHandler.settings().gearUp, false);
  lambdaUpdateText(ui->lbl_btn_GDown, m_gearHandler.settings().gearDown, false);

  lambdaUpdateText(ui->lbl_btn_gearR, m_gearHandler.settings().setReverseGear, false);
  lambdaUpdateText(ui->lbl_btn_gear1, m_gearHandler.settings().setFirstGear, false);
  lambdaUpdateText(ui->lbl_btn_gear2, m_gearHandler.settings().setSecondGear, false);
  lambdaUpdateText(ui->lbl_btn_gear3, m_gearHandler.settings().setThirdGear, false);
  lambdaUpdateText(ui->lbl_btn_gear4, m_gearHandler.settings().setFourthGear, false);
  lambdaUpdateText(ui->lbl_btn_gear5, m_gearHandler.settings().setFifthGear, false);
  lambdaUpdateText(ui->lbl_btn_gear6, m_gearHandler.settings().setSixthGear, false);
  lambdaUpdateText(ui->lbl_btn_gear7, m_gearHandler.settings().setSeventhGear, false);
  lambdaUpdateText(ui->lbl_btn_holdFirstGear, m_gearHandler.settings().setHoldFirstGear, false);

  lambdaUpdateText(ui->lbl_btn_switchMode, m_gearHandler.settings().switchMode, false);
  lambdaUpdateText(ui->lbl_btn_cycleProfile, m_gearHandler.settings().cycleProfile, false);
  lambdaUpdateText(ui->lbl_btn_disableSoftware, m_gearHandler.settings().disableSoftware, false);
}

void MainWindow::cycleGamepadProfile() {
  auto gamePadCount{ui->cb_selectDevice->count()};
  if (gamePadCount <= 1) {
    return;
  }
  auto nextIndex{(ui->cb_selectDevice->currentIndex() + 1) % gamePadCount};
  ui->cb_selectDevice->setCurrentIndex(nextIndex);

  const auto &deviceTitle{ui->cb_selectDevice->currentText()};
  if (deviceTitle.isEmpty()) {
    return;
  }

  qInfo() << "Selected device from cycle:" << deviceTitle;

  if (ui->cb_enableNotification->isChecked()) {
    m_gearDisplay->showNotif(deviceTitle);
  }
}

//------------------------------------------------------------------
//
//--------   Qt slots
//
//------------------------------------------------------------------

void MainWindow::UpdateConflicts() {
  bool has_at_least_one_conflict{false};
  if (m_gearHandler.settings().keyboardSeqGearUp == m_gearHandler.settings().seqGearUp) {
    ::SetLabelHasConflict(ui->lbl_keyboardGearUp, tr("In game config: %2").arg(ui->pb_selectKey_SeqUp->text()));
    ::SetLabelHasConflict(ui->lbl_seqUp, tr("Keyboard inputs: %2").arg(ui->pb_selectKey_keyboardGearUp->text()));
    has_at_least_one_conflict = has_at_least_one_conflict || true;
  } else {
    ::ClearLabelConflict(ui->lbl_keyboardGearUp);
    ::ClearLabelConflict(ui->lbl_seqUp);
  }

  if (m_gearHandler.settings().keyboardSeqGearDown == m_gearHandler.settings().seqGearDown) {
    ::SetLabelHasConflict(ui->lbl_keyboardGearDown, tr("In game config: %2").arg(ui->pb_selectKey_SeqDown->text()));
    ::SetLabelHasConflict(ui->lbl_seqDown, tr("Keyboard inputs: %2").arg(ui->pb_selectKey_keyboardGearDown->text()));
    has_at_least_one_conflict = has_at_least_one_conflict || true;
  } else {
    ::ClearLabelConflict(ui->lbl_keyboardGearDown);
    ::ClearLabelConflict(ui->lbl_seqDown);
  }

  bool gclutch_has_conflict{false};
  if (m_gearHandler.settings().keyboardSeqGearUp == m_gearHandler.settings().clutch) {
    ::SetLabelHasConflict(ui->lbl_keyboardGearUp, tr("In game config: %2").arg(ui->pb_selectKey_SeqUp->text()));
    ::SetLabelHasConflict(ui->lbl_GClutch, tr("Keyboard inputs: %2").arg(ui->pb_selectKey_GClutch->text()));
    has_at_least_one_conflict = has_at_least_one_conflict || true;
    gclutch_has_conflict = true;
  } else {
    ::ClearLabelConflict(ui->lbl_keyboardGearUp);
    ::ClearLabelConflict(ui->lbl_GClutch);
  }

  if (m_gearHandler.settings().keyboardSeqGearDown == m_gearHandler.settings().clutch) {
    ::SetLabelHasConflict(ui->lbl_keyboardGearDown, tr("In game config: %2").arg(ui->pb_selectKey_SeqDown->text()));
    has_at_least_one_conflict = has_at_least_one_conflict || true;
    gclutch_has_conflict = true;
  } else {
    ::ClearLabelConflict(ui->lbl_keyboardGearDown);
  }

  if (gclutch_has_conflict) {
    ::SetLabelHasConflict(ui->lbl_GClutch, tr("Keyboard inputs: %2").arg(ui->pb_selectKey_GClutch->text()));
  } else {
    ::ClearLabelConflict(ui->lbl_GClutch);
  }

  if (has_at_least_one_conflict) {
    m_has_keyboard_conflict = true;
    ui->lbl_conflict->setVisible(true);
  } else {
    m_has_keyboard_conflict = false;
    ui->lbl_conflict->setVisible(false);
  }
}

void MainWindow::onControllerPluggedIn(int id) {
  std::ignore = id;
  populateDevicesComboBox();
}
void MainWindow::onControllerUnplugged(int id) {
  std::ignore = id;
  populateDevicesComboBox();
}

void MainWindow::onControllerButtonPressed(int button) {
  static const QString background_pressed{"background-color:rgb(0,200,0)"};
  static const QString background_released{"background-color:rgb(190,0,0)"};

  auto lambdaForceGear{[&](tc::Gear g) {
    auto switchBack{false};
    if (m_gearHandler.mode() == tc::GearSwitchMode::SEQUENTIAL) {
      m_gearHandler.switchGearSwitchMode();
      switchBack = true;
    }
    m_gearHandler.setGear(g);
    if (switchBack) m_gearHandler.switchGearSwitchMode();
  }};

  auto setStyleSheet{[&](auto *lbl, const auto &style) { lbl->setStyleSheet(style); }};

  constexpr int displayDelay{150};

  if (button == m_gearHandler.settings().gearUp) {
    setStyleSheet(ui->lbl_pad_LB, background_pressed);
    m_gearHandler.gearUp();
    QTimer::singleShot(displayDelay, this, [&]() { setStyleSheet(ui->lbl_pad_LB, background_released); });
  } else if (button == m_gearHandler.settings().gearDown) {
    setStyleSheet(ui->lbl_pad_RB, background_pressed);
    m_gearHandler.gearDown();
    QTimer::singleShot(displayDelay, this, [&]() { setStyleSheet(ui->lbl_pad_RB, background_released); });
  } else if (button == m_gearHandler.settings().setReverseGear) {
    lambdaForceGear(tc::Gear::R);
  } else if (button == m_gearHandler.settings().setFirstGear) {
    lambdaForceGear(tc::Gear::G1);
  } else if (button == m_gearHandler.settings().setSecondGear) {
    lambdaForceGear(tc::Gear::G2);
  } else if (button == m_gearHandler.settings().setThirdGear) {
    lambdaForceGear(tc::Gear::G3);
  } else if (button == m_gearHandler.settings().setFourthGear) {
    lambdaForceGear(tc::Gear::G4);
  } else if (button == m_gearHandler.settings().setFifthGear) {
    lambdaForceGear(tc::Gear::G5);
  } else if (button == m_gearHandler.settings().setSixthGear) {
    lambdaForceGear(tc::Gear::G6);
  } else if (button == m_gearHandler.settings().setSeventhGear) {
    lambdaForceGear(tc::Gear::G7);
  } else if (button == m_gearHandler.settings().switchMode) {
    m_gearHandler.switchGearSwitchMode();
    if (ui->cb_enableNotification->isChecked()) m_gearDisplay->showGearModeChangeNotif(m_gearHandler.mode());
  } else if (button == m_gearHandler.settings().cycleProfile) {
    cycleGamepadProfile();
  } else if (button == m_gearHandler.settings().setHoldFirstGear) {
    m_gearHandler.holdFirstGear();
  } else if (button == m_gearHandler.settings().disableSoftware) {
    m_gearHandler.setEnabled(!m_gearHandler.isEnabled());
  }
}

void MainWindow::onControllerButtonReleased(int button) {
  if (button == m_gearHandler.settings().setHoldFirstGear) {
    m_gearHandler.releaseFirstGear();
  }
}

void MainWindow::onKeyboardPressed(int key) {
  if (m_ignore_kb_events) {
    return;
  }
  if (m_has_keyboard_conflict) {
    return;
  }
  if (key == m_gearHandler.settings().kSwitchMode) {
    m_gearHandler.switchGearSwitchMode();
    if (ui->cb_enableNotification->isChecked()) m_gearDisplay->showGearModeChangeNotif(m_gearHandler.mode());
  } else if (key == m_gearHandler.settings().kCycleProfile) {
    cycleGamepadProfile();
  } else if (key == m_gearHandler.settings().keyboardSeqGearUp) {
    m_gearHandler.gearUp();
  } else if (key == m_gearHandler.settings().keyboardSeqGearDown) {
    m_gearHandler.gearDown();
  } else if (key == m_gearHandler.settings().keyboardDisableSoftware) {
    m_gearHandler.setEnabled(!m_gearHandler.isEnabled());
  }
}

void MainWindow::on_cb_selectDevice_currentIndexChanged(int index) {
  if (!m_softSettings.isInit) return;

  if (index == -1 && qsdl::getPluggedJoysticksCount() == 0)  // no device selected and no available device connected
  {
    //        m_gearHandler.settings().profileName = {};//reset settings
    m_controller.disconnectController();  // disconnect controller
  } else if (index == -1 && ui->cb_selectDevice->count() > 0) {
    ui->cb_selectDevice->setCurrentIndex(0);
  } else {
    auto deviceName{ui->cb_selectDevice->currentText()};

    if (QFileInfo::exists(getProfileFilePath(deviceName)))  // If the profile file exists
    {
      if (!loadProfile(deviceName))  // and we can't load the profile
                                     // corresponding to the device
      {
        throw std::runtime_error{__CURRENT_PLACE_std_ + " : Cannot load profile for <" +
                                 getProfileFilePath(deviceName).toStdString() + ">"};
        return;
      }
    } else {
      // if the file doesn't exist, create the profile and save it
      m_gearHandler.settings() = {.profileName = deviceName};
    }
    updateSoftSettings();

    qInfo() << __PRETTY_FUNCTION__
            << "Connecting & registering new controller from mainwindow:" << ui->cb_selectDevice->currentData().toInt();
    m_controller.connectController(ui->cb_selectDevice->currentData().toInt());
    qsdl::SDLEventHandler::registerController(&m_controller);
  }

  refreshDisplayFromGearHandlerSettings();
}

void MainWindow::on_cb_showCurrentGear_stateChanged(int checked) {
  updateSoftSettings();

  if (checked) {
    auto screenId{ui->cb_gearDisplayScreen->currentData().toInt()};

    m_gearDisplay->showOnScreen(screenId);
    m_gearDisplay->setIndicatorVisible(true);
  } else {
    m_gearDisplay->setIndicatorVisible(false);
  }
}

void MainWindow::on_action_about_triggered() {
  Dialog_About dial(this);
  dial.exec();
}

void MainWindow::on_cb_lowPerfMode_currentIndexChanged(int arg1) {
  m_softSettings.setLowPerfMode(bool(arg1));
  saveSoftSettings();
}

void MainWindow::on_pb_bgHUDColor_clicked() {
  //    m_softSettings.bgHUDColor

  auto color{QColorDialog::getColor(m_softSettings.bgHUDColor(), this, tr("Select background HUD color"))};
  if (!color.isValid()) return;
  color.setAlpha(ui->sb_bgHUDColorAlpha->value());

  m_softSettings.setBgHUDColor(color, m_gearDisplay);
  saveSoftSettings();

  refreshFromSettings();
}

void MainWindow::on_sb_bgHUDColorAlpha_valueChanged(int arg1) {
  auto tmpColor{m_softSettings.bgHUDColor()};
  tmpColor.setAlpha(arg1);
  m_softSettings.setBgHUDColor(tmpColor, m_gearDisplay);
  saveSoftSettings();
  refreshFromSettings();
}

void MainWindow::on_pb_changeBackground_clicked() {
  const auto kImage{QFileDialog::getOpenFileName(
      this, tr("Select image"), {}, tr("Image Files %0;;All (*)").arg("(*.png *.jpg *.bmp *.jpeg)"))};
  setBackgroungImage(kImage);
}

void MainWindow::on_pb_ezConf_clicked() {
  auto deviceList{qsdl::getPluggedJoysticks()};
  Dialog_ConfigureGame::configure(this, deviceList, m_gearHandler.settings().profileName, c_appDataFolder);
  loadProfileSettings(); /* Because we might have modified it */
  refreshFromSettings();
  refreshDisplayFromGearHandlerSettings();
}

void MainWindow::on_action_checkUpdates_triggered() {
  m_updateHandler->show();
}

void MainWindow::on_actionOpen_logs_folder_triggered() {
  QFileInfo fInfo{QString::fromStdString(logHandler::GlobalLogInfo::i().progLogFilePath)};
  QDesktopServices::openUrl(QUrl::fromLocalFile(fInfo.dir().absolutePath()));
}

void MainWindow::paintEvent(QPaintEvent *pe) {
  std::ignore = pe;
  QPainter painter(this);

  auto winSize = size();
  auto pixmapRatio = (float)m_pixmapBg.width() / m_pixmapBg.height();
  auto windowRatio = (float)winSize.width() / winSize.height();

  if (pixmapRatio > windowRatio) {
    auto newWidth = (int)(winSize.height() * pixmapRatio);
    auto offset = (newWidth - winSize.width()) / -2;
    painter.drawPixmap(offset, 0, newWidth, winSize.height(), m_pixmapBg);
  } else {
    auto newHeight = (int)(winSize.width() / pixmapRatio);
    auto offset = (newHeight - winSize.height()) / -2;
    painter.drawPixmap(0, offset, winSize.width(), newHeight, m_pixmapBg);
  }
}
