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
#include "./ui_mainwindow.h"

#include "global.hpp"
#include "Update/PostUpdate.hpp"
#include <QTimer>

#include "Utils/Dialog_getKeyCode.hpp"
#include "Utils/Dialog_getGameControllerButton.hpp"
#include "Utils/Dialog_About.hpp"
#include "Utils/GUITools.hpp"

#include "Windows/WinEventHandler.hpp"

#include "TC/Profile.hpp"
#include "TC/GearHandler.hpp"

#include "QSDL/SDLEventHandler.hpp"
#include "QSDL/SDLGlobal.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "Utils/JSONTools.hpp"

#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QDesktopServices>
#include <QScreen>
#include <QWindow>
#include <QCloseEvent>

#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>

#include <QDebug>
#include <QProcess>

#include <windows.h>


namespace{

void setKey(int keyCode,QLabel* lblDisp,tc::ProfileSettings::Key& settingsKeyToChange)
{
    if(keyCode == 0)//means do nothing
    {
        return;
    }
    else if(keyCode < 0)//means unbind key
    {
        lblDisp->setText("-");
    }
    else
    {
        lblDisp->setText(QString::number(keyCode));
    }
    settingsKeyToChange = keyCode;//if key == -1, it will never be matched = Unbind
}

inline
void setButton(int button,QLabel* lblDisp,tc::ProfileSettings::Key& settingsBtnToChange)
{
    if(button < -1)//means do nothing
    {
        return;
    }
    else if(button == -1)//means unbind key
    {
        lblDisp->setText("-");
    }
    else
    {
        lblDisp->setText(QString::number(button));
    }
    settingsBtnToChange = button;//if button == -1, it will never be matched = Unbind
}

inline
QString screenName(const QScreen* screen)
{
    static QRegularExpression reToRemove{"(\\.)|(/)|(\\\\)"};
    return screen->name().remove(reToRemove);
}

}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      m_trayIcon{QIcon{":/img/img/softPic.png"},this},
      m_updateManager{false,this},
      m_wasUpdated{updt::postUpdateFunction()},
      c_appDataFolder{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/"},
      m_softSettings{},
      c_softSettingsFile{c_appDataFolder+"settings.conf"},
      m_gearDisplay{new Widget_gearDisplay()},
      m_controller{}
{
    // UI Init
    ui->setupUi(this);
    ui->statusbar->addPermanentWidget(new QLabel{PROJECT_VERSION,this});

    //Correct line appearances
    auto lineList{this->findChildren<QFrame*>(QRegularExpression{"^line_[0-9]*$"})};
    for(auto& e : lineList)
    {
        e->setMidLineWidth(1);
    }

    //tray icon
    auto trayIconMenu{new QMenu(this)};
    auto tmpAction = trayIconMenu->addAction(tr("Show window"));
    connect(tmpAction,&QAction::triggered,this,[&](){
        this->show();
    });
    tmpAction = trayIconMenu->addAction(tr("Exit program"));
    connect(tmpAction,&QAction::triggered,this,[&](){
        this->close();
    });
    m_trayIcon.setContextMenu(trayIconMenu);
    m_trayIcon.show();
    connect(&m_trayIcon,&QSystemTrayIcon::activated,this,[&](auto reason){
            if(reason == QSystemTrayIcon::ActivationReason::Trigger)
            {
                this->show();
            }});



    qDebug() << c_appDataFolder;
    qDebug() << c_softSettingsFile;

    qDebug() << "UPDATED ? " << m_wasUpdated;

    ui->dockConsole->setVisible(false);
    connect(ui->action_ShowDevconsole,&QAction::triggered,this,[&](bool){ui->dockConsole->setVisible(true);});

    connect(ui->toolB_help,&QToolButton::clicked,this,[&](){
        constexpr auto helpLink{"https://github.com/BrokenGameNoob/BrokenTC2"};
        qDebug() << __CURRENT_PLACE__ << " : open " << helpLink;
        if(!QDesktopServices::openUrl(QUrl{helpLink}))
        {
            QMessageBox::warning(this,tr("Can't help"),tr("Sorry, we can't open help. Go to this page :\n%0").
                                 arg(helpLink));
        }
    });

    //start keyboard event handler
    connect(windows::WindowsEventThread::ins(),&windows::WindowsEventThread::keyDown,this,&MainWindow::onKeyboardPressed);

    qsdl::SDLEventHandler::start();
    qsdl::SDLEventHandler::registerController(&m_controller);


    connect(ui->pb_selectKey_GReverse,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_GReverse,m_gearHandler.settings().reverse);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_GClutch,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_GClutch,m_gearHandler.settings().clutch);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_G1,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_G1,m_gearHandler.settings().g1);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_G2,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_G2,m_gearHandler.settings().g2);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_G3,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_G3,m_gearHandler.settings().g3);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_G4,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_G4,m_gearHandler.settings().g4);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_G5,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_G5,m_gearHandler.settings().g5);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_G6,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_G6,m_gearHandler.settings().g6);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_G7,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_G7,m_gearHandler.settings().g7);
        saveProfileSettings();
    });

    connect(ui->pb_selectKey_SeqUp,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_seqUp,m_gearHandler.settings().seqGearUp);
        saveProfileSettings();
    });
    connect(ui->pb_selectKey_SeqDown,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_seqDown,m_gearHandler.settings().seqGearDown);
        saveProfileSettings();
    });


    connect(ui->pb_selectKey_kSwitchMode,&QPushButton::clicked,this,[&](){
        auto key{Dialog_getKeyCode::getKey(this).code};
        setKey(key,ui->lbl_kSwitchMode,m_gearHandler.settings().kSwitchMode);
        saveProfileSettings();
    });
    connect(ui->sb_gearDelay,&QSpinBox::valueChanged,this,[&](int val){
        m_gearHandler.settings().keyDownTime = val;
        saveProfileSettings();
    });



    connect(ui->pb_selectButton_GUp,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_GUp,m_gearHandler.settings().gearUp);
        saveProfileSettings();
    });
    connect(ui->pb_selectButton_GDown,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_GDown,m_gearHandler.settings().gearDown);
        saveProfileSettings();
    });


    connect(ui->pb_selectButton_gearR,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_gearR,m_gearHandler.settings().setReverseGear);
        saveProfileSettings();
    });
    connect(ui->pb_selectButton_gear1,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_gear1,m_gearHandler.settings().setFirstGear);
        saveProfileSettings();
    });
    connect(ui->pb_selectButton_gear2,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_gear2,m_gearHandler.settings().setSecondGear);
        saveProfileSettings();
    });
    connect(ui->pb_selectButton_gear3,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_gear3,m_gearHandler.settings().setThirdGear);
        saveProfileSettings();
    });
    connect(ui->pb_selectButton_gear4,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_gear4,m_gearHandler.settings().setFourthGear);
        saveProfileSettings();
    });
    connect(ui->pb_selectButton_gear5,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_gear5,m_gearHandler.settings().setFifthGear);
        saveProfileSettings();
    });
    connect(ui->pb_selectButton_gear6,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_gear6,m_gearHandler.settings().setSixthGear);
        saveProfileSettings();
    });


    connect(ui->pb_selectButton_switchMode,&QPushButton::clicked,this,[&](){
        auto btn{Dialog_getGameControllerButton::getButton(&m_controller,this)};
        setButton(btn,ui->lbl_btn_switchMode,m_gearHandler.settings().switchMode);
        saveProfileSettings();
    });



    connect(&m_gearHandler,&tc::GearHandler::gearChanged,m_gearDisplay,&Widget_gearDisplay::refreshGear);
    connect(&m_gearHandler,&tc::GearHandler::gearSwitchModeChanged,m_gearDisplay,&Widget_gearDisplay::onSwitchGearModeChanged);
    connect(&m_gearHandler,&tc::GearHandler::gearSwitchModeChanged,this,[&](auto){saveProfileSettings();});

    using qsdl::SDLEventHandler;
    using qsdl::GameController;
    connect(SDLEventHandler::instance(),&SDLEventHandler::gameControllerAdded,this,&MainWindow::onControllerPluggedIn);
    connect(SDLEventHandler::instance(),&SDLEventHandler::gameControllerRemoved,this,&MainWindow::onControllerUnplugged);

    connect(&m_controller,&GameController::buttonDown,this,&MainWindow::onControllerButtonPressed);

    //--------------------------------------------------------------------

    qDebug() << __CURRENT_PLACE__ << "  " << getCurrentProfileFilePath();
    if(!QFileInfo::exists(c_appDataFolder))//if the appdata folder doesn't exist
    {
        if(!QDir::root().mkpath(c_appDataFolder))
        {
            throw std::runtime_error{__CURRENT_PLACE__.toStdString() + " : Cannot create appData folder : <" + c_appDataFolder.toStdString() + ">"};
        }
    }

    populateDevicesComboBox();




    auto availableScreens{QApplication::screens()};
    int i{};
    for(const auto& e : availableScreens)
    {
        ui->cb_gearDisplayScreen->addItem(screenName(e),i);
        ++i;
    }
    connect(ui->cb_gearDisplayScreen,&QComboBox::currentIndexChanged,this,[&](int){
        m_softSettings.displayGearScreen = ui->cb_gearDisplayScreen->currentText();
        this->on_cb_showCurrentGear_stateChanged(ui->cb_showCurrentGear->isChecked());
    });




    loadSoftSettings();

    //try to read settings profile
    if(!QFileInfo::exists(getCurrentProfileFilePath()))
    {
        qDebug() << "Saving initial configuration";
        saveProfileSettings();
    }
    else
    {
        loadProfileSettings();
    }


    refreshDisplayFromGearHandlerSettings();

    m_softSettings.isInit = true;



    //UPDATES
    connect(ui->action_checkUpdates,&QAction::triggered,&m_updateManager,&updt::UpdateManager::checkUpdate);

    connect(&m_updateManager,&updt::UpdateManager::hiddenUpdateAvailable,this,[&](){
        auto ans = QMessageBox::question(this,tr("Update available"),tr("A new update is available. Do you want to download it ?"));
        if(ans == QMessageBox::Yes)
        {
            m_updateManager.exec();
        }
    });
    connect(&m_updateManager,&updt::UpdateManager::hiddenNoUpdateAvailable,this,[&](){
        ui->statusbar->showMessage(tr("No available update found"),10000);
    });
    m_updateManager.checkUpdate();

