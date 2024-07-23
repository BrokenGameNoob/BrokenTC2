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

#include "GearHandler.hpp"

#include <algorithm>

#include "../global.hpp"

#ifdef Q_OS_WIN
#include "Windows/WinEventHandler.hpp"
#endif

#include <QDebug>
#include <QTimer>
#include <chrono>
#include <thread>

namespace tc {

int getKeyCode(Gear gear, const ProfileSettings& settings) {
  switch (gear) {
    case Gear::R:
      return settings.reverse;
    case Gear::N_CLUTCH:
      return settings.clutch;
    case Gear::G1:
      return settings.g1;
    case Gear::G2:
      return settings.g2;
    case Gear::G3:
      return settings.g3;
    case Gear::G4:
      return settings.g4;
    case Gear::G5:
      return settings.g5;
    case Gear::G6:
      return settings.g6;
    case Gear::G7:
      return settings.g7;
    case Gear::G8:;
  }
  return settings.g7;
}

GearHandler::GearHandler(QObject* parent, ProfileSettings settings)
    : QObject(parent), m_settings{settings}, m_currentGear{} {}

void GearHandler::setGear(int gear) {
  if (m_settings.gearSwitchMode == GearSwitchMode::SEQUENTIAL) {
    throw std::runtime_error{__CURRENT_PLACE_std_ + " : You must not use setGear when in Sequential Mode!"};
  }

#ifdef Q_OS_WIN
  using windows::sendKeyboardEvent;
#else
  auto sendKeyboardEvent{[&](auto a, auto b) {
    qDebug() << __CURRENT_PLACE__ << " - UNIX - "
             << "Sending keyboard event : " << a;
  }};
#endif

  gear = std::clamp(gear, toInt(Gear::R), toInt(kClutchMaxGear));
  const auto kOldGear{m_currentGear};
  m_currentGear = static_cast<Gear>(gear);

  auto keyCode{getKeyCode(m_currentGear, m_settings)};

  auto lambdaSwitchClutch{[&, kOldGear](int gearKeyCode) {
    auto t_starting{std::chrono::high_resolution_clock::now()};

    if (m_currentGear != Gear::N_CLUTCH) {
      sendKeyboardEvent(getKeyCode(Gear::N_CLUTCH, m_settings), true);  // press clutch
    } else {
      gearKeyCode = getKeyCode(kOldGear == Gear::R ? Gear::G2 : Gear::R, m_settings);
    }

    sendKeyboardEvent(gearKeyCode, true);  // press gear key
    std::this_thread::sleep_for(std::chrono::milliseconds(m_settings.keyDownTime));
    sendKeyboardEvent(gearKeyCode, false);  // release gear Key

    sendKeyboardEvent(getKeyCode(Gear::N_CLUTCH, m_settings), false);  // release clutch

    auto t_ending{std::chrono::high_resolution_clock::now()};
    auto duration{std::chrono::duration<double>(t_ending - t_starting).count() * 1000};
    std::ignore = duration;
    // qDebug() << "Gear change took :" << duration << "ms";
  }};

  std::thread t{lambdaSwitchClutch, keyCode};
  t.detach();

  emit gearChanged(gear);
}

void GearHandler::switchSeqGear(bool goUp)  // if you don't go up, I'll assume you want to go down
{
#ifdef Q_OS_WIN
  using windows::sendKeyboardEvent;
#else
  auto sendKeyboardEvent{[&](auto a, auto b) {
    qDebug() << __CURRENT_PLACE__ << " - UNIX - "
             << "Sending keyboard event : " << a;
  }};
#endif
  int gear{};
  ProfileSettings::Key gearKey{};
  if (goUp) {
    gear = toInt(m_currentGear + 1);
    gearKey = m_settings.seqGearUp;
  } else {
    gear = toInt(m_currentGear - 1);
    gearKey = m_settings.seqGearDown;
  }
  gear = std::clamp(gear, toInt(Gear::R), toInt(kSequentialMaxGear));
  m_currentGear = static_cast<Gear>(gear);

  auto lambdaSwitchSeq{[&](int gearKeyCode) {
    sendKeyboardEvent(gearKeyCode, true);  // press gear key
    std::this_thread::sleep_for(std::chrono::milliseconds(m_settings.keyDownTime));
    sendKeyboardEvent(gearKeyCode, false);  // release gear Key
  }};
  std::thread t{lambdaSwitchSeq, gearKey};
  t.detach();

  emit gearChanged(toInt(m_currentGear));
}

void GearHandler::gearUp() {
  const bool kShouldUseSequential{m_settings.useSequentialAfterClutch && m_currentGear >= kClutchMaxGear};
  const bool kUseClutch{m_settings.gearSwitchMode == GearSwitchMode::CLUTCH && !kShouldUseSequential};
  if (kUseClutch) {
    if (settings().skipNeutral) {
      setGear(m_currentGear + (((m_currentGear + 1) == tc::Gear::N_CLUTCH) ? 2 : 1));  // skip neutral
    } else {
      setGear(m_currentGear + 1);
    }
  } else  // GearSwitchMode::SEQUENTIAL
    switchSeqGear(true);
}

void GearHandler::gearDown() {
  const bool kShouldUseSequential{m_settings.useSequentialAfterClutch && m_currentGear > (kClutchMaxGear + 1)};
  const bool kUseClutch{m_settings.gearSwitchMode == GearSwitchMode::CLUTCH && !kShouldUseSequential};
  if (kUseClutch) {
    if (settings().skipNeutral) {
      setGear(m_currentGear - ((((m_currentGear - 1) == tc::Gear::N_CLUTCH) ? 2 : 1)));  // skip neutral
    } else {
      setGear(m_currentGear - 1);
    }
  } else  // GearSwitchMode::SEQUENTIAL
    switchSeqGear(false);
}

}  // namespace tc
