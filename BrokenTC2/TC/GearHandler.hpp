#ifndef TC_GEARHANDLER_H
#define TC_GEARHANDLER_H

#include <algorithm>

#include "TC/Profile.hpp"

#include <QObject>

namespace tc {

enum class Gear{
    R = -1,
    N_CLUTCH = 0,
    G1 = 1,
    G2 = 2,
    G3 = 3,
    G4 = 4,
    G5 = 5,
    G6 = 6,
    G7 = 7
};
inline int toInt(Gear g){return static_cast<int>(g);}

inline Gear operator+=(Gear g,int v){return static_cast<Gear>(static_cast<int>(g)+v);}
inline Gear operator+(Gear g,int v){return static_cast<Gear>(static_cast<int>(g)+v);}
inline Gear operator-=(Gear g,int v){return static_cast<Gear>(static_cast<int>(g)-v);}
inline Gear operator-(Gear g,int v){return static_cast<Gear>(static_cast<int>(g)-v);}
inline Gear operator++(Gear g){return g+1;}
inline Gear operator--(Gear g){return g-1;}
inline bool operator==(Gear g1,Gear g2){return toInt(g1) == toInt(g2);}
inline bool operator!=(Gear g1,Gear g2){return !(g1==g2);}


int getKeyCode(Gear gear,const ProfileSettings& settings);

class GearHandler : public QObject
{
Q_OBJECT

public:
    GearHandler(QObject* parent=nullptr,ProfileSettings settings = {});

    void setGear(int gear);
    void setGear(Gear gear){
        setGear(static_cast<int>(gear));
    }

    void switchSeqGear(bool goUp);//if you don't go up, I'll assume you want to go down

    const ProfileSettings& settings()const{
        return m_settings;
    }
    ProfileSettings& settings(){//that, here, is a fcking bad idea that lead to some problems
        return m_settings;
    }

    void setGearSwitchMode(GearSwitchMode mode){
        m_settings.gearSwitchMode = mode;
        emit gearSwitchModeChanged(mode);
    }

public slots:
    void gearUp(){
        if(m_settings.gearSwitchMode == GearSwitchMode::CLUTCH)
            setGear(m_currentGear + 1);
        else//GearSwitchMode::SEQUENTIAL
            switchSeqGear(true);
    }
    void gearDown(){
        if(m_settings.gearSwitchMode == GearSwitchMode::CLUTCH)
            setGear(m_currentGear - 1);
        else//GearSwitchMode::SEQUENTIAL
            switchSeqGear(false);
    }

    void switchGearSwitchMode(){
        if(m_settings.gearSwitchMode == GearSwitchMode::CLUTCH)
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

} // namespace tc

#endif // TC_GEARHANDLER_H
