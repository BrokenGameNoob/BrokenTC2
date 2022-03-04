#ifndef TC_PROFILE_HPP
#define TC_PROFILE_HPP

#include <QString>

namespace tc {

struct ProfileSettings{
    using Key = int;
    using Button = int;

    //---- global ----//

    QString profileName{"Default"};
    int maxGear{7};

    //---- keyboard ----//

    Key reverse{56};//key "8"
    Key clutch{75};//key "0"
    Key g1{49};//key "1"
    Key g2{50};//key "2"
    Key g3{51};//key "3"
    Key g4{52};//key "4"
    Key g5{53};//key "5"
    Key g6{54};//key "6"
    Key g7{55};//key "7"

    int interActionDelay{20};//ms

    //---- Controller ----//

    Button gearUp{4};
    Button gearDown{5};

};

bool saveSettings(const ProfileSettings& sett,const QString& fileName);

ProfileSettings readProfileSettings(const QString& fileName);

}//namespace tc

#endif // TC_PROFILE_HPP
