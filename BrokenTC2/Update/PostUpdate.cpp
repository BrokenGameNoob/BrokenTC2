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
    //v0.1.0 -> v0.1.1
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