//    Dialog_getGameControllerButton::getButton(&m_controller,this);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_gearDisplay;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if(event->spontaneous())//if the window is closed by the top right closing icon
    {
        this->hide();
        m_trayIcon.show();
        event->setAccepted(false);
    }
    else//closed programmatically (or expected to be so?)
    {
        m_gearDisplay->close();
        QMainWindow::closeEvent(event);
    }
}

void MainWindow::showEvent(QShowEvent* event)//when the window is shown
{
    QMainWindow::showEvent(event);

    if(m_softSettings.displayAboutOnStartup)
    {/*
        on_action_about_triggered();*/
        m_softSettings.displayAboutOnStartup = false;
        saveSoftSettings();

        QMetaObject::invokeMethod(this, "on_action_about_triggered", Qt::ConnectionType::QueuedConnection);//call it after function terminated
    }
    if(m_wasUpdated)
    {
        QMetaObject::invokeMethod(this,[this](){
            updt::showChangelog(this);
        },Qt::ConnectionType::QueuedConnection);
    }

    m_trayIcon.hide();
}

//------------------------------------------------------------------
//
//--------   Soft settings
//
//------------------------------------------------------------------


void MainWindow::updateSoftSettings()
{
    m_softSettings.currentDeviceName = ((ui->cb_selectDevice->currentText().isEmpty())?m_softSettings.currentDeviceName
                                                                                     :ui->cb_selectDevice->currentText());
    m_softSettings.gearDisplayed = ui->cb_showCurrentGear->isChecked();

    saveSoftSettings();
}

