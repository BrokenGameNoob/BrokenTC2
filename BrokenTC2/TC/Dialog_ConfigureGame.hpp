#ifndef DIALOG_CONFIGUREGAME_HPP
#define DIALOG_CONFIGUREGAME_HPP

#include <QDialog>

#include "TC/gameProfile.hpp"

namespace Ui {
class Dialog_ConfigureGame;
}

class Dialog_ConfigureGame : public QDialog {
  Q_OBJECT

  enum PagesIndex : int32_t { kIntro = 0, kNoConfigFileFound = 1, kWaitingForTC2Closed = 2, kDone = 3 };

 public:
  explicit Dialog_ConfigureGame(const QStringList &availableDevices, const QString &currentDevice,
                                const QString &appdataFolder, QWidget *parent = nullptr);
  ~Dialog_ConfigureGame();

  bool succeeded() const {
    return m_succeeded;
  }

  static bool configure(QWidget *parent, const QStringList &availableDevices, const QString &currentDevice,
                        const QString &appdataFolder);

  void configure();

 private slots:
  void setWaitWidgetsVisible(bool visible);
  void onCurrentIndexChanged(int curIndex);

  void on_pb_cancel_clicked();

  void on_pb_nextOk_clicked();

  void on_comboBox_currentIndexChanged(int index);

 private:
  void closeEvent(QCloseEvent *e);

  tc::GameId getSelectedGameId() const;

 private:
  Ui::Dialog_ConfigureGame *ui;

  QString m_appdataFolder{};
  QString m_currentDevice{};
  QStringList m_availableDevices{};

  bool m_succeeded{true};

  bool m_breakInfLoop{false};
};

#endif  // DIALOG_CONFIGUREGAME_HPP
