#pragma once

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QWidget>

#include "Constants.hpp"
#include "ProgArgs.hpp"
#include "libSimpleUpdater/SimpleUpdater.hpp"

namespace {

bool createEmptyFile(const QString& f, bool overwrite = true) {
  QFileInfo fInfo{f};
  if (fInfo.exists() && !overwrite) {
    return true;
  }

  if (!fInfo.absoluteDir().exists()) {
    fInfo.absoluteDir().mkpath(".");
  }

  QFile file{fInfo.absoluteFilePath()};
  if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
    qCritical() << "Could not create file:" << fInfo.absoluteFilePath();
    return false;
  }
  file.close();
  return true;
}

bool deleteFile(const QString& f) {
  if (f.isEmpty()) return true;
  if (QFileInfo::exists(f))
    return QFile::remove(f);
  else
    return true;
}

bool deleteUpdatedTag(const QString& path) {
  auto success{deleteFile(path)};
  if (!success) {
    qCritical() << __PRETTY_FUNCTION__ << ": Failed to remove the updated tag... Aïe aïe aïe...";
  }
  return success;
}

}  // namespace

inline std::optional<bool> signPackage(const ProgArgs& args, const QString& updatePck) {
  if (args.signerKeyFile.isEmpty())  // if empty key file, consider we did not ask for signature
  {
    return {};
  }

  QFileInfo signerKeyInf{args.signerKeyFile};
  if (!args.signerKeyFile.isEmpty() && !signerKeyInf.exists()) {
    qWarning() << "Invalid signer key file provided: does not exists:" << signerKeyInf.absoluteFilePath()
               << " results: not signing the package";
    return false;
  }
  auto signerKeyOpt{signing::loadSigner(signerKeyInf.absoluteFilePath())};
  if (!signerKeyOpt) {
    qWarning() << "Could not load private signer key from:" << signerKeyInf.absoluteFilePath();
    return false;
  }

  auto signerKey{signerKeyOpt.value()};
  auto signature{signing::signFile(updatePck, signerKey)};
  if (signature.isEmpty()) {
    qWarning() << "Failed to generate signature for package:" << updatePck;
    return false;
  }

  QString installerSignature{};
  if (!args.installerFile.isEmpty()) {
    auto tmp{signing::signFile(args.installerFile, signerKey)};
    qInfo() << "Signing installer file:" << args.installerFile;
    if (tmp.isEmpty()) {
      qWarning() << "Failed to generate signature for installer:" << args.installerFile;
      return false;
    }
    installerSignature = tmp;
  }

  auto manifest_opt{updt::getManifest(args.manifestFile)};
  if (!manifest_opt) {
    qWarning() << ": Invalid manifest file for signature:" << args.manifestFile;
    return false;
  }
  auto manifest{manifest_opt.value()};
  manifest.packageSignature = signature;
  if (!installerSignature.isEmpty()) manifest.installerSignature = installerSignature;
  QFileInfo pckInfo{updatePck};

  QString manifestName{
      QString{"%0/%1Manifest.json"}.arg(pckInfo.absoluteDir().absolutePath(), pckInfo.completeBaseName())};
  if (!updt::save(manifest, manifestName)) {
    qWarning() << "Could not save new manifest as:" << manifestName;
    return false;
  } else {
    qInfo() << "Saved manifest containing signature as:" << manifestName;
  }

  return true;
}

