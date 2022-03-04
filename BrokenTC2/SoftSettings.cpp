#include "SoftSettings.hpp"

#include <QString>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <QFile>
#include <QTextStream>

#include <QDebug>


namespace {

constexpr bool throwErrorOnFileFail{true};

bool saveSettings(const QJsonDocument& doc,const QString& fileName)
{
    auto jsonFormatted(QString(doc.toJson()));
    QFile f(fileName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate)){
        if constexpr(throwErrorOnFileFail)
            throw std::runtime_error{"Cannot open file (w) "+fileName.toStdString()};
        else
            return false;
    }

    QTextStream stream(&f);
    stream << jsonFormatted;
    f.close();
    return true;
}

inline
bool saveSettings(const QJsonObject& obj,const QString& filename)
{
    return saveSettings(QJsonDocument(obj),filename);
}

std::optional<QJsonDocument> readJSONDoc(const QString& fileName)
{
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly)){
        if constexpr(throwErrorOnFileFail)
            throw std::runtime_error{"Cannot open file (r) "+fileName.toStdString()};
        else
        return {};
    }

    auto jsonBytes{f.readAll()};
    f.close();
    auto docR{QJsonDocument::fromJson(jsonBytes)};
    return {docR};
}

}

namespace tc
{

bool saveSettings(const GearHandlerSettings& sett,const QString& fileName)
{
    QJsonObject globObj{};

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

//    QJsonObject gamepad{};
//    gamepad.insert("gearUp",sett.);

    globObj.insert("keys",keys);

    return ::saveSettings(globObj,fileName);
}

GearHandlerSettings readGearHandlerSettings(const QString& fileName)
{
    auto docOpt{::readJSONDoc(fileName)};

    if(!docOpt)
    {
        throw std::runtime_error{"SoftSettings.cpp : "+std::string{__LINE__}+" : "+
                                 std::string{"Cannot find settings file "}+fileName.toStdString()};
    }

    auto docObj{docOpt.value().object()};

    auto keys{docObj.value("keys").toObject()};

    GearHandlerSettings out{};
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
    out.maxGear = keys.value("maxGear").toInt();
    return out;
}

}//namespace tc
