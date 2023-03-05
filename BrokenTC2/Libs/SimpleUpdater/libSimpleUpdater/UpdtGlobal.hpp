#pragma once

#include <QDebug>

#include <QJsonObject>

#ifndef PROJECT_V_MAJOR
#define PROJECT_V_MAJOR 0
#endif
#ifndef PROJECT_V_MINOR
#define PROJECT_V_MINOR 0
#endif
#ifndef PROJECT_V_PATCH
#define PROJECT_V_PATCH 0
#endif

namespace updt {

struct Version
{
    int32_t major{PROJECT_V_MAJOR};
    int32_t minor{PROJECT_V_MINOR};
    int32_t patch{PROJECT_V_PATCH};
};

struct ReleaseInfo
{
    Version versionAvailable{};
    QStringList assetsURLs{};
};

struct ShouldInstall{
    enum Code{
        VALID,
        MANIFEST_NOT_READ,
        MIN_VERSION_NOT_FULFILLED,
        COULD_NOT_CHECK_SIGNATURE,
        INVALID_SIGNATURE,
    };
    Code code;
    bool mustNotUpdate{false};
};


template<typename Str_t0,typename Str_t1>
QString getGithubReleaseApiUrl(const Str_t0& userSlug, const Str_t1& projectSlug){
    return QString{"https://api.github.com/repos/%0/%1/releases/latest"}.arg(userSlug,projectSlug);
}


//inline //ask online and get it in callback. Get a "ReleaseInfo"
//void getLatestReleaseInfo(std::function<void(std::optional<ReleaseInfo>)> callback,
//                          const QString& projectGithubRelease);
std::optional<ReleaseInfo> getLatestReleaseInfo(QJsonDocument doc);

QStringList getLatestReleaseAssetsURLs(const QJsonDocument& doc);

std::optional<Version> getLatestReleaseVersion(const QJsonDocument& doc);





bool operator>(const Version& v0,const Version& v1);
bool operator>=(const Version& v0,const Version& v1);
inline
QDebug operator<<(QDebug dbg,const Version& v){
    dbg.nospace() << v.major << "." << v.minor << "." << v.patch;
    return dbg;
}
inline
QString to_string(const Version& v){
    return QString{"%0.%1.%2"}.arg(v.major).arg(v.minor).arg(v.patch);
}

inline
QJsonObject toJson(const Version& v){
    QJsonObject rVal{};
    rVal.insert("major",v.major);
    rVal.insert("minor",v.minor);
    rVal.insert("patch",v.patch);
    return rVal;
}
inline
Version versionFromJson(const QJsonObject& jObj){
    Version rVal{0,0,0};
    rVal.major = jObj.value("major").toInt();
    rVal.minor = jObj.value("minor").toInt();
    rVal.patch = jObj.value("patch").toInt();
    return rVal;
}

} // namespace updt
