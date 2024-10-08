#include "UpdateHandler.hpp"

#include <QCoreApplication>
#include <QFileDialog>
#include <QPixmap>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#include "ui_UpdateHandler.h"

namespace updt {

const QString UpdateHandler::InfoBoxMsgType::kUnknownTitleText{QCoreApplication::tr("Information")};
const QString UpdateHandler::InfoBoxMsgType::kWarningTitleText{QCoreApplication::tr("Warning")};
const QString UpdateHandler::InfoBoxMsgType::kOkTitleText{QCoreApplication::tr("Information")};
const QString UpdateHandler::InfoBoxMsgType::kCriticalTitleText{QCoreApplication::tr("Error")};

UpdateHandler::UpdateHandler(Version progRunningVersion, QString githubReleaseApiAddress, QString publicVerifierKeyFile,
                             bool searchAvailableOnCreation, QString postUpdateCmd,
                             bool showInstallPropositionOnNextOccasion, QWidget* parent)
    : QWidget(),
      ui(new Ui::UpdateHandler),
      m_kRunningVersion{std::move(progRunningVersion)},
      m_kGithubReleaseApiAddress{std::move(githubReleaseApiAddress)},
      m_kPblicVerifierKeyFile{std::move(publicVerifierKeyFile)},
      m_kPostUpdateCmd{std::move(postUpdateCmd)},
      m_showInstallPropositionOnNextOccasion{showInstallPropositionOnNextOccasion} {
  ui->setupUi(this);

  hideInfoMessage();

  ui->lbl_runningVersion->setText(to_string(m_kRunningVersion));

  if (searchAvailableOnCreation) {
    checkAvailableOnline(false);
  }

  if (parent) {
    connect(parent, &QObject::destroyed, this, [this]() { delete this; });
  }

  ui->lbl_disp_infoPic->setPixmap(QPixmap{":/img/img/warning.png"});
  ui->lbl_disp_infoPic->hide();

  connect(ui->cb_allowManualInstallation, &QCheckBox::clicked, this, [&](bool checked) {
    ui->pb_manualInstall->setVisible(checked);
  });
  ui->pb_manualInstall->hide();

  m_baseInfoMsg = ui->lbl_disp_infoBox->text();
}

UpdateHandler::~UpdateHandler() {
  delete ui;
}

void UpdateHandler::showInfoMessage(InfoBoxMsgType::Type type, const QString& message, int32_t timeout) {
  auto getColorStrA{[&](const QColor& in) { return in.name(QColor::HexArgb); }};
  auto getColorStr{[&](const QColor& in) { return in.name(QColor::HexRgb); }};

  auto newHash{QString::number(type) + message};
  if (newHash == m_prevInfoMsgHash) {
    ++m_infoMsgRepetitionCount;
  } else {
    m_infoMsgRepetitionCount = 1;
  }
  m_prevInfoMsgHash = newHash;

  auto setColorAndTitle{[&](const QColor& bg, const QColor& txtColor, const QString& title) {
    ui->widget_infoBox->setStyleSheet(QString{"background-color: %0;"}.arg(getColorStrA(bg)));
    ui->lbl_disp_infoBox->setStyleSheet(QString{"QLabel{color: %0;}"}.arg(getColorStr(txtColor)));
    if (m_infoMsgRepetitionCount <= 1)
      ui->lbl_disp_infoBox->setText(title);
    else
      ui->lbl_disp_infoBox->setText(title + " (" + QString::number(m_infoMsgRepetitionCount) + ")");
  }};

  if (!ui->widget_infoBox->isVisible() && this->isVisible()) {
    ui->widget_infoBox->show();
    ui->widget_infoBox->update();
  }

  switch (type) {
    case InfoBoxMsgType::Type::kWarning:
      setColorAndTitle(
          InfoBoxMsgType::kWarningColor, InfoBoxMsgType::kWarningTextColor, InfoBoxMsgType::kWarningTitleText);
      break;
    case InfoBoxMsgType::Type::kOk:
      setColorAndTitle(InfoBoxMsgType::kOkColor, InfoBoxMsgType::kOkTextColor, InfoBoxMsgType::kOkTitleText);
      break;
    case InfoBoxMsgType::Type::kCritical:
      setColorAndTitle(
          InfoBoxMsgType::kCriticalColor, InfoBoxMsgType::kCriticalTextColor, InfoBoxMsgType::kCriticalTitleText);
      break;
    case InfoBoxMsgType::Type::kUnknown:
    default:
      setColorAndTitle(
          InfoBoxMsgType::kUnknownColor, InfoBoxMsgType::kUnknownTextColor, InfoBoxMsgType::kUnknownTitleText);
      qCritical() << "Unknown InfoBoxMsgType::Type:" << type;
      break;
  }
  ui->lbl_infoBox->setText(message);

  if (timeout > 0) {
    QTimer::singleShot(timeout, this, [this]() { this->hideInfoMessage(); });
  }
}
void UpdateHandler::hideInfoMessage() {
  ui->widget_infoBox->hide();
}

std::optional<qint64> UpdateHandler::startDetachedUpdateProcess(const QString& packagePath,
                                                                const QString& verifierKeyPath,
                                                                const QString& updateManifest,
                                                                const QString& postUpdateCmd) {
  static constexpr auto kStartCommand{"SimpleUpdater.exe"};
  QStringList argList{"-i", packagePath, "-p", postUpdateCmd, "-o", "."};
  if (!(verifierKeyPath.isEmpty() || updateManifest.isEmpty())) {
    argList.append({"-v", verifierKeyPath, "-m", updateManifest});
  }
  return startProcess(kStartCommand, argList, QApplication::applicationDirPath());
}

std::optional<qint64> UpdateHandler::startProcess(const QString& executable, const QStringList& argsList,
                                                  const QString& working_dir) {
  qint64 pid{};
  auto success{QProcess::startDetached(executable, argsList, working_dir, &pid)};
  if (!success || pid == 0) {
    return {};
  }
  return pid;
}

void UpdateHandler::on_pb_close_clicked() {
  this->close();
}

void UpdateHandler::on_pb_checkAvailable_clicked() {
  checkAvailableOnline(false);
}

void UpdateHandler::on_pb_downloadAndInstall_clicked() {
  downloadAndInstall(false);
}

void UpdateHandler::resetState(const QString& errMsg) {
  m_readyToUpdate = false;
  m_latestReleaseInfoOpt = {};
  if (!errMsg.isEmpty()) {
    QMessageBox::warning(this, tr("Error when trying to update"), errMsg);
  }
  setState(States::kReset);
}

void UpdateHandler::setState(States::State newState, bool keepInfoBoxVisible) {
  if (newState == m_state) return;

  auto lambdaSetAllPBEnabled{[&](bool state) {
    ui->pb_checkAvailable->setEnabled(state);
    ui->pb_downloadAndInstall->setEnabled(state);
  }};

  switch (newState) {
    case States::kReset:  // error occured
      lambdaSetAllPBEnabled(true);
      ui->pb_downloadAndInstall->setEnabled(false);
      break;
    case States::kRetrievingManifest:  // downloading something that we can't cancel
    case States::kRetrievingReleaseInfo:
      lambdaSetAllPBEnabled(false);
      break;
    case States::kReleaseInfoRetrieved:
      // fall through
    case States::kManifestRetrieved:
      lambdaSetAllPBEnabled(true);
      break;
    case States::kDownloadingUpdatePackage:
    case States::kUpdatePackageRetrieved:
      lambdaSetAllPBEnabled(false);
      break;
    default:
      qWarning() << "Unhandled UpdateHandler state:" << newState;
      break;
  }
  if (newState != States::kReset && !keepInfoBoxVisible) {
    hideInfoMessage();
  }

  m_state = newState;
}

void UpdateHandler::downloadAndInstall(bool onlyRetrieveManifest) {
  setState(States::kRetrievingManifest);
  if (!m_latestReleaseInfoOpt) {
    resetState();
    showInfoMessage(InfoBoxMsgType::kWarning, tr("Failed to retrieve update information online.\nRetrying..."));
    checkAvailableOnline(true);
    return;
  }
  QString manifestUrl{};
  for (const auto& asset : m_latestReleaseInfoOpt->assetsURLs) {
    if (asset.contains(m_kDistantManifestName)) {
      manifestUrl = asset;
    }
  }
  if (manifestUrl.isEmpty()) {
    resetState();
    qCritical() << "Could not retrieve the manifest url from github";
    showInfoMessage(InfoBoxMsgType::kWarning, tr("Could not retrieve manifest url from github"));
    return;
  }

  qInfo() << "Github manifest url: <" << manifestUrl << ">";

  net::getJsonFromAPI(manifestUrl, [this, onlyRetrieveManifest](std::optional<QJsonDocument> docOpt) {
    this->onManifestRetrieved(std::move(docOpt), !onlyRetrieveManifest);
  });
}

void UpdateHandler::checkAvailableOnline(bool installAfterward) {
  setState(States::kRetrievingReleaseInfo);
  std::function<void(std::optional<updt::ReleaseInfo>)> callback{
      [=, this](std::optional<updt::ReleaseInfo> releaseInfoOpt) {
        this->onLatestUpdateRetrieved(releaseInfoOpt, installAfterward);
      }};
  updt::getLatestReleaseInfoRq(m_kGithubReleaseApiAddress, callback);
}

// NO dialog should be showed here
// unless installAfterwards=true as this should be the case ONLY when user requested it manually
void UpdateHandler::onLatestUpdateRetrieved(std::optional<ReleaseInfo> releaseInfoOpt, bool installAfterwards) {
  if (!releaseInfoOpt) {
    showInfoMessage(InfoBoxMsgType::kCritical,
                    tr("Could not retrieve info about the latest version available online."));
    qCritical() << "Could not retrieve info about the latest version available online.";
    resetState();
    return;
  }
  m_latestReleaseInfoOpt = std::move(releaseInfoOpt.value());
  const auto& releaseInfo{m_latestReleaseInfoOpt.value()};
  ui->lbl_distVersion->setText(to_string(releaseInfo.versionAvailable));

  qInfo() << "Release infos: <" << releaseInfo << ">";
  qInfo() << "--- Update status ---\n\tRunning version: " << m_kRunningVersion
          << "\n\tDistant version: " << releaseInfo.versionAvailable;

  if (m_kRunningVersion >= releaseInfo.versionAvailable) {
    qInfo() << "No installation candidate found (distant version <= running version)";
    showInfoMessage(InfoBoxMsgType::kOk, tr("You are already running the latest available version"));
    resetState();

    if (m_showInstallPropositionOnNextOccasion) m_showInstallPropositionOnNextOccasion = false;

    return;
  }

  qInfo() << "Installation candidate found (distant version > running version)";

  showInfoMessage(InfoBoxMsgType::kWarning, tr("A newer version is available, you should install it"));

  setState(States::kReleaseInfoRetrieved);

  if (m_showInstallPropositionOnNextOccasion) {
    auto ans{QMessageBox::question(this,
                                   tr("Update installation"),
                                   tr("A new update is available (%0). Would you like to install it?")
                                       .arg(to_string(releaseInfo.versionAvailable)))};
    if (ans == QMessageBox::Yes) {
      qInfo() << "User chose to install the new version: " << releaseInfo.versionAvailable;
      downloadAndInstall(false);
      if (!this->isVisible()) {
        this->show();
      }
    } else {
      qInfo() << "User chose NOT to install the new version: " << releaseInfo.versionAvailable;
    }
    m_showInstallPropositionOnNextOccasion = false;
    return;  // give priority to user reply
  }

  if (installAfterwards) {
    downloadAndInstall(false);
    return;
  }

  // if we are finished here
  //    setState(States::kReset);
}

void UpdateHandler::onManifestRetrieved(std::optional<QJsonDocument> docOpt, bool install) {
  // we must enter this function only when m_latestReleaseInfoOpt has a value
  if (!docOpt) {
    resetState();
    qCritical() << "Could not retrieve manifest JSON document";
    showInfoMessage(InfoBoxMsgType::kWarning, tr("Could not retrieve update manifest. Retry later."));
    return;
  }
  qInfo() << "Successfully retrieved manifest JSON document";
  if (!utils::json::save(docOpt.value(), m_kDistantManifestName)) {
    resetState();
    qCritical() << "Could not save manifest file";
    return;
  }
  auto manifestOpt{getManifest(docOpt.value())};
  if (!manifestOpt) {
    resetState();
    qCritical() << "Could not read a valid manifest file from the retrieved manifest";
    return;
  }

  auto minVersion{manifestOpt.value().minVersionRequired};
  // minVersion = Version{0, 0, 0};  // for testing purposes
  setState(States::kManifestRetrieved);

  // download & install update

  if (!install) {
    return;
  }

  if (!m_latestReleaseInfoOpt) {
    qCritical() << "Latest release info could not be found. Unexpected situation - abort update process";
    return;
  }

  qInfo() << "Min version required: <" << minVersion << "> Will update using installer? <"
          << (minVersion > m_kRunningVersion) << ">  Will update using update package? <"
          << !(minVersion > m_kRunningVersion) << ">";
  if (minVersion > m_kRunningVersion) {
    updateFromInstaller(manifestOpt.value());
  } else {
    updateFromPackage();
  }
}

void UpdateHandler::updateFromInstaller(const Manifest& manifest) {
  QString installerLink{};
  for (const auto& link : m_latestReleaseInfoOpt.value().assetsURLs) {
    if (m_kInstallerRegex.match(link).hasMatch()) {
      installerLink = link;
      break;
    }
  }
  if (installerLink.isEmpty()) {
    qCritical() << "Could not find link for installer";
    showInfoMessage(InfoBoxMsgType::kWarning,
                    tr("Sorry, we could not find the update package (installer).\n"
                       "Please retry or contact support"));
    resetState();
    return;
  }

  qInfo() << "Downloading installer at <" << installerLink << ">";

  auto dlManager{new updt::net::DownloadManager({installerLink}, m_kDownloadDir, true, this)};
  connect(dlManager,
          &updt::net::DownloadManager::allDlCompleted,
          this,
          [=, this](int32_t successCount, const QVector<updt::net::FailedDownload>& fails) {
            qDebug() << "Success: <" << successCount << ">  Fails: <" << fails.size() << ">";
            for (const auto& e : fails) {
              qWarning() << "\t Failed to dl <" << e.url << "> because of <" << e.errMsg << ">";
            }
            if (fails.size() != 0) {
              this->resetState();
              this->showInfoMessage(InfoBoxMsgType::kWarning,
                                    tr("Failed to download the update package (installer).\n"
                                       "Retry or contact support"));
              return;
            }

            this->setState(States::kUpdatePackageRetrieved);
            this->showInfoMessage(InfoBoxMsgType::kOk,
                                  tr("Successfully downloaded the update package (installer).\n"
                                     "Starting installation..."));

            const auto verifKey{m_kPblicVerifierKeyFile};
            const auto kInstallerPath{dlManager->GetOutputFile(installerLink)};
            auto safeToInstallOpt{signing::checkFileSignature(kInstallerPath, verifKey, manifest.installerSignature)};
            if (!safeToInstallOpt || !safeToInstallOpt.value()) {
              qCritical() << "Failed to validate installer signature";
              this->showInfoMessage(InfoBoxMsgType::kCritical,
                                    tr("Unsecure update - aborting.\n"
                                       "Download from website or contact support"));
              return;
            }

            onInstallerRetrieved(kInstallerPath);
          });
  dlManager->startAllDownloads();
}

void UpdateHandler::updateFromPackage() {
  QString updatePackageLink{};
  for (const auto& link : m_latestReleaseInfoOpt.value().assetsURLs) {
    if (link.contains(m_kUpdatePackageName)) updatePackageLink = link;
  }
  if (updatePackageLink.isEmpty()) {
    qCritical() << "Could not find update archive link";
    showInfoMessage(InfoBoxMsgType::kWarning,
                    tr("Sorry, we could not find the update package.\n"
                       "Please retry or contact support"));
    resetState();
    return;
  }

  qInfo() << "Downloading the update package at: <" << updatePackageLink << ">";

  auto dlManager{new updt::net::DownloadManager({updatePackageLink}, m_kDownloadDir, true, this)};
  connect(dlManager,
          &updt::net::DownloadManager::allDlCompleted,
          this,
          [&, this](int32_t successCount, const QVector<updt::net::FailedDownload>& fails) {
            qDebug() << "Success: <" << successCount << ">  Fails: <" << fails.size() << ">";
            for (const auto& e : fails) {
              qWarning() << "\t Failed to dl <" << e.url << "> because of <" << e.errMsg << ">";
            }
            if (fails.size() != 0) {
              this->resetState();
              this->showInfoMessage(InfoBoxMsgType::kWarning,
                                    tr("Failed to download the update package.\n"
                                       "Retry or contact support"));
              return;
            }

            this->setState(States::kUpdatePackageRetrieved);
            this->showInfoMessage(InfoBoxMsgType::kOk,
                                  tr("Successfully downloaded the update package.\n"
                                     "Starting installation..."));

            onUpdatePackageRetrieved();
          });
  dlManager->startAllDownloads();
}

void UpdateHandler::onUpdatePackageRetrieved() {
  auto startProcessResult{startDetachedUpdateProcess(
      m_kDownloadDir + m_kUpdatePackageName, m_kPblicVerifierKeyFile, m_kDistantManifestName, m_kPostUpdateCmd)};

  if (!startProcessResult.has_value()) {
    qCritical() << "Failed to start the update process";
    showInfoMessage(InfoBoxMsgType::kCritical, tr("Cannot start the update process for an unknown reason"));
    resetState();
    return;
  }
  qInfo() << "Started update with PID: <" << startProcessResult.value() << ">";
  QApplication::exit(0);
}

void UpdateHandler::onInstallerRetrieved(const QString& installer_path) {
  // TODO(Mathieu) : check if the installer signature matches
  qDebug() << "Installer downloaded at: <" << installer_path << ">";
  auto startProcessResult{startProcess(installer_path)};

  if (!startProcessResult.has_value()) {
    qCritical() << "Failed to start the update process";

    showInfoMessage(InfoBoxMsgType::kCritical, tr("Cannot start the update process for an unknown reason"));
    resetState();
    return;
  }
  qInfo() << "Started update with PID: <" << startProcessResult.value() << ">";
  QApplication::exit(0);
}

void UpdateHandler::on_pb_manualInstall_clicked() {
  QMessageBox::information(this,
                           tr("Information"),
                           tr("Please select at least an update package.\n"
                              "If provided, please select the corresponding UpdateManifest.json file"));

  auto filenames = QFileDialog::getOpenFileNames(
      this,
      tr("Select update files"),
      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
      QString{"%0 (*.pck UpdateManifest*.json);;%1 (*.pck);;%2 (UpdateManifest*.json)"}.arg(
          tr("All"), tr("Update package"), tr("Update manifest")));

  int32_t updatePackageCount{};
  int32_t manifestCount{};
  QString manifest{};
  QString updatePackage{};
  for (const auto& filepath : filenames) {
    QFileInfo fInfo{filepath};
    const QString f{fInfo.fileName()};
    if (f.startsWith("UpdateManifest") && f.endsWith(".json")) {
      ++manifestCount;
      manifest = filepath;
    } else if (f.endsWith(".pck")) {
      updatePackage = filepath;
      ++updatePackageCount;
    }
  }

  if (updatePackageCount > 1 || manifestCount > 1 || updatePackageCount < 1) {
    showInfoMessage(
        InfoBoxMsgType::kCritical,
        tr("Please select at most one update package (.pck) and one update manifest (UpdateManifest*.json)"));
    return;
  }

  auto startProcessResult{
      startDetachedUpdateProcess(updatePackage, m_kPblicVerifierKeyFile, manifest, m_kPostUpdateCmd)};

  if (!startProcessResult.has_value()) {
    qCritical() << "Could not start the update process (" << __PRETTY_FUNCTION__ << ")";
    showInfoMessage(InfoBoxMsgType::kCritical, tr("Cannot start the update process for an unknown reason "));
    resetState();
    return;
  }
  qInfo() << "Started update with PID:" << startProcessResult.value();
  QApplication::exit(0);
}

}  // namespace updt
