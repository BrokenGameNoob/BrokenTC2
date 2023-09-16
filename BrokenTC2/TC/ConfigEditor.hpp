#ifndef TC_XML_CONFIGEDITOR_HPP
#define TC_XML_CONFIGEDITOR_HPP

#include <QString>
#include <QDomDocument>
#include <QDomNode>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QStringList>

#include <functional>

#include "TC/gameProfile.hpp"

#ifdef Q_OS_WIN
#include "Windows/WinUtils.hpp"
#endif

namespace{
static const auto TC2_PROCESS_NAME{"TheCrew2.exe"};
}

namespace tc {

inline
    QString getConfigPath(const tc::GameInfo& gameInfo = tc::GetGameInfo(GameId::THE_CREW2)){
    return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/"+gameInfo.kFolderInDocuments;
}

inline
bool isGameRunning(const tc::GameInfo& gameInfo = tc::GetGameInfo(GameId::THE_CREW2)){
    return win::isProcessRunning(gameInfo.kProcessName);
}

inline
bool killGame(const tc::GameInfo& gameInfo = tc::GetGameInfo(GameId::THE_CREW2)){
    auto pId{win::findProcessId(gameInfo.kProcessName)};
    if(!pId)
    {
        return true;
    }

    return win::terminateProcess(pId.value());
}

QStringList getBindingsFiles(const QString& configDirPath);

namespace xml {

/*!
 * \brief Edit a TC2 xml config file to remove GearUp and GearDown key
 * \param xmlPath : file to edit
 * \return success
 */
bool editXmlControllerConf(const QString& filePath);

} // namespace xml
} // namespace tc

#endif // TC_XML_CONFIGEDITOR_HPP
