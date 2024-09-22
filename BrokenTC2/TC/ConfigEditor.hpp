#ifndef TC_XML_CONFIGEDITOR_HPP
#define TC_XML_CONFIGEDITOR_HPP

#include <QDir>
#include <QDomDocument>
#include <QDomNode>
#include <QFile>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <functional>

#include "TC/Profile.hpp"
#include "TC/gameProfile.hpp"
#include "global.hpp"

#ifdef Q_OS_WIN
#include "Windows/WinUtils.hpp"
#endif

namespace {
static const auto TC2_PROCESS_NAME{"TheCrew2.exe"};
}

namespace tc {

inline QString getConfigPath(const tc::GameInfo& gameInfo = tc::GetGameInfo(GameId::THE_CREW2)) {
  return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + gameInfo.kFolderInDocuments;
}

inline bool isGameRunning(const tc::GameInfo& gameInfo = tc::GetGameInfo(GameId::THE_CREW2)) {
  return win::isProcessRunning(gameInfo.kProcessName);
}

inline bool killGame(const tc::GameInfo& gameInfo = tc::GetGameInfo(GameId::THE_CREW2)) {
  auto pId{win::findProcessId(gameInfo.kProcessName)};
  if (!pId) {
    return true;
  }

  return win::terminateProcess(pId.value());
}

QStringList getBindingsFiles(const QString& configDirPath);
QString getBindingKeyboardFile(const QString& configDirPath);

namespace xml {

/*!
 * \brief Edit a TC2 xml config file to remove GearUp and GearDown key
 * \param xmlPath : file to edit
 * \return success
 */
bool editXmlControllerConf(const QString& xmlPath);

bool editXmlKeyboardConf(const QString& xmlPath, tc::ProfileSettings* settings = nullptr);

}  // namespace xml
}  // namespace tc

#endif  // TC_XML_CONFIGEDITOR_HPP
