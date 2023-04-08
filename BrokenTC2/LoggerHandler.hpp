#pragma once

#include <QtMessageHandler>
#include <QDebug>
#include <QDateTime>
#include <QLoggingCategory>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#ifdef Q_OS_WIN
#include <debugapi.h>
#endif

#include <iostream>

namespace logHandler
{

struct GlobalLogInfo{
    std::string progLogFilePath{};
    std::string progName{};

    static auto& i(){
        static GlobalLogInfo instance{};
        return instance;
    }
};

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
#if defined(Q_OS_WIN) && defined(CMAKE_DEBUG_MODE)
    int size_s = std::snprintf( nullptr, 0, format, args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ return; }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format, args ... );
    std::string message( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    std::wstring widestr = std::wstring(message.begin(), message.end());

    OutputDebugString(reinterpret_cast<const wchar_t *>(widestr.c_str()));
#endif
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
    static LoggerFile f{GlobalLogInfo::i().progLogFilePath};
    if(firstCall){
        firstCall = false;
        progLog(stdout,f,"\n\n  [Info]  : Starting log session - %s - %s\n\n",date.constData(),time.constData());
    }

    switch (type) {
    case QtDebugMsg:
        progLog(stdout,f, "%s   [Debug] : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
//        progLog(stdout,f, "%s   [Info]  : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        progLog(stdout,f, "%s   [Info]  : %s\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        progLog(stdout,f, "%s [Warning] : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        progLog(stderr,f, "%s  [Error]  : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        progLog(stderr,f, "%s   [Fatal] : %s \t\t(%s:%u, %s)\n", time.constData(), localMsg.constData(), file, context.line, function);
        break;
    }
}

}//namespace logHandler


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
void installCustomLogHandler(logHandler::GlobalLogInfo logInfo){
#ifdef LOG_HANDLER_BASE_DIR_QSTRING
    QFileInfo originalInfo{QString::fromStdString(logInfo.progLogFilePath)};

    if(!originalInfo.isAbsolute())
    {
        logInfo.progLogFilePath = (LOG_HANDLER_BASE_DIR_QSTRING).toStdString()+"/Logs/"+logInfo.progLogFilePath;
    }
#else
#endif

#ifdef CMAKE_DEBUG_MODE
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, true);
#else
    QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, false);
#endif

    QFileInfo fInfo{QString::fromStdString(logInfo.progLogFilePath)};
    auto logDir{fInfo.absoluteDir()};
    if(!logDir.exists())
    {
        logDir.mkpath(".");
    }

    logHandler::GlobalLogInfo::i() = std::move(logInfo);

    qInstallMessageHandler(logHandler::customMessageHandler);
}
