#pragma once

#include "ProgArgs.hpp"

#include "libSimpleUpdater/SimpleUpdater.hpp"
#include "Constants.hpp"

#include <QFileInfo>
#include <QDir>

#include <QMessageBox>
#include <QWidget>

#include <QDebug>

inline
std::optional<bool> signPackage(const ProgArgs& args,const QString& updatePck){
    if(args.signerKeyFile.isEmpty())//if empty key file, consider we did not ask for signature
    {
        return {};
    }

    QFileInfo signerKeyInf{args.signerKeyFile};
    if(!args.signerKeyFile.isEmpty() && !signerKeyInf.exists())
    {
        qWarning() << "Invalid signer key file provided: does not exists:" << signerKeyInf.absoluteFilePath()<< " results: not signing the package";
        return false;
    }
    auto signerKeyOpt{signing::loadSigner(signerKeyInf.absoluteFilePath())};
    if(!signerKeyOpt)
    {
        qWarning() << "Could not load private signer key from:" << signerKeyInf.absoluteFilePath();
        return false;
    }

    auto signerKey{signerKeyOpt.value()};
    auto signature{signing::signFile(updatePck,signerKey)};
    if(signature.isEmpty())
    {
        qWarning() << "Failed to generate signature for package:" << updatePck;
        return false;
    }

    auto manifest_opt{updt::getManifest(args.manifestFile)};
    if(!manifest_opt)
    {
        qWarning() << ": Invalid manifest file for signature:" << args.manifestFile;
        return false;
    }
    auto manifest{manifest_opt.value()};
    manifest.packageSignature = signature;
    QFileInfo pckInfo{updatePck};

    QString manifestName{QString{"%0/%1Manifest.json"}.arg(
                    pckInfo.absoluteDir().absolutePath(),
                    pckInfo.completeBaseName()
    )};
    if(!updt::save(manifest,manifestName))
    {
        qWarning() << "Could not save new manifest as:" << manifestName;
        return false;
    }
    else
    {
        qInfo() << "Saved manifest containing signature as:" << manifestName;
    }

    return true;
}

inline
auto createPackage(const ProgArgs& args){
    auto manifest_opt{updt::getManifest(args.manifestFile)};
    if(!manifest_opt)
    {
        qCritical() << __PRETTY_FUNCTION__ << ": Invalid manifest file:" << args.manifestFile;
        return false;
    }
    const auto& manifest{manifest_opt.value()};
    QFileInfo fInfo{args.manifestFile};
    auto manifestDir{fInfo.absoluteDir()};
    qDebug() << "manifest in:" << manifestDir;

    updt::fs::Compressor compressor{INSTALLER_PACKAGE_EXTENSION};
    compressor.paths() = manifest.updateFileList;
    auto baseFolder{(args.outputFolder.isEmpty())?(manifestDir.absolutePath()):(QDir{args.outputFolder}.absolutePath())};
    auto outFile{QString{"%0/%1%2"}.arg(baseFolder,UPDATE_PACKAGE_FILENAME,INSTALLER_PACKAGE_EXTENSION)};
    auto trueOutput{compressor.compress(outFile,fInfo.absoluteDir().absolutePath())};
    if(trueOutput.isEmpty())
    {
        qCritical() << "Failed to compress & save update package";
        return false;
    }

    qInfo() << "Created update package:" << trueOutput;

    auto signPackageResult{signPackage(args,trueOutput)};
    if(signPackageResult)//the package was asked to be signed
    {
        if(signPackageResult.value())//signature succeeded
        {
            qInfo() << "Successfully signed the update package";
        }
        else//signature failed
        {
            qWarning() << "Could not sign the update package";
        }
    }

    return true;
}


inline
auto installPackage(const ProgArgs& args,QWidget* parent){
    QFileInfo fInfo{args.inputUpdateArchive};
    if(!fInfo.exists() || !fInfo.isFile())
    {
        qCritical() << __PRETTY_FUNCTION__ << ": Invalid update package (not found or not a file):" << args.inputUpdateArchive;
        return false;
    }

    if(args.verifierKeyFile.isEmpty())
    {
        if(args.quiet)
        {
            qWarning() << "Installing an update without signature verification (unsecure)";
        }
        else
        {
            auto ans=QMessageBox::warning(parent,QObject::tr("Warning!"),
                                 QObject::tr("You are about to install an update without verifying its origin.\n"
                                             "It is highly UNRECOMMENDED to proceed as it may have malicious content.\n\n"
                                             "Are you still willing to continue?"),
                                 QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
            if(ans == QMessageBox::StandardButton::No)
            {
                qInfo() << "Installation aborted because refused to install package without signature verification";
                return false;
            }
            qWarning() << "Installing an update without signature verification (unsecure)";
        }
    }
    else//we should check signature
    {
        auto safeToInstallOpt{signing::checkFileSignatureFromManifest(fInfo.absoluteFilePath(),args.manifestFile,args.verifierKeyFile)};
        if(!safeToInstallOpt)
        {
            if(!args.quiet)
            {
                QMessageBox::critical(parent,QObject::tr("Error!"),
                          QObject::tr("The update was expected to be signed but an invalid manifest file (without signature or could not read it) was given.\n"
                                      "As it may be a security issue, the installation was cancelled"));
            }
            qCritical() << "The update was expected to be signed but no signature was found in the provided manifest file (or unreachable manifest). ABORTING installation";
            return false;
        }
        const auto safeToInstall{safeToInstallOpt.value()};
        if(!safeToInstall)
        {
            if(!args.quiet)
            {
                QMessageBox::critical(parent,QObject::tr("Error during installation"),
                                      QObject::tr("Failed to verify the update package.\n"
                                                  "As it is a serious security ceoncern, the installation was cancelled\n\n"
                                                  "Please try to reinstall the software or reach support"));
            }
            qCritical() << "Failed to verify the update package.";
            return false;
        }
        else
        {
            qInfo() << "Successfully verified the update package! Should be safe to install";
        }
    }


    auto updtPckDir{fInfo.absoluteDir()};
    auto baseFolder{(args.outputFolder.isEmpty())?updtPckDir.absolutePath():QDir{args.outputFolder}.absolutePath()};

    updt::fs::Compressor c{};
    auto success{c.uncompress(fInfo.absoluteFilePath(),baseFolder)};
    if(!success)
    {
        qCritical() << __PRETTY_FUNCTION__ << ": Failed to extract update package:" << fInfo.absoluteFilePath() << " to:" << baseFolder;
        return false;
    }
    qInfo() << "Successfully extracted update package:" << fInfo.absoluteFilePath() << " to:" << baseFolder;

    return true;
}
