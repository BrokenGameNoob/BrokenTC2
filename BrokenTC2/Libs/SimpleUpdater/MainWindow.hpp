#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include "ProgArgs.hpp"

#include <atomic>

#include <QShowEvent>
#include <QMessageBox>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const ProgArgs& args,QWidget *parent = nullptr);
    ~MainWindow();

    void readyToQuit(int32_t rCode){
        m_rval = rCode;
        m_canLeave = true;
    }

public slots:
    void exit(){
        while(!m_canLeave)//should not even be used... Does not enter in the function before constructor ending
        {
//            QCoreApplication::processEvents(QEventLoop::AllEvents,100);//not a good thing
        }
        if(m_rval != 0)
        {
            if(!m_args.quiet)
                QMessageBox::critical(this,tr("Installation error"),tr("An error occured during the program.\n"
                                                                       "Please reach support for more information.\n\n"
                                                                       "More details can be found in the log file."));
        }
        qApp->exit(m_rval);
    };

protected:
    void showEvent(QShowEvent* event);

private:
    const ProgArgs m_args;
    Ui::MainWindow *ui;

    int32_t m_rval{0};
    std::atomic_bool m_canLeave{false};
};
#endif // MAINWINDOW_HPP
