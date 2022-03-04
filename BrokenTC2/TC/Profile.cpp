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

    keys.insert("gearDelay",sett.interActionDelay);
    keys.insert("maxGear",sett.maxGear);

    QJsonObject controller{};
    controller.insert("gearUp",sett.gearUp);
    controller.insert("gearDown",sett.gearDown);

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

    out.profileName = docObj.value("name").toString();

    auto keys{docObj.value("keys").toObject()};
    out.reverse = keys.value("gear_reverse").toInt();
    out.clutch = keys.value("gear_clutch").toInt();
    out.g1 = keys.value("gear_1").toInt();
    out.g2 = keys.value("gear_2").toInt();
    out.g3 = keys.value("gear_3").toInt();
    out.g4 = keys.value("gear_4").toInt();
    out.g5 = keys.value("gear_5").toInt();
    out.g6 = keys.value("gear_6").toInt();
    out.g7 = keys.value("gear_7").toInt();
    out.interActionDelay = keys.value("gearDelay").toInt();

    auto controller{docObj.value("controller").toObject()};
    out.gearUp = controller.value("gearUp").toInt();
    out.gearDown = controller.value("gearDown").toInt();

    out.maxGear = keys.value("maxGear").toInt();

    return out;
}

}//namespace tc
