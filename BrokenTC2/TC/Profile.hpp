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

#ifndef TC_PROFILE_HPP
#define TC_PROFILE_HPP

#include <QString>

namespace tc {

enum class GearSwitchMode { CLUTCH = 0, SEQUENTIAL = 1 };
inline int toInt(GearSwitchMode g) {
  return static_cast<int>(g);
}

struct ProfileSettings {
  using Key = int;
  using Button = int;

  //---- global ----//

  QString profileName{"Default"};
  GearSwitchMode gearSwitchMode{GearSwitchMode::CLUTCH};

  //---- keyboard ----//

  Key reverse{-1};  // key "8"
  Key clutch{-1};   // key "0"
  Key g1{-1};       // key "1"
  Key g2{-1};       // key "2"
  Key g3{-1};       // key "3"
  Key g4{-1};       // key "4"
  Key g5{-1};       // key "5"
  Key g6{-1};       // key "6"
  Key g7{-1};       // key "7"

  // the crew keyboard keys for gear up/down
  Key seqGearUp{-1};    // key "e"
  Key seqGearDown{-1};  // key "a"

  Key kSwitchMode{-1};    // keyboard key to switch
  Key kCycleProfile{-1};  // keyboard key to cycle controller
  Key keyboardSeqGearUp{-1};
  Key keyboardSeqGearDown{-1};
  int keyDownTime{20};  // ms
  bool skipNeutral{true};
  bool useSequentialAfterClutch{false};  // if true, after 7th gear, it will switch use sequential

  //---- Controller ----//

  Button gearUp{4};
  Button gearDown{5};

  Button setReverseGear{-1};
  Button setFirstGear{-1};
  Button setSecondGear{-1};
  Button setThirdGear{-1};
  Button setFourthGear{-1};
  Button setFifthGear{-1};
  Button setSixthGear{-1};
  Button setSeventhGear{-1};

  Button switchMode{-1};
  Button cycleProfile{-1};
};

bool saveSettings(const ProfileSettings &sett, const QString &fileName);

ProfileSettings readProfileSettings(const QString &fileName);

}  // namespace tc

#endif  // TC_PROFILE_HPP
