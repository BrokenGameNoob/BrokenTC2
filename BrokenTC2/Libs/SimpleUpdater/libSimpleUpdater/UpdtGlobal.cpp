#include "UpdtGlobal.hpp"

#include <QJsonDocument>
#include <QJsonArray>

namespace updt {

bool operator>(const Version& r,const Version& d)
{
    if(r.major > d.major)
        return true;
    else if(r.major < d.major)
        return false;

    if(r.minor > d.minor)
        return true;
    else if(r.minor < d.minor)
        return false;


    if(r.patch > d.patch)
        return true;
    return false;
}

bool operator>=(const Version& r,const Version& d)
{
    if(r.major == d.major && r.minor == d.minor && r.patch == d.patch)
    {
        return true;
    }
    return r > d;
}


std::optional<ReleaseInfo> getLatestReleaseInfo(QJsonDocument doc){

    auto version{getLatestReleaseVersion(doc)};
    if(!version)
        return {};
    auto assetsURLs{getLatestReleaseAssetsURLs(doc)};

    return {{version.value(),assetsURLs}};
}

QStringList getLatestReleaseAssetsURLs(const QJsonDocument& doc){
    auto jsonObj{doc.object()};
    auto assets(jsonObj["assets"].toArray());

    QStringList rVal{};
    rVal.reserve(assets.size());
    for(const auto& e : assets)
    {
        auto dlUrl{e.toObject()["browser_download_url"].toString()};
        rVal.append(std::move(dlUrl));
    }
    return rVal;
}

std::optional<Version> getLatestReleaseVersion(const QJsonDocument& doc){
    auto jsonObj{doc.object()};
    QString releaseTagName{jsonObj.value("tag_name").toString()};

    static const QRegularExpression tagNameRegex{"^v([0-9]{1,}).([0-9]{1,}).([0-9]{1,})$"};
    //one group per version number (major, minor, patch)

    const auto match{tagNameRegex.match(releaseTagName)};
    //check for format : <vX.X.X>
    if(!match.hasMatch())
    {
        qCritical() << __PRETTY_FUNCTION__ << ": Cannot find proper <tag_name> in given JsonDocument";
        return {};
    }

    Version out{};
    out.major = match.captured(1).toInt();
    out.minor = match.captured(2).toInt();
    out.patch = match.captured(3).toInt();
    return out;
}

} // namespace updt
