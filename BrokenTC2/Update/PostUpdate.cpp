#include "PostUpdate.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>

#include <QDialog>
#include <QTextBrowser>
#include <QString>
#include <QTextStream>
#include <QMessageBox>
#include <QVBoxLayout>

#include <QDebug>

namespace{

inline
void postUpdate()
{
    //v0.1.0 -> v0.1.1
}

}


namespace updt {

bool postUpdateFunction()
{
    if(!wasUpdated())
    {
        qDebug() << "Wasnt updated";
        return false;
    }

    postUpdate();

    markAsNotUpdated();
    qDebug() << "Was updated";
    return true;
}

void showChangelog(QWidget* parent,const QString& changelogMdFile)
{
    auto dial{QDialog(parent)};
    dial.setWindowTitle(QObject::tr("Changelog"));
    dial.setModal(true);
    if(parent){
        dial.resize(parent->width()*0.9,parent->height()*0.9);
    }


    QVBoxLayout* lay{new QVBoxLayout(&dial)};

    auto tb{new QTextBrowser(&dial)};
    lay->addWidget(tb);

    QFile f(changelogMdFile);
    if(!f.open(QIODevice::ReadOnly))
    {
        qCritical() << "Cannot open dialog file";
        QMessageBox::critical(parent,QObject::tr("Error"),QObject::tr("Cannot open changelog file"));
        return;
    }
    QString fContent{f.readAll()};

    tb->setMarkdown(fContent);

    dial.exec();
}

} // namespace updt
