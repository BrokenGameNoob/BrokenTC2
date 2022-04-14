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

#include "Profile.hpp"
#include "../global.hpp"

#include <QString>

#include "Utils/JSONTools.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <QFile>
#include <QTextStream>

#include <QDebug>




namespace tc
{

bool saveSettings(const ProfileSettings& sett,const QString& fileName)
{
    QJsonObject globObj{};

    globObj.insert("name",sett.profileName);
    globObj.insert("gearSwitchMode",toInt(sett.gearSwitchMode));

    QJsonObject keys{};
    keys.insert("gear_reverse",sett.reverse);
    keys.insert("gear_clutch",sett.clutch);
    keys.insert("gear_1",sett.g1);
    keys.insert("gear_2",sett.g2);
    keys.insert("gear_3",sett.g3);
    keys.insert("gear_4",sett.g4);
    keys.insert("gear_5",sett.g5);
    keys.insert("gear_6",sett.g6);
    keys.insert("gear_7",sett.g7);
    keys.insert("seqGearUp",sett.seqGearUp);
    keys.insert("seqGearDown",sett.seqGearDown);

    keys.insert("kSwitchMode",sett.kSwitchMode);
    keys.insert("keyDownTime",sett.keyDownTime);
    keys.insert("maxGear",sett.maxGear);

    QJsonObject controller{};
    controller.insert("gearUp",sett.gearUp);
    controller.insert("gearDown",sett.gearDown);
    controller.insert("setFirstGear",sett.setFirstGear);
    controller.insert("switchMode",sett.switchMode);

    globObj.insert("keys",keys);
    globObj.insert("controller",controller);

    auto rval{utils::json::save(globObj,fileName)};
    return rval;
}

ProfileSettings readProfileSettings(const QString& fileName)
{
    auto docOpt{utils::json::read(fileName)};

    if(!docOpt)
    {
        throw std::runtime_error{"SoftSettings.cpp : "+std::string{__LINE__}+" : "+
                                 std::string{"Cannot find settings file "}+fileName.toStdString()};
    }

    auto docObj{docOpt.value().object()};

    ProfileSettings out{};
    ProfileSettings ref{};

    out.profileName = docObj.value("name").toString(ref.profileName);
    out.gearSwitchMode = GearSwitchMode{docObj.value("gearSwitchMode").toInt(
                                        toInt(ref.gearSwitchMode))};

    auto keys{docObj.value("keys").toObject()};
    out.reverse = keys.value("gear_reverse").toInt(ref.reverse);
    out.clutch = keys.value("gear_clutch").toInt(ref.clutch);
    out.g1 = keys.value("gear_1").toInt(ref.g1);
    out.g2 = keys.value("gear_2").toInt(ref.g2);
    out.g3 = keys.value("gear_3").toInt(ref.g3);
    out.g4 = keys.value("gear_4").toInt(ref.g4);
    out.g5 = keys.value("gear_5").toInt(ref.g5);
    out.g6 = keys.value("gear_6").toInt(ref.g6);
    out.g7 = keys.value("gear_7").toInt(ref.g7);
    out.seqGearUp = keys.value("seqGearUp").toInt(ref.seqGearUp);
    out.seqGearDown = keys.value("seqGearDown").toInt(ref.seqGearDown);
    out.kSwitchMode = keys.value("kSwitchMode").toInt(ref.kSwitchMode);
    out.keyDownTime = keys.value("keyDownTime").toInt(ref.keyDownTime);

    auto controller{docObj.value("controller").toObject()};
    out.gearUp = controller.value("gearUp").toInt(ref.gearUp);
    out.gearDown = controller.value("gearDown").toInt(ref.gearDown);
    out.setFirstGear = controller.value("setFirstGear").toInt(ref.setFirstGear);
    out.switchMode = controller.value("switchMode").toInt(ref.switchMode);

    out.maxGear = keys.value("maxGear").toInt();

    return out;
}

}//namespace tc
