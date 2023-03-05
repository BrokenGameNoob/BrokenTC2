#pragma once

#include <functional>
#include <source_location>

#include "Network.hpp"
#include "Signing.hpp"
#include "UpdtGlobal.hpp"

namespace {

static bool acquireUpdatedAlreadyCalled{false};
static std::string acquireUpdatedFirstCaller{};

}

namespace updt {

//Retrieve latest update info online
//template<typename... Args_t>
//void getLatestReleaseInfo(const QString& projectGithubRelease,std::function<void(std::optional<updt::ReleaseInfo>,Args_t...)> funcToCall,Args_t...args);

//Run a given function if the program was updated and reset the tag to notify that the update was taken into account
//template<typename... Args_t>
//bool acquireUpdated(std::function<void(Args_t...)> funcToCall,const QString& updatedTagName,Args_t...args);

//Retrieve latest update info online
inline
ShouldInstall shouldInstall(const Version& curVersion,const QString& updtPackageToCheck,const QString& manifestPath, const QString& verifierPath);

/*!
 * \brief Test whether a package should be installed considering a signature from a manifest, a public key and a version
 * \param curVersion
 * \param updtPackageToCheck
 * \param manifestPath
 * \param verifierPath
 * \return
 */
inline
ShouldInstall shouldInstall(const Version& curVersion,const QString& updtPackageToCheck,const QString& manifestPath, const QString& verifierPath){
    auto manifest_opt{updt::getManifest(manifestPath)};
    if(!manifest_opt)
    {
        qWarning() << "Could not read manifest:" << manifestPath;
        return {.code=ShouldInstall::MANIFEST_NOT_READ,.mustNotUpdate=false};
    }
    if(manifest_opt.value().minVersionRequired > curVersion)
    {
        qCritical() << "Current version ("<<curVersion<<") does not meet the requirements of the minimum version:" << manifest_opt.value().minVersionRequired;
        return {.code=ShouldInstall::MIN_VERSION_NOT_FULFILLED,.mustNotUpdate=true};
        qCritical() << "Must not update";
    }

    auto verifiedOpt{signing::checkFileSignatureFromManifest(updtPackageToCheck,manifestPath,verifierPath)};
    if(!verifiedOpt)
    {
        qWarning() << "Could not check signature of:" << updtPackageToCheck << "with manifest:" << manifestPath << "and verifier key:" << verifierPath;
        return {.code=ShouldInstall::COULD_NOT_CHECK_SIGNATURE,.mustNotUpdate=false};
    }
    if(!verifiedOpt.value())
    {
        qCritical() << "Invalid signature for:" << updtPackageToCheck << "with manifest:" << manifestPath << "and verifier key:" << verifierPath;
        qCritical() << "Must not update";
        return {.code=ShouldInstall::INVALID_SIGNATURE,.mustNotUpdate=true};
    }

    return {.code=ShouldInstall::VALID,.mustNotUpdate=false};
}



#define MACRO_GET_CALLER_LAMBDA_TEMPLATE_TYPENAME auto GetCaller = []() {return std::string{std::source_location::current().function_name()};}
/*!
 * \brief Run a given function if the program was updated and reset the tag to notify that the update was taken into account
 * \param funcToCall: function to call. Note that its signature is void(...). All arguments must be given through args
 * \param args: arguments to give to the function called if updated
 * \param updatedTagName: file that should exists if the program was updated. It should be deleted if found.
 * \return true if the program was updated, false otherwise
 */
template<typename... Args_t,MACRO_GET_CALLER_LAMBDA_TEMPLATE_TYPENAME>
bool acquireUpdated(std::function<void(Args_t...)> funcToCall,const QString& updatedTagName,Args_t...args){
    if(::acquireUpdatedAlreadyCalled)
    {
        qCritical() << __PRETTY_FUNCTION__ << ": This function must be called only once.";
        throw std::runtime_error{
            std::string{"Function "}+__PRETTY_FUNCTION__+
                    std::string{" has already be called by "+::acquireUpdatedFirstCaller+
                    ".\nIt must be called only once to prevent unknown behavior.\nLastly called by:\n"
                    +GetCaller()}};
    }
    ::acquireUpdatedFirstCaller = GetCaller();
    ::acquireUpdatedAlreadyCalled = true;

    if(!QFileInfo::exists(updatedTagName))
    {
        return false;
    }

    funcToCall(args...);

    if(!QFile::remove(updatedTagName))
    {
        qWarning() << "Failed to remove the updated tag file at:" << updatedTagName;
        qWarning() << "Even though it was found.";
    }

    return true;
}

/*!
 * \brief Retrieve latest update info online
 * \param projectGithubRelease
 * \param funcToCall
 * \param args
 */
template<typename... Args_t>
void getLatestReleaseInfoRq(const QString& projectGithubRelease,std::function<void(std::optional<updt::ReleaseInfo>,Args_t...)> funcToCall,Args_t...args){
    net::getJsonFromAPI(projectGithubRelease,[=](std::optional<QJsonDocument> optDoc){
        if(!optDoc)
        {
            qCritical() << __PRETTY_FUNCTION__ << ": Cannot retrieve latest release info from network";
            funcToCall({},args...);
        }
        else //we retrieved the Github API json containing everything
        {

            funcToCall(getLatestReleaseInfo(optDoc.value()),args...);
        }
    });
}

}//namespace updt
