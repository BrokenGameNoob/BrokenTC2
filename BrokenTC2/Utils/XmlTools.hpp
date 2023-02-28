#ifndef XML_XMLUTILS_HPP
#define XML_XMLUTILS_HPP

#include <QString>
#include <QDomDocument>
#include <QDomNode>
#include <QList>
#include <QFile>
#include <QTextStream>

#include <functional>

namespace xml {

QList<QDomElement> findElement(QDomNode parent,const QString& subNode,
                  std::function<bool(const QDomElement&)> predicate = [](const auto&){return true;});

std::optional<QDomDocument> getXmlDoc(const QString& filePath);

inline
QDomElement getXmlRoot(const QDomDocument& doc){
    return doc.documentElement();
}

bool saveXml(const QDomDocument& doc,const QString& filePath);

} // namespace xml

#endif // XML_XMLUTILS_HPP
