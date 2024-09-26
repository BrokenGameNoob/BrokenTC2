#pragma once

#include <QStringList>

#include "UpdtGlobal.hpp"
#include "UpdtJSONTools.hpp"

namespace updt {

struct Manifest {
  Version minVersionRequired{};
  QStringList updateFileList{};
  QString finalizeUpdateCommand{};
  QString updatedTagFile{"UPDATED.TAG"};  // File to create when updating to notify that an update took place
  QString packageSignature{};             // placeholder for the signature of an update package
  QString installerSignature{};
};
inline QDebug operator<<(QDebug stream, const Manifest& manifest) {
  return stream.nospace() << "Manifest{Version{" << manifest.minVersionRequired << "}," << manifest.updateFileList
                          << "," << manifest.finalizeUpdateCommand << "," << manifest.updatedTagFile << "}";
}

// Retrieve manifest from a dedicated json file
std::optional<Manifest> getManifest(const QJsonDocument& jDoc);
inline std::optional<Manifest> getManifest(const QString& filePath) {
  auto docOpt{utils::json::read(filePath)};
  if (!docOpt) {
    return {};
  }
  return getManifest(docOpt.value());
}

bool save(const Manifest& manifest, const QString& path);

}  // namespace updt
