#include "ManifestTools.hpp"

#include <QJsonArray>

namespace updt {

std::optional<Manifest> getManifest(const QJsonDocument& jDoc) {
  Manifest rVal{};

  QJsonObject globObj{jDoc.object()};

  rVal.minVersionRequired = versionFromJson(globObj.value("minVersionRequired").toObject());
  rVal.updateFileList = utils::json::strListFromJsonArray(globObj.value("updateFileList").toArray());
  rVal.finalizeUpdateCommand = globObj.value("finalizeUpdateCommand").toString(rVal.finalizeUpdateCommand);
  rVal.updatedTagFile = globObj.value("updatedTagFile").toString(rVal.updatedTagFile);
  rVal.packageSignature = globObj.value("packageSignature").toString(rVal.packageSignature);
  rVal.installerSignature = globObj.value("installerSignature").toString(rVal.installerSignature);

  return rVal;
}

bool save(const Manifest& manifest, const QString& path) {
  QJsonObject globObj{};

  globObj.insert("minVersionRequired", toJson(manifest.minVersionRequired));
  globObj.insert("updateFileList", QJsonArray::fromStringList(manifest.updateFileList));
  globObj.insert("finalizeUpdateCommand", manifest.finalizeUpdateCommand);
  globObj.insert("updatedTagFile", manifest.updatedTagFile);
  globObj.insert("packageSignature", manifest.packageSignature);
  globObj.insert("installerSignature", manifest.installerSignature);

  return utils::json::save(globObj, path);
}

}  // namespace updt
