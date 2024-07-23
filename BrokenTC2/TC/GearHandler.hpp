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

#ifndef TC_GEARHANDLER_H
#define TC_GEARHANDLER_H

#include <QObject>

#include "TC/Profile.hpp"

namespace tc {

enum class Gear {
  R = -1,
  N_CLUTCH = 0,
  G1 = 1,
  G2 = 2,
  G3 = 3,
  G4 = 4,
  G5 = 5,
  G6 = 6,
  G7 = 7,
  G8 = 8,
  G9 = 9,
  G10 = 10
};
constexpr auto kSequentialMaxGear{Gear::G10};
constexpr auto kClutchMaxGear{Gear::G7};
inline int toInt(Gear g) {
  return static_cast<int>(g);
}

inline Gear operator+=(Gear g, int v) {
  return static_cast<Gear>(static_cast<int>(g) + v);
}
inline Gear operator+(Gear g, int v) {
  return static_cast<Gear>(static_cast<int>(g) + v);
}
inline Gear operator-=(Gear g, int v) {
  return static_cast<Gear>(static_cast<int>(g) - v);
}
inline Gear operator-(Gear g, int v) {
  return static_cast<Gear>(static_cast<int>(g) - v);
}
inline Gear operator++(Gear g) {
  return g + 1;
}
inline Gear operator--(Gear g) {
  return g - 1;
}
inline bool operator==(Gear g1, Gear g2) {
  return toInt(g1) == toInt(g2);
}
inline bool operator!=(Gear g1, Gear g2) {
  return !(g1 == g2);
}

int getKeyCode(Gear gear, const ProfileSettings& settings);

class GearHandler : public QObject {
  Q_OBJECT

 public:
  GearHandler(QObject* parent = nullptr, ProfileSettings settings = {});

  void setGear(int gear);
  void setGear(Gear gear) {
    setGear(static_cast<int>(gear));
  }
  const auto& gear() const {
    return m_currentGear;
  }

  void switchSeqGear(bool goUp);  // if you don't go up, I'll assume you want to go down

  const ProfileSettings& settings() const {
    return m_settings;
  }
  ProfileSettings& settings() {  // that, here, is a fcking bad idea that lead to some problems
    return m_settings;
  }

  void setGearSwitchMode(GearSwitchMode mode) {
    m_settings.gearSwitchMode = mode;
    emit gearSwitchModeChanged(mode);
  }
  GearSwitchMode mode() const {
    return m_settings.gearSwitchMode;
  }

  Gear getMaxGear() const {
    return m_settings.useSequentialAfterClutch ? kSequentialMaxGear : kClutchMaxGear;
  }

 public slots:
  void gearUp();
  void gearDown();

  void switchGearSwitchMode() {
    if (m_settings.gearSwitchMode == GearSwitchMode::CLUTCH)
      setGearSwitchMode(GearSwitchMode::SEQUENTIAL);
    else
      setGearSwitchMode(GearSwitchMode::CLUTCH);
  }

 signals:
  void gearChanged(int value);
  void gearSwitchModeChanged(tc::GearSwitchMode newMode);

 private:
  ProfileSettings m_settings;

  Gear m_currentGear;
};

}  // namespace tc

#endif  // TC_GEARHANDLER_H
