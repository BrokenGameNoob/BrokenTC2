#pragma once

#include <QDialog>

namespace Ui {
class Dialog_deviceSelection;
}

class Dialog_deviceSelection : public QDialog {
  Q_OBJECT

 public:
  explicit Dialog_deviceSelection(QWidget *parent = nullptr);
  ~Dialog_deviceSelection();

  void populateDevices();
  void setSelectedDevice(const QString &device);

  QString getSelectedDevice() const;

  static QString getDevice(QWidget *parent, const QString &default_selected = "");

 private:
  Ui::Dialog_deviceSelection *ui;
};
