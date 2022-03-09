#ifndef DEBUG_LOGCONSOLE_H
#define DEBUG_LOGCONSOLE_H

#include <QTextBrowser>
#include <QObject>
#include <QScrollBar>

#include <QVector>

#include <QDebug>

namespace debug {

class LogConsole : public QTextBrowser
{
Q_OBJECT

public:
    using InstanceListType = QVector<LogConsole*>;

    struct Option{
        bool printTime = false;
    };

public:
    LogConsole(QWidget* parent);
    ~LogConsole();

    void setOptions(Option opt) {
        m_options = opt;
    }
    Option options() const {
        return m_options;
    }

    /*!
     * \brief saveLogs allows to save log browser content in a specified file
     *
     * \note To allow to open the file at the end of saving, the path have to be absolute
     *
     * \param fileName : File name/path without extension
     * \param saveAsHtml : Save logs as html including color formatting
     * \param openAfterSave : Open logs using default system soft
     * \return
     */
    bool saveLogs(QString fileName,bool saveAsHtml,bool openAfterSave);


    void setMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static void getMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    LogConsole& operator<< (QString const &str);

private:
    static InstanceListType m_instancesList;
    Option m_options;
};

} // namespace debug

#endif // DEBUG_LOGCONSOLE_H