bool MainWindow::saveSoftSettings()
{
    if(!m_softSettings.isInit)//if settings still not initialized
        return true;//they won't be saved

    QJsonObject globObj{};

    QJsonObject settings{};
    settings.insert("displayAboutOnStartup",m_softSettings.displayAboutOnStartup);
    settings.insert("lastProfile",m_softSettings.currentDeviceName);
    settings.insert("displayGear",m_softSettings.gearDisplayed);
    settings.insert("displayGearScreen",m_softSettings.displayGearScreen);
    settings.insert("lowPerfMode",m_softSettings.lowPerfMode());

    globObj.insert("settings",settings);

    return utils::json::save(globObj,c_softSettingsFile);
}

bool MainWindow::loadSoftSettings()
{
    auto docOpt{utils::json::read(c_softSettingsFile)};

    if(!docOpt)//if we could not read the settings file
    {
        if(QFileInfo(c_softSettingsFile).exists())//if the config file exist
            throw std::runtime_error{__CURRENT_PLACE__.toStdString()+" : "+
                                     std::string{"Cannot read settings file "}+c_softSettingsFile.toStdString()};
        if(!saveSoftSettings())//if we fail to create a settings file
        {
            throw std::runtime_error{__CURRENT_PLACE__.toStdString()+" : "+
                                     std::string{"Cannot create settings file "}+c_softSettingsFile.toStdString()};
        }
        else//if we saved new settings
        {
            return true;
        }
    }

    auto docObj{docOpt.value().object()};

    Settings out{};

    auto settings{docObj.value("settings").toObject()};
    out.displayAboutOnStartup = settings.value("displayAboutOnStartup").toBool(true);
    out.currentDeviceName = settings.value("lastProfile").toString();
    out.gearDisplayed = settings.value("displayGear").toBool();
    out.setLowPerfMode(settings.value("lowPerfMode").toBool());
    out.displayGearScreen = settings.value("displayGearScreen").toString();

    m_softSettings = out;

    refreshFromSettings();

    return true;
}

