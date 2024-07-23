/* Broken The Crew 2 sequential clutch assist
 * Copyright (C) 2022 BrokenGameNoob <brokengamenoob@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TC/Widget_gearDisplay.hpp"

#include <QDebug>
#include <QScreen>
#include <QTimer>

#include "../global.hpp"
#include "TC/Profile.hpp"
#include "ui_Widget_gearDisplay.h"

namespace tc {
QString getStyleSheet(const QColor& textColor, const QColor& backgroundColor) {
  QString baseCss{"color:%0;background-color:%1;border-radius:20px"};
  return baseCss.arg(colorToString(textColor)).arg(colorToString(backgroundColor));
}
QColor stringToColor(const QString& input) {
  auto tmp{input.mid(5)};
  tmp.remove(')');
  auto colorList{tmp.split(',')};
  if (colorList.size() != 4) throw std::runtime_error{"Invalid color count for conversion (expected rgba)"};
  return QColor{colorList[0].toInt(), colorList[1].toInt(), colorList[2].toInt(), colorList[3].toInt()};
}
}  // namespace tc

Widget_gearDisplay::Widget_gearDisplay(QWidget* parent) : QWidget(parent), ui(new Ui::Widget_gearDisplay) {
  ui->setupUi(this);
  this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
  this->setAttribute(Qt::WA_TranslucentBackground, true);
  this->setAttribute(Qt::WA_OpaquePaintEvent, false);
  //    this->setStyleSheet("background-color:transparent;color:rgb(255,255,255)");

  refreshGear(0);
  //    this->showFullScreen();

  m_tmpShowTimer.setSingleShot(true);
  connect(&m_tmpShowTimer, &QTimer::timeout, this, [&]() { ui->lbl_notif->hide(); });
  m_tmpShowTimer.setInterval(1500);

  ui->lbl_notif->hide();
}

Widget_gearDisplay::~Widget_gearDisplay() {
  delete ui;
}

void Widget_gearDisplay::setBgHUDColor(QColor bgColor) {
  m_backgroundColor = std::move(bgColor);
  ui->label->setStyleSheet(tc::getStyleSheet(m_currentColor, m_backgroundColor));
  ui->lbl_notif->setStyleSheet(tc::getStyleSheet(m_currentColor, m_backgroundColor));
}

void Widget_gearDisplay::refreshGear(int value) {
  QString text{QString::number(value)};
  if (value <= 0) {
    switch (value) {
      case -1:
        text = "R";
        break;
      case 0:
        text = "N";
        break;
      default:
        text = "-";
        break;
    }
  }
  ui->label->setText(text);
}

void Widget_gearDisplay::onSwitchGearModeChanged(tc::GearSwitchMode newMode) {
  auto newColor{colorFromMode(newMode)};
  ui->label->setStyleSheet(tc::getStyleSheet(newColor, m_backgroundColor));
  m_currentColor = newColor;
}

void Widget_gearDisplay::showNotif(const QString& text) {
  ui->lbl_notif->setStyleSheet(tc::getStyleSheet(m_currentColor, m_backgroundColor));

  ui->lbl_notif->setText(text);
  ui->lbl_notif->show();

  if (m_tmpShowTimer.isActive()) m_tmpShowTimer.stop();
  m_tmpShowTimer.start();
}

void Widget_gearDisplay::showGearModeChangeNotif(tc::GearSwitchMode newMode) {
  showNotif(m_gearModeText[newMode]);
}

void Widget_gearDisplay::showOnScreen(int screenId) {
  auto screenList{QApplication::screens()};
  auto screen{screenList[screenId]};

  if (this->isVisible() && screen == this->screen()) return;
  if (screenId >= 0 && screenId < screenList.size()) {
    this->setScreen(screen);
    this->setGeometry(screen->geometry());
  } else {
    this->showFullScreen();
  }
  if (!this->isVisible()) this->show();
}

void Widget_gearDisplay::setIndicatorVisible(bool visible) {
  ui->label->setVisible(visible);
}
