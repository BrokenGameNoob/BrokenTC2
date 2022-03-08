#include "PostUpdate.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>

#include <QDebug>

namespace{

inline
void postUpdate()
{
    auto appdataPath{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};
    QDir newAppdata{appdataPath};
    QFileInfo appdataInfo{appdataPath};
    auto appdataRoaming{appdataInfo.dir()};
    QDir oldAppdataDir{appdataRoaming.path()+"/BrokenTC2v0"};

    if(!appdataRoaming.exists("BrokenTC2v0"))
    {
        qDebug() << "Old dir not found";
        return;
    }

    if(newAppdata.exists())//if the new dir already exists (How ?)
    {
        newAppdata.removeRecursively();//delete it
    }

    qDebug() << oldAppdataDir.rename(oldAppdataDir.path(),"../"+QFileInfo(appdataPath).completeBaseName());
}

}


namespace updt {

void postUpdateFunction()
{
    if(!wasUpdated())
    {
        qDebug() << "Wasnt updated";
        return;
    }

    postUpdate();

    markAsNotUpdated();
    qDebug() << "Was updated";
}

} // namespace updt