inline auto createPackage(const ProgArgs& args) {
  auto manifest_opt{updt::getManifest(args.manifestFile)};
  if (!manifest_opt) {
    qCritical() << __PRETTY_FUNCTION__ << ": Invalid manifest file:" << args.manifestFile;
    return false;
  }
  const auto& manifest{manifest_opt.value()};
  QFileInfo fInfo{args.manifestFile};
  auto manifestDir{fInfo.absoluteDir()};
  qDebug() << "manifest in:" << manifestDir;

  updt::fs::Compressor compressor{INSTALLER_PACKAGE_EXTENSION};

  auto targetList{manifest.updateFileList};
  if (!args.manifestReferenceFolder.isEmpty()) {
    for (auto& f : targetList) {
      QFileInfo fInfo{f};
      if (fInfo.isAbsolute()) {
        continue;
      }
      f = QString{"%0/%1"}.arg(args.manifestReferenceFolder, f);
    }
  }

  compressor.paths() = targetList;
  auto baseFolder{(args.outputFolder.isEmpty()) ? (manifestDir.absolutePath())
                                                : (QDir{args.outputFolder}.absolutePath())};
  auto outFile{QString{"%0/%1%2"}.arg(baseFolder, UPDATE_PACKAGE_FILENAME, INSTALLER_PACKAGE_EXTENSION)};
  auto trueOutput{compressor.compress(outFile, fInfo.absoluteDir().absolutePath())};
  if (trueOutput.isEmpty()) {
    qCritical() << "Failed to compress & save update package";
    return false;
  }

  qInfo() << "Created update package:" << trueOutput;

  auto signPackageResult{signPackage(args, trueOutput)};
  if (signPackageResult)  // the package was asked to be signed
  {
    if (signPackageResult.value())  // signature succeeded
    {
      qInfo() << "Successfully signed the update package";
    } else  // signature failed
    {
      qWarning() << "Could not sign the update package";
    }
  }

  return true;
}

struct InstallPackageSettings {
  bool forceQuiet{false};
  bool forceUnsecureInstallPrompt{false};
  bool forceHideUnsecureInstallPrompt{false};
};
struct InstallPackageRVal {
  enum FailureReason : int32_t {
    kSucceeded,
    kUnknown,
    kRefusedUnsecure,
    kInvalidUpdateFile,
    kSignatureError,
    kVerificationError,
    kInvalidManifest,
    kFailedUpdateTag,
    kExtractionFailure,
    kPostUpdateFailure
  };
  FailureReason reason{};

  explicit operator bool() const {
    return reason == kSucceeded;
  }
};