//------------------------------------------------------------------
//
//--------   TC profiles settings save/load
//
//------------------------------------------------------------------


void MainWindow::saveProfileSettings()
{
    if(m_gearHandler.settings().profileName.isEmpty())
        return;

    auto fileName{getCurrentProfileFilePath()};

    if(!tc::saveSettings(m_gearHandler.settings(),fileName))
    {
        qCritical() << "Could not save gearHandler settings as " << fileName;
        QMessageBox::critical(this,tr("Error"),
                              tr("Cannot save gear keys and settings to the file\n%0")
                              .arg(fileName));
    }
}


bool MainWindow::loadProfileSettings()
{
    try
    {
        m_gearHandler.settings() = tc::readProfileSettings(getCurrentProfileFilePath());
        m_gearHandler.setGearSwitchMode(m_gearHandler.settings().gearSwitchMode);
    }
    catch (const std::runtime_error& e)
    {
        qCritical() << "Cannot read gearHandler settings file";
        QMessageBox::critical(this,tr("Error"),
                              tr("Cannot read gear keys and settings from the file\n%0")
                              .arg(getCurrentProfileFilePath()));
        return false;
    }
    return true;
}

bool MainWindow::loadProfile(QString gamePadName)
{
    auto settingsSave{m_gearHandler.settings()};

    m_gearHandler.settings().profileName = gamePadName;

    if(!loadProfileSettings())
    {
        m_gearHandler.settings() = settingsSave;//restore old profile name
        return false;
    }
    return true;
}

//------------------------------------------------------------------
//
//--------   Display
//
//------------------------------------------------------------------

void MainWindow::refreshFromSettings()
{
    ui->cb_showCurrentGear->setChecked(m_softSettings.gearDisplayed);
    ui->cb_lowPerfMode->setCurrentIndex(int(m_softSettings.lowPerfMode()));

    auto deviceIndex{ui->cb_selectDevice->findText(m_softSettings.currentDeviceName)};
    if(deviceIndex >= 0)
    {
        ui->cb_selectDevice->setCurrentIndex(deviceIndex);
    }

    auto screenIndex{ui->cb_gearDisplayScreen->findText(m_softSettings.displayGearScreen)};
    if(screenIndex >= 0)
    {
        ui->cb_gearDisplayScreen->setCurrentIndex(screenIndex);
    }
    else
    {
        ui->cb_gearDisplayScreen->setCurrentIndex(-1);
    }
}

void MainWindow::populateDevicesComboBox()
{
    auto curDevice{ui->cb_selectDevice->currentText()};
    if(curDevice.isEmpty())
        curDevice = m_softSettings.currentDeviceName;

    ui->cb_selectDevice->clear();

    auto deviceList{qsdl::getPluggedJoysticks()};

//    deviceList.append("Ceci est un test !");
//    deviceList.append("Et en voici un autre");

    auto newDeviceIndex{-1};
    int i{};
    for(const auto& e : deviceList)
    {
        if(curDevice == e)
            newDeviceIndex = i;
        ui->cb_selectDevice->addItem(e,i);//store device id as data. Even though it should match cb index
        ++i;
    }
    ui->cb_selectDevice->setCurrentIndex(newDeviceIndex);
}

