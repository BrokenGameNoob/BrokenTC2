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

} // namespace updt
