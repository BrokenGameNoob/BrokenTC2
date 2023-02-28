#pragma once

#include <functional>

#include "Signing.hpp"
#include "Global.hpp"

namespace updt {

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

/*!
 * \brief Run a given function if the program was updated and reset the tag to notify that the update was taken into account
 * \param funcToCall: function to call. Note that its signature is void(...). All arguments must be given through args
 * \param args: arguments to give to the function called if updated
 * \param updatedTagName: file that should exists if the program was updated. It should be deleted if found.
 * \return true if the program was updated, false otherwise
 */
template<typename... Args_t>
bool acquireUpdated(std::function<void(Args_t...)> funcToCall,const QString& updatedTagName,Args_t...args){
    if(!QFileInfo::exists(updatedTagName))
    {
        return false;
    }

    funcToCall(args...);

    return true;
}

}//namespace updt