inline InstallPackageRVal installPackage(const ProgArgs& args, QWidget* parent,
                                         const InstallPackageSettings& installSettings) {
  auto lambda_isQuiet{[&]() { return args.quiet || installSettings.forceQuiet; }};

  qInfo() << "Installation parameters:";
  qInfo().noquote() << "\tforceQuiet=" << installSettings.forceQuiet;
  qInfo().noquote() << "\tforceUnsecureInstallPrompt=" << installSettings.forceUnsecureInstallPrompt;
  qInfo().noquote() << "\tforceHideUnsecureInstallPrompt=" << installSettings.forceHideUnsecureInstallPrompt;

  QFileInfo fInfo{args.inputUpdateArchive};
  if (!fInfo.exists() || !fInfo.isFile()) {
    qCritical() << __PRETTY_FUNCTION__
                << ": Invalid update package (not found or not a file):" << args.inputUpdateArchive;
    return {InstallPackageRVal::kInvalidUpdateFile};
  }

  if (args.verifierKeyFile.isEmpty()) {
    if ((lambda_isQuiet() && !installSettings.forceUnsecureInstallPrompt) ||
        installSettings.forceHideUnsecureInstallPrompt) {
      qWarning() << "Installing an update without signature verification (unsecure)";
    } else {
      auto ans =
          QMessageBox::warning(parent,
                               QObject::tr("Warning!"),
                               QObject::tr("You are about to install an update without verifying its origin.\n"
                                           "It is highly UNRECOMMENDED to proceed as it may have malicious content.\n\n"
                                           "Are you still willing to continue?"),
                               QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
      if (ans == QMessageBox::StandardButton::No) {
        qInfo() << "Installation aborted because refused to install package without signature verification";
        return {InstallPackageRVal::kRefusedUnsecure};
      }
      qWarning() << "Installing an update without signature verification (unsecure)";
    }
  } else  // we should check signature
  {
    auto safeToInstallOpt{
        signing::checkFileSignatureFromManifest(fInfo.absoluteFilePath(), args.manifestFile, args.verifierKeyFile)};
    if (!safeToInstallOpt) {
      if (!lambda_isQuiet()) {
        QMessageBox::critical(parent,
                              QObject::tr("Error!"),
                              QObject::tr("The update was expected to be signed but an invalid manifest file (without "
                                          "signature or could not read it) was given.\n"
                                          "As it may be a security issue, the installation was cancelled"));
      }
      qCritical() << "The update was expected to be signed but no signature was found in the provided manifest file "
                     "(or unreachable manifest). ABORTING installation";
      return {InstallPackageRVal::kSignatureError};
    }
    const auto safeToInstall{safeToInstallOpt.value()};
    if (!safeToInstall) {
      if (!lambda_isQuiet()) {
        QMessageBox::critical(parent,
                              QObject::tr("Error during installation"),
                              QObject::tr("Failed to verify the update package.\n"
                                          "As it is a serious security concern, the installation was cancelled\n\n"
                                          "Please try to reinstall the software or reach support"));
      }
      qCritical() << "Failed to verify the update package.";
      return {InstallPackageRVal::kVerificationError};
    } else {
      qInfo() << "Successfully verified the update package! Should be safe to install";
    }
  }

  QString updatedTagFilePath{};
  if (!args.manifestFile.isEmpty()) {
    auto manifestOpt{updt::getManifest(args.manifestFile)};
    if (!manifestOpt) {
      qCritical() << "Invalid manifest file";
      return {InstallPackageRVal::kInvalidManifest};
    }
    const auto& manifest{manifestOpt.value()};
    updatedTagFilePath = manifest.updatedTagFile;
  }

  if (!updatedTagFilePath.isEmpty()) {
    if (!createEmptyFile(updatedTagFilePath)) {
      qCritical() << __PRETTY_FUNCTION__ << "Failed to create the update tag";
      return {InstallPackageRVal::kFailedUpdateTag};
    }
  } else {
    qWarning() << "Installing an update without updated tag file";
  }

  auto updtPckDir{fInfo.absoluteDir()};
  auto baseFolder{(args.outputFolder.isEmpty()) ? updtPckDir.absolutePath() : QDir{args.outputFolder}.absolutePath()};

  updt::fs::Compressor c{};
  QProgressDialog extractionProgress{QObject::tr("Installation progress"), "", 0, 100};
  extractionProgress.setCancelButton(nullptr);
  extractionProgress.connect(&c, &updt::fs::Compressor::progress, &extractionProgress, [&](int progress) {
    extractionProgress.setValue(progress);
  });
  extractionProgress.show();

  auto success{c.uncompress(fInfo.absoluteFilePath(), baseFolder)};
  if (!success) {
    qCritical() << __PRETTY_FUNCTION__ << ": Failed to extract update package:" << fInfo.absoluteFilePath()
                << " to:" << baseFolder;
    deleteUpdatedTag(updatedTagFilePath);
    return {InstallPackageRVal::kExtractionFailure};
  }
  qInfo() << "Successfully extracted update package:" << fInfo.absoluteFilePath() << " to:" << baseFolder;

  if (!args.postUpdateCmd.isEmpty()) {
    qInfo() << "Running post update command:";
    qInfo().nospace() << "\t" << args.postUpdateCmd;

    qint64 pid{};
    // -i
    auto success = QProcess::startDetached(args.postUpdateCmd, {}, QApplication::applicationDirPath(), &pid);
    if (pid == 0 || !success) {
      qCritical() << "Could not start the post update command (" << __PRETTY_FUNCTION__ << ")";
      qCritical() << args.postUpdateCmd;
      deleteUpdatedTag(updatedTagFilePath);
      return {InstallPackageRVal::kPostUpdateFailure};
    }
    qInfo() << "Started post update with PID:" << pid;
  } else {
    qWarning() << __PRETTY_FUNCTION__ << ": No post-update command specified... Doesn't seem normal";
  }

  return {InstallPackageRVal::kSucceeded};
}
