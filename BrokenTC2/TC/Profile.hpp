#ifndef TC_PROFILE_HPP
#define TC_PROFILE_HPP

#include <QString>

namespace tc {

enum class GearSwitchMode{
    CLUTCH=0,
    SEQUENTIAL=1
};
inline int toInt(GearSwitchMode g){return static_cast<int>(g);}

struct ProfileSettings{
    using Key = int;
    using Button = int;

    //---- global ----//

    QString profileName{"Default"};
    GearSwitchMode gearSwitchMode{GearSwitchMode::CLUTCH};

    int maxGear{7};

    //---- keyboard ----//

    Key reverse{-1};//key "8"
    Key clutch{-1};//key "0"
    Key g1{-1};//key "1"
    Key g2{-1};//key "2"
    Key g3{-1};//key "3"
    Key g4{-1};//key "4"
    Key g5{-1};//key "5"
    Key g6{-1};//key "6"
    Key g7{-1};//key "7"

    //the crew keyboard keys for gear up/down
    Key seqGearUp{-1};//key "e"
    Key seqGearDown{-1};//key "a"

    int keyDownTime{20};//ms

    //---- Controller ----//

    Button gearUp{4};
    Button gearDown{5};
    Button setFirstGear{-1};

    Button switchMode{-1};
};

bool saveSettings(const ProfileSettings& sett,const QString& fileName);

ProfileSettings readProfileSettings(const QString& fileName);

}//namespace tc

#endif // TC_PROFILE_HPP
