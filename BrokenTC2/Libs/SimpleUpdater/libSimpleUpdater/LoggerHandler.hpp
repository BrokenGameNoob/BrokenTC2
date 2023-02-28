#pragma once

#include <QtMessageHandler>
#include <QDebug>
#include <QDateTime>
#include <QLoggingCategory>

#include <iostream>

namespace
{

std::string progLogFileName{};

class LoggerFile{
public:
    LoggerFile(std::string name):
        m_name{std::move(name)},
        m_file{fopen(m_name.c_str(),"a")}
    {
        if(!m_file)
        {
            const auto date{QDateTime::currentDateTime().toString("dd-MM-yyyy").toLocal8Bit()};
            const auto time{QDateTime::currentDateTime().toString("hh:mm:ss").toLocal8Bit()};
            fprintf(stderr,"%s -> %s : Can't open log file <%s> -> Should abort program\n",date.constData(),time.constData(),m_name.c_str());
            throw std::runtime_error{"Can't open app log file -> abort"};
        }
    }
    ~LoggerFile(){
        std::cout << "Closing log file\n";
        fclose(m_file);
    }

    auto* file(){
        return m_file;
    }

private:
    std::string m_name{};
    FILE* m_file{nullptr};
};

template<typename... Args>
void progLog(FILE* stdstream,LoggerFile& file,const char * format, Args... args) {
  fprintf(stdstream,format,args...);
  fprintf(file.file(),format,args...);
}

}


inline
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    const auto date{QDateTime::currentDateTime().toString("dd-MM-yyyy").toLocal8Bit()};
    const auto time{QDateTime::currentDateTime().toString("hh:mm:ss").toLocal8Bit()};

    static bool firstCall{true};
    static LoggerFile f{::progLogFileName};
    if(firstCall){
        firstCall = false;
        progLog(stdout,f,"\n\n  [Info]  : Starting log session - %s - %s\n\n",date.constData(),time.constData());
    }

    switch (type) {
    case QtDebugMsg:
        ::progLog(stdout,f, "%s   [Debug] : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
//        ::progLog(stdout,f, "%s   [Info]  : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        ::progLog(stdout,f, "%s   [Info]  : %s\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        ::progLog(stdout,f, "%s [Warning] : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        ::progLog(stderr,f, "%s  [Error]  : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        ::progLog(stderr,f, "%s   [Fatal] : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    default:
        ::progLog(stderr,f, "%s [Unknown] : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    }
}


/*!
 * \brief installCustomLogHandler
 *
 * To make this function work properly, the following should be added to the cmake file:
 * \code{.cmake}
 * IF(CMAKE_BUILD_TYPE MATCHES Debug)
        add_compile_definitions(CMAKE_DEBUG_MODE)
 * ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)
 * \endcode
 * If you want to always enable debug messages, you can just keep the line:
 *  QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
 */
inline
void installCustomLogHandler(std::string logFile){
    ::progLogFileName = std::move(logFile);
#ifdef CMAKE_DEBUG_MODE
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
#else
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, false);
#endif
    qInstallMessageHandler(customMessageHandler);
}
