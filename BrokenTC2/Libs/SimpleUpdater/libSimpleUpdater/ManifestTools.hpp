#pragma once

#include "JSONTools.hpp"
#include "Global.hpp"

#include <QStringList>

namespace updt {

struct Manifest{
    Version minVersionRequired{};
    QStringList updateFileList{};
    QString finalizeUpdateCommand{};
    QString updatedTagFile{"UPDATED.TAG"};//File to create when updating to notify that an update took place
    QString packageSignature{};//placeholder for the signature of an update package
};
inline
QDebug operator<<(QDebug stream,const Manifest& manifest){
    return stream.nospace()<< "Manifest{Version{" << manifest.minVersionRequired << "},"
                           << manifest.updateFileList << ","
                           << manifest.finalizeUpdateCommand << ","
                           << manifest.updatedTagFile << "}";
}

std::optional<Manifest> getManifest(const QJsonDocument& jDoc);
inline
std::optional<Manifest> getManifest(const QString& filePath){
    using namespace utils;
    auto docOpt{json::read(filePath)};
    if(!docOpt)
    {
        return {};
    }
    return getManifest(docOpt.value());
}

bool save(const Manifest& manifest,const QString& path);

} // namespace updt

