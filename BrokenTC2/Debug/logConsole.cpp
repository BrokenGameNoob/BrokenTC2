#include "Debug/logConsole.hpp"

#include <QTextBrowser>
#include <QtGlobal>
#include <QMap>
#include <QTime>

#include <QDesktopServices>
#include <QDir>
#include <QFile>

#include <iostream>

namespace debug {

LogConsole::InstanceListType LogConsole::m_instancesList{};//static member init

LogConsole::LogConsole(QWidget* parent) : QTextBrowser(parent),m_options{}
{
    static bool handlerInstalled{false};
    if(!handlerInstalled)
    {
        qInstallMessageHandler(&LogConsole::getMessage);
    }

    m_instancesList.append(this);
}

LogConsole::~LogConsole()
{
    m_instancesList.remove(m_instancesList.indexOf(this));
}

void LogConsole::setMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    std::cerr << qPrintable(qFormatLogMessage(type, context, msg)) <<"\n";//standard output

    static const QMap<QtMsgType,QString> colorMap{{QtMsgType::QtDebugMsg,"rgb(80,80,200)"},
                                                  {QtMsgType::QtInfoMsg,"rgb(118, 0, 191)"},
                                                  {QtMsgType::QtWarningMsg,"rgb(191, 172, 0)"},
                                                  {QtMsgType::QtCriticalMsg,"rgb(180,0,0)"},
                                                  {QtMsgType::QtFatalMsg,"rgb(255,0,0)"}};

    static const QMap<QtMsgType,QString> prefixMap{{QtMsgType::QtDebugMsg,"Debug"},
                                                {QtMsgType::QtInfoMsg,"Info"},
                                                {QtMsgType::QtWarningMsg,"Warning"},
                                                {QtMsgType::QtCriticalMsg,"Critical"},
                                                {QtMsgType::QtFatalMsg,"Fatal"}};

    auto lambdaColor{
        //ex color : rgb(X,X,X) | #FF00FF | rgba(X,X,X,X)
        [&](QString color,QString text,QString additionalCss = "") -> QString {
            return QString{"<font style=\"color:%1;%2\">%3</font>"}.arg(color,additionalCss,text);
        }
    };

    QString prefix{""};
    if(m_options.printTime)
        prefix += QString{"%0 : "}.arg(QTime::currentTime().toString("HH:mm:ss"));
    prefix += prefixMap[type];
    QString output{QString{"%0 : %1"}.arg(lambdaColor(colorMap[type],prefix,"font-weight:bold;"),msg.toHtmlEscaped())};
    *this << output;
}

void LogConsole::getMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    for(auto& e : m_instancesList)
    {
        e->setMessage(type,context,msg);
    }
}

bool LogConsole::saveLogs(QString fileName,bool saveAsHtml,bool openAfterSave)
{
    auto filePath{QString{"%0.%1"}.arg(fileName,(saveAsHtml)?"html":"txt")};
    QFile f{filePath};

    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical() << "Cannot open "<<f.fileName();
        return false;
    }

    QTextStream stream{&f};
    stream << ((saveAsHtml)?this->toHtml():this->toPlainText());

    f.close();

    if(openAfterSave)
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    return true;
}

LogConsole& LogConsole::operator<< (QString const &str)
{
    const bool atBottom = this->verticalScrollBar()->value() == this->verticalScrollBar()->maximum();
    QTextDocument* doc = this->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::End);
    cursor.beginEditBlock();
    cursor.insertBlock();
    cursor.insertHtml(str);
    cursor.endEditBlock();

    //scroll scrollarea to bottom if it was at bottom when we started
    //(we don't want to force scrolling to bottom if user is looking at a
    //higher position)
    if (atBottom) {
        QScrollBar* bar = this->verticalScrollBar();
        bar->setValue(bar->maximum());
    }

    return *this;
}

} // namespace debug
