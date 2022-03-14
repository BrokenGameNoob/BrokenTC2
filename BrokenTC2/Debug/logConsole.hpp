/* Broken The Crew 2 sequential clutch assist
 * Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
