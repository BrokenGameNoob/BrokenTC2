#include "MainWindow.hpp"
#include "./ui_MainWindow.h"

#include <QDebug>

#include <QThread>

#include "ProgFeatures.hpp"
#include "libSimpleUpdater/SimpleUpdater.hpp"

MainWindow::MainWindow(const ProgArgs &args, QWidget *parent)
    : QMainWindow(parent),
      m_args{args}
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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
        int32_t kRetryCount{3};
        int32_t kMsDelayBetweenRetry{1000};

        using RVType = InstallPackageRVal;
        const std::array<int32_t,3> reasonNotToRetry{
            RVType::kInvalidUpdateFile,
            RVType::kRefusedUnsecure,
            RVType::kUnknown
        };

        InstallPackageSettings callParameters{};
        for(int32_t i{}; i < kRetryCount; ++i)
        {
            qInfo() << "Installation try" << i;
            //note: if the user is prompted to install unsecure update and refuse
            //installation won't be retried. So we shall not show the prompt
            //anymore anyway
            if(i == 0)//first call
            {
                callParameters.forceQuiet = true;
                callParameters.forceUnsecureInstallPrompt = true;
                callParameters.forceHideUnsecureInstallPrompt = false;
            }
            else if(i == kRetryCount-1)//last call
            {
                callParameters.forceQuiet = false;
                callParameters.forceUnsecureInstallPrompt = false;
                callParameters.forceHideUnsecureInstallPrompt = true;
            }
            else//mid? call
            {
                callParameters.forceQuiet = true;
                callParameters.forceUnsecureInstallPrompt = false;
                callParameters.forceHideUnsecureInstallPrompt = true;
            }

            auto success{installPackage(m_args,this,callParameters)};

            if(success)
            {
                qInfo() << "Successfully installed the update";
                readyToQuit(0);
                return;
            }
            else if(std::find(cbegin(reasonNotToRetry),cend(reasonNotToRetry),success.reason) != cend(reasonNotToRetry))
            {
                qWarning() << "Installation failure (" << success.reason << ") lead to not retrying";
                break;
            }
            qWarning() << "Failed extraction try" << i <<", retrying after" << kMsDelayBetweenRetry << "ms";
            QThread::msleep(kMsDelayBetweenRetry);
        }
        //we should not reach this place if we extracted the update
        qCritical() << "Cannot extract installation package to given folder";
        readyToQuit(2);
        return;
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
