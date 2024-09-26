#include "ConfigEditor.hpp"

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>

#include "TC/Profile.hpp"
#include "Utils/XmlTools.hpp"

namespace tc {

QStringList getBindingsFiles(const QString& configDirPath) {
  //    QDir configDir{configDirPath};
  QDirIterator configDirIt{configDirPath, {"*.xml"}};

  static const QRegularExpression bindingFilenameRegex{R"(Bindings_(?=.*[0-9])(?=.*[a-zA-Z])([a-zA-Z0-9]+)\.xml)"};

  if (!bindingFilenameRegex.isValid()) {
    throw std::runtime_error{"Invalid regex for binding files in " + std::string{__PRETTY_FUNCTION__}};
  }

  QStringList out{};

  while (configDirIt.hasNext()) {
    QString file{configDirIt.next()};
    QFileInfo fi{file};
    file = fi.fileName();
    if (bindingFilenameRegex.match(file).hasMatch()) {
      out << file;
    }
  }

  return out;
}

QString getBindingKeyboardFile(const QString& configDirPath) {
  constexpr auto kBindingFileName{"Bindings_Keyboard.xml"};
  if (QFileInfo::exists(QDir{configDirPath}.filePath(kBindingFileName))) {
    return kBindingFileName;
  }
  return "";
}

namespace xml {

bool editXmlControllerConf(const QString& xmlPath) {
  auto xmlDoc{::xml::getXmlDoc(xmlPath)};
  if (!xmlDoc) {
    qCritical() << "Cannot open file " << xmlPath;
    return false;
  }
  auto xmlRoot{::xml::getXmlRoot(xmlDoc.value())};
  auto actionBindings{::xml::findElement(xmlRoot, "ActionBinding", [&](const QDomElement& e) -> bool {
    return e.attribute("Name", "") == "DrivingBindings";
  })};
  if (actionBindings.size() != 1) {
    qCritical() << QString{"<ActionBinding Name=%0> not found"}.arg("DrivingBindings");
    return false;
  }

  auto actions{::xml::findElement(actionBindings[0], "Action", [&](const QDomElement& e) -> bool {
    auto name{e.attribute("Name", "")};
    return name == "RoadGearUp" || name == "RoadGearDown";
  })};
  if (actions.size() != 2) {
    qCritical() << QString{"<Action Name=(RoadGearUp | RoadGearDown)> not found"}.arg("DrivingBindings");
    return false;
  }

  for (auto& a : actions) {
    auto channels{::xml::findElement(a, "Channel", [&](const QDomElement& e) -> bool {
      auto device{e.attribute("Device", "")};
      return device == "PAD";
    })};
    for (auto& c : channels) {
      a.removeChild(c);
    }
  }

  auto success{::xml::saveXml(xmlDoc.value(), xmlPath)};
  return success;
}

namespace values {
struct Mapping {
  const char* kAttributeName;
  const char* kGameKeyName;
  const UINT kVkKey;
  tc::ProfileSettings::Key tc::ProfileSettings::*kProfileKey;
  constexpr Mapping(const char* attributeName, const char* gameKeyName, const UINT vkKey,
                    tc::ProfileSettings::Key tc::ProfileSettings::*profileKey)
      : kAttributeName{attributeName}, kGameKeyName{gameKeyName}, kVkKey{vkKey}, kProfileKey{profileKey} {}
};

std::array wheelBindingMapping{
    Mapping{"Clutch", "NUM0", VK_NUMPAD0, &tc::ProfileSettings::clutch},
    Mapping{"GearReverse", "NUM8", VK_NUMPAD8, &tc::ProfileSettings::reverse},
    Mapping{"Gear1", "NUM1", VK_NUMPAD1, &tc::ProfileSettings::g1},
    Mapping{"Gear2", "NUM2", VK_NUMPAD2, &tc::ProfileSettings::g2},
    Mapping{"Gear3", "NUM3", VK_NUMPAD3, &tc::ProfileSettings::g3},
    Mapping{"Gear4", "NUM4", VK_NUMPAD4, &tc::ProfileSettings::g4},
    Mapping{"Gear5", "NUM5", VK_NUMPAD5, &tc::ProfileSettings::g5},
    Mapping{"Gear6", "NUM6", VK_NUMPAD6, &tc::ProfileSettings::g6},
    Mapping{"Gear7", "NUM7", VK_NUMPAD7, &tc::ProfileSettings::g7},
};

std::array drivingBindingMapping{Mapping{"RoadGearUp", "NUM_PLUS", VK_ADD, &tc::ProfileSettings::seqGearUp},
                                 Mapping{"RoadGearDown", "NUM_MINUS", VK_SUBTRACT, &tc::ProfileSettings::seqGearDown}};

}  // namespace values

void OnGearActionFound(QDomElement* action, const values::Mapping& mapping, tc::ProfileSettings* settings) {
  // <Channel Device="KEYBOARD" Name=""/>

  while (action->hasChildNodes()) {
    action->removeChild(action->firstChild());
  }

  QDomElement channel{action->ownerDocument().createElement("Channel")};
  channel.setAttribute("Device", "KEYBOARD");
  channel.setAttribute("Name", mapping.kGameKeyName);
  action->appendChild(channel);

  if (settings) {
    *settings.*(mapping.kProfileKey) = mapping.kVkKey;
  }
}

bool editXmlKeyboardConf(const QString& xmlPath, tc::ProfileSettings* settings) {
  qInfo() << "editXmlKeyboardConf: <" << xmlPath << ">";
  auto xmlDoc{::xml::getXmlDoc(xmlPath)};
  if (!xmlDoc) {
    qCritical() << "Cannot open file " << xmlPath;
    return false;
  }
  auto xmlRoot{::xml::getXmlRoot(xmlDoc.value())};
  constexpr auto kWheelActionBindingName{"WheelBindings"};
  auto wheelBindingsList{::xml::findElement(xmlRoot, "ActionBinding", [&](const QDomElement& e) -> bool {
    return e.attribute("Name", "") == kWheelActionBindingName;
  })};
  if (wheelBindingsList.size() != 1) {
    qCritical() << QString{"<ActionBinding Name=%0> not found"}.arg(kWheelActionBindingName);
    return false;
  }
  auto& wheelBinding{wheelBindingsList[0]};

  for (const auto& mapping : values::wheelBindingMapping) {
    auto actions{::xml::findElement(wheelBinding, "Action", [&](const QDomElement& e) -> bool {
      auto name{e.attribute("Name", "")};
      return name == mapping.kAttributeName;
    })};
    if (actions.size() != 1) {
      qCritical() << QString{"<Action Name=%0> not found"}.arg(mapping.kAttributeName);
      return false;
    }
    OnGearActionFound(&actions[0], mapping, settings);
  }

  //

  constexpr auto kDrivingActionBindingName{"WheelBindings"};
  auto drivingBindingsList{::xml::findElement(xmlRoot, "ActionBinding", [&](const QDomElement& e) -> bool {
    return e.attribute("Name", "") == kDrivingActionBindingName;
  })};
  if (drivingBindingsList.size() != 1) {
    qCritical() << QString{"<ActionBinding Name=%0> not found"}.arg(kDrivingActionBindingName);
    return false;
  }
  auto& drivingBinding{drivingBindingsList[0]};

  for (const auto& mapping : values::drivingBindingMapping) {
    auto actions{::xml::findElement(drivingBinding, "Action", [&](const QDomElement& e) -> bool {
      auto name{e.attribute("Name", "")};
      return name == mapping.kAttributeName;
    })};
    if (actions.size() != 1) {
      qCritical() << QString{"<Action Name=%0> not found"}.arg(mapping.kAttributeName);
      return false;
    }
    OnGearActionFound(&actions[0], mapping, settings);
  }

  auto success{::xml::saveXml(xmlDoc.value(), xmlPath)};
  return success;
}

}  // namespace xml
}  // namespace tc