void MainWindow::refreshDisplayFromGearHandlerSettings()
{
    auto lambdaUpdateText{
        [&](QLabel* lbl,auto newCode){
            if(newCode < 0)
                lbl->setText("-");
            else
                lbl->setText(QString::number(newCode));
        }
    };

    lambdaUpdateText(ui->lbl_GReverse,m_gearHandler.settings().reverse);
    lambdaUpdateText(ui->lbl_GClutch,m_gearHandler.settings().clutch);
    lambdaUpdateText(ui->lbl_G1,m_gearHandler.settings().g1);
    lambdaUpdateText(ui->lbl_G2,m_gearHandler.settings().g2);
    lambdaUpdateText(ui->lbl_G3,m_gearHandler.settings().g3);
    lambdaUpdateText(ui->lbl_G4,m_gearHandler.settings().g4);
    lambdaUpdateText(ui->lbl_G5,m_gearHandler.settings().g5);
    lambdaUpdateText(ui->lbl_G6,m_gearHandler.settings().g6);
    lambdaUpdateText(ui->lbl_G7,m_gearHandler.settings().g7);
    lambdaUpdateText(ui->lbl_seqUp,m_gearHandler.settings().seqGearUp);
    lambdaUpdateText(ui->lbl_seqDown,m_gearHandler.settings().seqGearDown);

    lambdaUpdateText(ui->lbl_kSwitchMode,m_gearHandler.settings().kSwitchMode);
    ui->sb_gearDelay->setValue(m_gearHandler.settings().keyDownTime);

    lambdaUpdateText(ui->lbl_btn_GUp,m_gearHandler.settings().gearUp);
    lambdaUpdateText(ui->lbl_btn_GDown,m_gearHandler.settings().gearDown);

    lambdaUpdateText(ui->lbl_btn_gearR,m_gearHandler.settings().setReverseGear);
    lambdaUpdateText(ui->lbl_btn_gear1,m_gearHandler.settings().setFirstGear);
    lambdaUpdateText(ui->lbl_btn_gear2,m_gearHandler.settings().setSecondGear);
    lambdaUpdateText(ui->lbl_btn_gear3,m_gearHandler.settings().setThirdGear);
    lambdaUpdateText(ui->lbl_btn_gear4,m_gearHandler.settings().setFourthGear);
    lambdaUpdateText(ui->lbl_btn_gear5,m_gearHandler.settings().setFifthGear);
    lambdaUpdateText(ui->lbl_btn_gear6,m_gearHandler.settings().setSixthGear);

    lambdaUpdateText(ui->lbl_btn_switchMode,m_gearHandler.settings().switchMode);
}

//------------------------------------------------------------------
//
//--------   Qt slots
//
//------------------------------------------------------------------


void MainWindow::onControllerPluggedIn(int id)
{
    populateDevicesComboBox();
}
void MainWindow::onControllerUnplugged(int id)
{
    populateDevicesComboBox();
}

