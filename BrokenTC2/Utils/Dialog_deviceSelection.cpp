#include "Dialog_deviceSelection.hpp"

#include <QDebug>
#include <QMessageBox>

#include "QSDL/SDLGlobal.hpp"
#include "ui_Dialog_deviceSelection.h"

Dialog_deviceSelection::Dialog_deviceSelection(QWidget* parent) : QDialog(parent), ui(new Ui::Dialog_deviceSelection) {
  ui->setupUi(this);
}

Dialog_deviceSelection::~Dialog_deviceSelection() {
  delete ui;
}

void Dialog_deviceSelection::populateDevices() {
  auto curDevice{ui->cb_devices->currentText()};
  ui->cb_devices->clear();
  auto deviceList{qsdl::getPluggedJoysticks()};
  auto newDeviceIndex{-1};
  int i{};
  for (const auto& e : deviceList) {
    if (curDevice == e) newDeviceIndex = i;
    ui->cb_devices->addItem(e, i);  // store device id as data. Even though it should match cb index
    ++i;
  }
  ui->cb_devices->setCurrentIndex(newDeviceIndex);
}

void Dialog_deviceSelection::setSelectedDevice(const QString& device) {
  for (int i{}; i < ui->cb_devices->count(); ++i) {
    if (ui->cb_devices->itemText(i) == device) {
      ui->cb_devices->setCurrentIndex(i);
      return;
    }
  }
  qWarning() << "Device" << device << "not found";
}

QString Dialog_deviceSelection::getSelectedDevice() const {
  return ui->cb_devices->currentText();
}

QString Dialog_deviceSelection::getDevice(QWidget* parent, const QString& default_selected) {
  auto* dial{new Dialog_deviceSelection{parent}};

  dial->populateDevices();
  dial->setSelectedDevice(default_selected);

  auto ans{dial->exec()};
  if (ans != QDialog::Accepted) return "";

  const auto kRValue{dial->getSelectedDevice()};
  dial->deleteLater();
  return kRValue;
}
