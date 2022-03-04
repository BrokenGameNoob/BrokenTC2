#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "global.hpp"

#include "TC/Profile.hpp"
#include "TC/GearHandler.hpp"
#include "TC/Widget_gearDisplay.hpp"

#include "QSDL/GameController.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

struct Settings{
    bool isInit{false};

    bool displayAboutOnStartup{true};
    QString currentDeviceName;
    bool gearDisplayed;
};

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onControllerPluggedIn(int id);
    void onControllerUnplugged(int id);

    void onControllerButtonPressed(int id);



    void on_pb_selectKey_resetDefault_clicked();

    void on_cb_selectDevice_currentIndexChanged(int index);

    void on_cb_showCurrentGear_stateChanged(int checked);

    void on_action_about_triggered();

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