void MainWindow::onControllerButtonPressed(int button)
{
    static const QString background_pressed{"background-color:rgb(0,200,0)"};
    static const QString background_released{"background-color:rgb(190,0,0)"};

    auto lambdaForceGear{[&](tc::Gear g){
            auto switchBack{false};
            if(m_gearHandler.mode() == tc::GearSwitchMode::SEQUENTIAL){
                m_gearHandler.switchGearSwitchMode();
                switchBack = true;
            }
            m_gearHandler.setGear(g);
            if(switchBack)
                m_gearHandler.switchGearSwitchMode();
        }};

    auto setStyleSheet{
        [&](auto* lbl,const auto& style)
        {
            lbl->setStyleSheet(style);
        }
    };

    constexpr int displayDelay{150};

    if(button == m_gearHandler.settings().gearUp)
    {
        setStyleSheet(ui->lbl_pad_LB,background_pressed);
        m_gearHandler.gearUp();
        QTimer::singleShot(displayDelay,this,[&](){setStyleSheet(ui->lbl_pad_LB,background_released);});
    }
    else if(button == m_gearHandler.settings().gearDown)
    {
        setStyleSheet(ui->lbl_pad_RB,background_pressed);
        m_gearHandler.gearDown();
        QTimer::singleShot(displayDelay,this,[&](){setStyleSheet(ui->lbl_pad_RB,background_released);});
    }
    else if(button == m_gearHandler.settings().setReverseGear)
    {
        lambdaForceGear(tc::Gear::R);
    }
    else if(button == m_gearHandler.settings().setFirstGear)
    {
        lambdaForceGear(tc::Gear::G1);
    }
    else if(button == m_gearHandler.settings().setSecondGear)
    {
        lambdaForceGear(tc::Gear::G2);
    }
    else if(button == m_gearHandler.settings().setThirdGear)
    {
        lambdaForceGear(tc::Gear::G3);
    }
    else if(button == m_gearHandler.settings().setFourthGear)
    {
        lambdaForceGear(tc::Gear::G4);
    }
    else if(button == m_gearHandler.settings().setFifthGear)
    {
        lambdaForceGear(tc::Gear::G5);
    }
    else if(button == m_gearHandler.settings().setSixthGear)
    {
        lambdaForceGear(tc::Gear::G6);
    }
    else if(button == m_gearHandler.settings().switchMode)
    {
        m_gearHandler.switchGearSwitchMode();
    }
}

void MainWindow::onKeyboardPressed(int key)
{
    if(key == m_gearHandler.settings().kSwitchMode)
    {
        m_gearHandler.switchGearSwitchMode();
    }
}

void MainWindow::on_pb_selectKey_resetDefault_clicked()
{
    auto ans{QMessageBox::question(this,tr("Confirmation"),tr("This will erase your current settings and they won't be recoverable.\nDo you want to continue ?"))};
    if(ans != QMessageBox::Yes)
    {
        return;
    }

    auto oldProfileName{m_gearHandler.settings().profileName};
    m_gearHandler.settings() = {.profileName=oldProfileName};
    refreshDisplayFromGearHandlerSettings();
    saveProfileSettings();
}


void MainWindow::on_cb_selectDevice_currentIndexChanged(int index)
{
    if(!m_softSettings.isInit)
        return;

    if(index == -1 && qsdl::getPluggedJoysticksCount() == 0)//no device selected and no available device connected
    {
//        m_gearHandler.settings().profileName = {};//reset settings
        m_controller.disconnectController();//disconnect controller
    }
    else if(index == -1 && ui->cb_selectDevice->count() > 0)
    {
        ui->cb_selectDevice->setCurrentIndex(0);
    }
    else
    {
        auto deviceName{ui->cb_selectDevice->currentText()};

        if(QFileInfo(getProfileFilePath(deviceName)).exists())//If the profile file exists
        {
            if(!loadProfile(deviceName))//and we can't load the profile corresponding to the device
            {
                throw std::runtime_error{__CURRENT_PLACE__.toStdString() + " : Cannot load profile for <"+getProfileFilePath(deviceName).toStdString()+">"};
                return;
            }
        }
        else
        {
            //if the file doesn't exist, create the profile and save it
            m_gearHandler.settings() = {.profileName = deviceName};
        }
        updateSoftSettings();

        m_controller.connectController(ui->cb_selectDevice->currentData().toInt());
    }

    refreshDisplayFromGearHandlerSettings();
}


void MainWindow::on_cb_showCurrentGear_stateChanged(int checked)
{
    updateSoftSettings();

    if(checked)
    {
        auto screenList{QApplication::screens()};
        auto screenId{ui->cb_gearDisplayScreen->currentData().toInt()};
        if(screenId >= 0  && screenId < screenList.size())
        {
            auto screen{screenList[screenId]};

            m_gearDisplay->setScreen(screen);
            m_gearDisplay->setGeometry(screen->geometry());
        }
        else
        {
            m_gearDisplay->showFullScreen();
        }
        if(!m_gearDisplay->isVisible())
            m_gearDisplay->show();
    }
    else
    {
        m_gearDisplay->hide();
    }
}


void MainWindow::on_action_about_triggered()
{
    Dialog_About dial(this);
    dial.exec();
}


void MainWindow::on_cb_lowPerfMode_currentIndexChanged(int arg1)
{
    m_softSettings.setLowPerfMode(bool(arg1));
    saveSoftSettings();
}

