#include "ConfigEditor.hpp"

#include "Utils/XmlTools.hpp"

#include <QDirIterator>
#include <QRegularExpression>
#include <QFileInfo>

#include <QDebug>

namespace tc {

QStringList getBindingsFiles(const QString& configDirPath){
//    QDir configDir{configDirPath};
    QDirIterator configDirIt{configDirPath,{"*.xml"}};

    static const QRegularExpression bindingFilenameRegex{R"(Bindings_(?=.*[0-9])(?=.*[a-zA-Z])([a-zA-Z0-9]+)\.xml)"};

    if(!bindingFilenameRegex.isValid())
    {
        throw std::runtime_error{"Invalid regex for binding files in "+std::string{__PRETTY_FUNCTION__}};
    }

    QStringList out{};

    while(configDirIt.hasNext())
    {
        QString file{configDirIt.next()};
        QFileInfo fi{file};
        file = fi.fileName();
        if(bindingFilenameRegex.match(file).hasMatch())
        {
            out << file;
        }
    }

    return out;
}



namespace xml {

bool editXmlControllerConf(const QString& xmlPath){
    auto xmlDoc{::xml::getXmlDoc(xmlPath)};
    if(!xmlDoc)
    {
        qCritical() << "Cannot open file "<<xmlPath;
        return false;
    }
    auto xmlRoot{::xml::getXmlRoot(xmlDoc.value())};
    auto actionBindings{::xml::findElement(xmlRoot,"ActionBinding",[&](const QDomElement& e)->bool{
            return e.attribute("Name","") == "DrivingBindings";
        })};
    if(actionBindings.size() != 1)
    {
        qCritical() << QString{"<ActionBinding Name=%0> not found"}.arg("DrivingBindings");
        return false;
    }

    auto actions{::xml::findElement(actionBindings[0],"Action",[&](const QDomElement& e)->bool{
            auto name{e.attribute("Name","")};
            return name == "RoadGearUp" || name == "RoadGearDown";
        })};
    if(actions.size() != 2)
    {
        qCritical() << QString{"<Action Name=(RoadGearUp | RoadGearDown)> not found"}.arg("DrivingBindings");
        return false;
    }

    for(auto& a : actions)
    {
        auto channels{::xml::findElement(a,"Channel",[&](const QDomElement& e)->bool{
                auto device{e.attribute("Device","")};
                return device == "PAD";
            })};
        for(auto& c : channels)
        {
            a.removeChild(c);
        }
    }

    auto success{::xml::saveXml(xmlDoc.value(),xmlPath)};
    return success;
}

} // namespace xml
} // namespace tc
