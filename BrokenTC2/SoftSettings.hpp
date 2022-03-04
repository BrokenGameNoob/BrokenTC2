#ifndef SOFTSETTINGS_H
#define SOFTSETTINGS_H

#include <QString>

namespace tc {

struct GearHandlerSettings{
    using Key = int;

    Key reverse{56};//key "8"
    Key clutch{75};//key "0"
    Key g1{49};//key "1"
    Key g2{50};//key "2"
    Key g3{51};//key "3"
    Key g4{52};//key "4"
    Key g5{53};//key "5"
    Key g6{54};//key "6"
    Key g7{55};//key "7"

    int keyDownTime{20};//ms
    int maxGear{-9999};
};

bool saveSettings(const GearHandlerSettings& sett,const QString& fileName);

GearHandlerSettings readGearHandlerSettings(const QString& fileName);

}//namespace tc

#endif // SOFTSETTINGS_H
