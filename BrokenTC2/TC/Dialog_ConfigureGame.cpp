#include "Dialog_ConfigureGame.hpp"
#include "ui_Dialog_ConfigureGame.h"

#include <QMessageBox>
#include <QTime>

#include "ConfigEditor.hpp"

Dialog_ConfigureGame::Dialog_ConfigureGame(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_ConfigureGame)
{
    ui->setupUi(this);

    setModal(true);

    connect(ui->stackedWidget,&QStackedWidget::currentChanged,this,&Dialog_ConfigureGame::onCurrentIndexChanged);

    setWaitWidgetsVisible(false);
}

Dialog_ConfigureGame::~Dialog_ConfigureGame()
{
    delete ui;
}


bool Dialog_ConfigureGame::configure(QWidget* parent){
    Dialog_ConfigureGame dial{parent};

    auto ans{dial.exec()};

    auto success{dial.succeeded()};

    if(ans != QDialog::Accepted)
        return success;

    if(success)
    {
        QMessageBox::information(&dial,dial.windowTitle(),tr("Successfully configured The Crew 2"));
//        QMessageBox::information()
    }
    else
    {
        QMessageBox::warning(&dial,dial.windowTitle(),tr("An error occurred when trying to configure The Crew 2"));
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
    case 0:
        ui->pb_nextOk->setText(tr("Next"));
        break;
    case 1:
        ui->pb_nextOk->setText(tr("Next"));
        break;
    case 2:
        ui->pb_nextOk->setText(tr("Finish"));
        break;
    default:
        break;
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

    switch(curIndex)
    {
    case 0:
        ui->stackedWidget->setCurrentIndex(tc::isTC2Running()?1:2);
        break;
    case 1:{
        tc::killTheCrew2();
        setWaitWidgetsVisible(true);
        QStringList animText{"┏⁠(⁠＾⁠0⁠＾⁠)⁠⁠┛","ƪ⁠(⁠‾⁠.⁠‾⁠“⁠)⁠┐","ヘ⁠(⁠￣⁠ω⁠￣⁠ヘ⁠)","ƪ⁠(⁠˘⁠⌣⁠˘⁠)⁠ʃ"};
        int32_t animIndex{};
        QTime lastAnimTime{QTime::currentTime().addSecs(-10)};
        while(tc::isTC2Running())
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

        ui->stackedWidget->setCurrentIndex(2);
        break;}
    case 2:{
        auto bindingList{tc::getBindingsFiles(tc::getConfigPath())};
        for(const auto& f : bindingList)
        {
            qDebug() << __PRETTY_FUNCTION__ << " edit config file -> " << f;
            tc::xml::editXmlConf(tc::getConfigPath()+"/"+f);
        }
        this->done(QDialog::Accepted);
        break;}
    default:
        break;
    }
}


void Dialog_ConfigureGame::configure(){
    auto bindingList{tc::getBindingsFiles(tc::getConfigPath())};

    bool success{true};
    for(const auto& xmlPath : bindingList)
    {
        success = success && tc::xml::editXmlConf(xmlPath);
    }

    m_succeeded = success;
}










void Dialog_ConfigureGame::closeEvent(QCloseEvent *e){
    m_breakInfLoop = true;

    QDialog::closeEvent(e);
}
