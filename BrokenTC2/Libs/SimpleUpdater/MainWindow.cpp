#include "MainWindow.hpp"
#include "./ui_MainWindow.h"

#include <QDebug>

#include "ProgFeatures.hpp"
#include "libSimpleUpdater/SimpleUpdater.hpp"

MainWindow::MainWindow(const ProgArgs &args, QWidget *parent)
    : QMainWindow(parent),
      m_args{args}
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    updt::getLatestReleaseInfo([&](std::optional<updt::ReleaseInfo> opt){
//            if(opt)
//            {
//                qDebug() << opt.value().versionAvailable << "  " << opt.value().assetsURLs;
////                updt::net::downloadFileList(this,/*{"https://releases.ubuntu.com/22.04/ubuntu-22.04.1-desktop-amd64.iso"}*/opt.value().assetsURLs,"UpdateDlTest/",true,[&](bool ok){
////                    qDebug() << "Multiple DL succeeded?" << ok;
////                });
//                opt.value().assetsURLs.append("Truc");
//                opt.value().assetsURLs.append("https://releases.ubuntu.com/22.04/ubuntu-22.04.1-desktop-amd64.iso");
//                auto dlManager{new updt::net::DownloadManager(opt.value().assetsURLs,"UpdateDlTest/",true,this)};
//                connect(dlManager,&updt::net::DownloadManager::allDlCompleted,this,[&](int32_t successCount,const QVector<updt::net::FailedDownload>& fails){
//                    qDebug() << "Success: " << successCount << "   Fails:";
//                    for(const auto& e : fails)
//                    {
//                        qDebug().nospace() << "Failed to dl " << e.url << " because of " << e.errMsg;
//                    }
//                });
//                dlManager->startAllDownloads();
//            }
//            else
//            {
//                qDebug() << "Can't retrieve release versions";
//            }
//        },updt::getGithubReleaseApiUrl("BrokenGameNoob","BrokenTC2"));

    if(m_args.progGoal == ProgArgs::CREATE_PACKAGE)
    {
        bool success{createPackage(m_args)};
        if(!success)
        {
            qCritical() << "Cannot create installation package from given manifest";
            readyToQuit(1);
            return;
        }
        qInfo() << "Successfully created the update package";
    }
    else if(m_args.progGoal == ProgArgs::INSTALL_UPDATE)
    {
        auto success{installPackage(m_args,this)};
        if(!success)
        {
            qCritical() << "Cannot extract installation package to given folder";
            readyToQuit(2);
            return;
        }
        qInfo() << "Successfully installed the update";
    }

    readyToQuit(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent* event)//when the window is shown
{
    QMainWindow::showEvent(event);
}
