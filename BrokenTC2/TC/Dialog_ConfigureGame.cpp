#include "Dialog_ConfigureGame.hpp"
#include "global.hpp"
#include "ui_Dialog_ConfigureGame.h"

#include <QMessageBox>
#include <QTime>
#include <QDebug>

#include "ConfigEditor.hpp"

Dialog_ConfigureGame::Dialog_ConfigureGame(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_ConfigureGame)
{
    ui->setupUi(this);

    setModal(true);

    connect(ui->stackedWidget,&QStackedWidget::currentChanged,this,&Dialog_ConfigureGame::onCurrentIndexChanged);

    setWaitWidgetsVisible(false);

    ui->pb_nextOk->setEnabled(false);
}

Dialog_ConfigureGame::~Dialog_ConfigureGame()
{
    delete ui;
}


bool Dialog_ConfigureGame::configure(QWidget* parent){
    Dialog_ConfigureGame dial{parent};

    auto ans{dial.exec()};

    auto success{dial.succeeded()};

    const auto& kGameInfos{tc::GetGameInfo(dial.getSelectedGameId())};

    if(ans != QDialog::Accepted)
        return success;

    if(success)
    {
        QMessageBox::information(&dial,dial.windowTitle(),tr("Successfully configured %0").arg(kGameInfos.kGameName));
//        QMessageBox::information()
    }
    else
    {
        QMessageBox::warning(&dial,dial.windowTitle(),tr("An error occurred when trying to configure %0").arg(kGameInfos.kGameName));
    }

    return success;//cancelling is a success because it's an expected behavior
}

void Dialog_ConfigureGame::setWaitWidgetsVisible(bool visible){
    ui->lbl_disp_waitTC2Closed->setVisible(visible);
    ui->lbl_waitTC2Closed->setVisible(visible);
}
void Dialog_ConfigureGame::onCurrentIndexChanged(int curIndex){
    switch(curIndex)
    {
    case kIntro:
    case kNoConfigFileFound:
    case kWaitingForTC2Closed:
        ui->pb_nextOk->setText(tr("Next"));
        break;
    case 3:
        ui->pb_nextOk->setText(tr("Finish"));
        break;
    default:
        break;
    }
    if(curIndex != 0){
        ui->comboBox->setEnabled(false);
    } else {
        ui->comboBox->setEnabled(true);
    }
    setWaitWidgetsVisible(false);
}


void Dialog_ConfigureGame::on_pb_cancel_clicked()
{
    m_breakInfLoop = true;
    this->done(QDialog::Rejected);
}


void Dialog_ConfigureGame::on_pb_nextOk_clicked()
{
    auto curIndex{ui->stackedWidget->currentIndex()};

    const auto& kGameInfos{tc::GetGameInfo(getSelectedGameId())};
    const auto kConfigPath{tc::getConfigPath(kGameInfos)};
    auto configFileList{tc::getBindingsFiles(kConfigPath)};

    qInfo() << "Setting up game: ";
    qInfo() << "\t" << getSelectedGameId();
    qInfo() << "\t" << kGameInfos.kProcessName;
    qInfo() << "\t" << kGameInfos.kFolderInDocuments;
    qInfo() << "\t" << kConfigPath;

    switch(curIndex)
    {
    case kIntro:{
        if(configFileList.empty())
        {
            ui->stackedWidget->setCurrentIndex(kNoConfigFileFound);
        }
        else
        {
            ui->stackedWidget->setCurrentIndex(tc::isGameRunning(kGameInfos)?kWaitingForTC2Closed:kDone);
        }
        break;
    }case kNoConfigFileFound:{
        if(configFileList.empty())
        {
            QMessageBox::warning(this,tr("Attention please"),tr("Please follow instruction on screen. No configuration file was found."));
        }
        else
        {
            ui->stackedWidget->setCurrentIndex(tc::isGameRunning(kGameInfos)?kWaitingForTC2Closed:kDone);
        }
        break;
    } case kWaitingForTC2Closed:{
        tc::killGame(kGameInfos);
        setWaitWidgetsVisible(true);
        QStringList animText{"┏⁠(⁠＾⁠0⁠＾⁠)⁠⁠┛","ƪ⁠(⁠‾⁠.⁠‾⁠“⁠)⁠┐","ヘ⁠(⁠￣⁠ω⁠￣⁠ヘ⁠)","ƪ⁠(⁠˘⁠⌣⁠˘⁠)⁠ʃ"};
        int32_t animIndex{};
        QTime lastAnimTime{QTime::currentTime().addSecs(-10)};
        while(tc::isGameRunning(kGameInfos))
        {
            auto curTime{QTime::currentTime()};
            if(lastAnimTime.msecsTo(curTime) > 500)
            {
                ui->lbl_waitTC2Closed->setText(animText[animIndex++]);
                animIndex %= animText.size();
                lastAnimTime = curTime;
            }
            qApp->processEvents();
            if(m_breakInfLoop)
                break;
        }

        ui->stackedWidget->setCurrentIndex(3);
        break;}
    case kDone:{
        auto bindingList{tc::getBindingsFiles(kConfigPath)};
        for(const auto& f : bindingList)
        {
            qDebug() << __PRETTY_FUNCTION__ << " edit config file -> " << f;
            tc::xml::editXmlControllerConf(kConfigPath+"/"+f);
        }
        this->done(QDialog::Accepted);
        break;}
    default:
        break;
    }
}


void Dialog_ConfigureGame::configure(){
    const auto& kGameInfos{tc::GetGameInfo(getSelectedGameId())};
    auto bindingList{tc::getBindingsFiles(tc::getConfigPath(kGameInfos))};

    bool success{true};
    for(const auto& xmlPath : bindingList)
    {
        success = success && tc::xml::editXmlControllerConf(xmlPath);
    }

    m_succeeded = success;
}




void Dialog_ConfigureGame::closeEvent(QCloseEvent *e){
    m_breakInfLoop = true;

    QDialog::closeEvent(e);
}

tc::GameId Dialog_ConfigureGame::getSelectedGameId() const {
    return static_cast<tc::GameId>(ui->comboBox->currentIndex()-1);
}

void Dialog_ConfigureGame::on_comboBox_currentIndexChanged(int index)
{
//    auto labelList{retrieveChildren<QLabel>(this)};
//    for(QLabel* lbl: labelList){
//        lbl->setText(lbl->text().arg(ui->comboBox->currentText()));
//    }

    if(index == 0){
        ui->pb_nextOk->setEnabled(false);
    } else {
        ui->pb_nextOk->setEnabled(true);
    }
}

