#include "CompressionTools.hpp"

#include <QFileInfo>
#include <optional>

namespace updt {
namespace fs {

/*
 *  Compression format :
 *  File output path
 *  File size in bytes (64 bits)
 *  File bytes
*/

int32_t getRecursiveFileCount(const QString& dir,const QString& workingDir){
    auto realPath{getFilePathFromWorkingDir(dir,workingDir)};
    if(QFileInfo{realPath}.isFile())
        return 1;
    QDirIterator it(realPath,{"*"}, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    int32_t rVal{};
    while (it.hasNext())
    {
        ++rVal;
        it.next();
    }
    return rVal;
}

QString getFilePathFromWorkingDir(const QString& path,const QString& workingDir){
    QFileInfo pInf{path};
    if(pInf.isRelative()){
        return QString{"%0/%1"}.arg(QDir{workingDir}.absolutePath(),pInf.filePath());
    }
    return path;
}

qint64 getFileCountFromPathList(const QStringList& pathList,const QString& workingDir){
    qint64 fileCount{};

    for(const auto& p : pathList)
    {
        auto realPath{getFilePathFromWorkingDir(p,workingDir)};
        QFileInfo inf{realPath};
        if(!inf.exists())
            continue;
        if(inf.isDir())
        {
            fileCount += getRecursiveFileCount(p,workingDir);
        }
        else if(inf.isFile())
        {
            ++fileCount;
        }
        else
            continue;
    }
    return fileCount;
}

QDataStream& operator<<(QDataStream& stream,const CompressedFileInfo& inf){
    stream << inf.outputPath;
    stream << inf.size;
    return stream;
}
QDataStream& operator>>(QDataStream& stream,CompressedFileInfo& inf){
    stream >> inf.outputPath;
    stream >> inf.size;
    return stream;
}

QByteArray compressFile(const QString& inFilePath,const QString& outFilePath){
    QFileInfo inf_f{inFilePath};

    if(!inf_f.exists())
    {
        return {};
    }

    CompressedFileInfo inf{.outputPath=outFilePath,.size=inf_f.size()};

    QByteArray tmp{};
    tmp.reserve(inf.size+sizeof(CompressedFileInfo));
    QBuffer buff{&tmp};
    if(!buff.open(QIODevice::WriteOnly))
    {
        return {};
    }

    QDataStream stream{&buff};
    QFile f{inFilePath};

    if(!f.open(QIODevice::ReadOnly))
    {
        return {};
    }

    auto tmpArr{qCompress(f.readAll(),9)};
    inf.size = tmpArr.size();

    stream << inf;
    stream << tmpArr;

    buff.close();
    f.close();

    return tmp;
}

bool saveBA(const QByteArray& ba,const QString& path){
    QFile f{path};

    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    qDebug() << __PRETTY_FUNCTION__ << ": Saving to " << path <<" >> " << ba.size();
    auto count{f.write(ba)};
    f.close();

    if(count != ba.size())
    {
        return false;
    }

    return true;
}

CompressedFileInfo extractNextFileInfo(QDataStream& stream){
    CompressedFileInfo out;
    stream >> out;
    return out;
}

bool uncompressFile(QDataStream& iStream, std::optional<const CompressedFileInfo> fInfoOpt){
    auto fInfo{fInfoOpt.has_value()?std::move(fInfoOpt.value()):CompressedFileInfo{}};
    if(!fInfoOpt.has_value())
    {
        iStream >> fInfo;
    }
    QByteArray tmp{};
    tmp.reserve(fInfo.size);
    iStream >> tmp;

    return saveBA(qUncompress(tmp),fInfo.outputPath);
}


//----------------------------

QString getFilePathFromRoot(const QString& rootPath,const QString& filePath){
    QDir root{rootPath};
    return root.relativeFilePath(filePath);
}

QString Compressor::compress(const QString& outPath, QString workingDir){
    QByteArray ba{};
    QBuffer buff{&ba,this};
    buff.setData(ba);
    auto success = buff.open(QIODevice::WriteOnly);
    if(!success)
    {
        qCritical() << __PRETTY_FUNCTION__ << ": Can't open output file";
        return QString{};
    }
    QDataStream stream{&buff};
    qint64 fileCount{getFileCountFromPathList(m_customPaths,workingDir)};
    stream << fileCount;

    qInfo() << "Planning on compressing to archive <" << outPath << "> ->" << fileCount << "files";

    for(const auto& p : m_customPaths)
    {
        QString pathToSend{getFilePathFromWorkingDir(p,workingDir)};

        qDebug().nospace() << "\t\t<" << p << "> Add to arch:" << pathToSend;
        compressToStream(pathToSend,stream);
    }

    ba = qCompress(ba,9);
    buff.close();

    qDebug() << ba.size();

    if(saveBA(ba,outPath))
        return outPath;
    return {};
}

bool Compressor::uncompress(const QString& path, const QString& outFolderPath){
//    qDebug() << "=================== " << __PRETTY_FUNCTION__ << " ===================";
    QDir dir(outFolderPath);
    if (!dir.exists())
        dir.mkpath(".");

    QFile compressedArchive{path};
    if(!compressedArchive.open(QIODevice::ReadOnly))
    {
        qCritical() << "Can't open archive file: "<<path;
        return false;
    }
    QByteArray fileBA{compressedArchive.readAll()};
    compressedArchive.close();

    fileBA = qUncompress(fileBA);

    QBuffer fileBuff{&fileBA,this};
    auto success = fileBuff.open(QIODevice::ReadOnly);
    if(!success)
    {
        qCritical() << "Can't open input file";
        return false;
    }
    QDataStream fileStream{&fileBuff};

    qDebug() << "Opening : " << path << "   " << fileBuff.size();

    qint64 fileCount{};
    fileStream >> fileCount;

    qInfo() << "Planning on extracting from archive <" << path << "> ->" << fileCount << "files";

    bool allSucceded{true};
    for(qint64 i{}; i < fileCount; ++i)
    {
        QByteArray elemBA{};
        fileStream >> elemBA;
        QBuffer elemBuff{&elemBA,this};
        success = elemBuff.open(QIODevice::ReadOnly);
        if(!success)
        {
            qCritical() << __PRETTY_FUNCTION__ << ": Can't open compressed element in archive file:"<<path;
            return false;
        }
        QDataStream elemStream{&elemBuff};

        auto fInfo{extractNextFileInfo(elemStream)};
        QFileInfo qFInfo{outFolderPath+'/'+fInfo.outputPath};
        fInfo.outputPath = qFInfo.absoluteFilePath();

        if(!qFInfo.absoluteDir().exists())
            qFInfo.absoluteDir().mkpath(".");
        auto success{uncompressFile(elemStream,fInfo)};
//        qDebug() << to_string(fInfo);
        allSucceded = success && allSucceded;
    }
    fileBuff.close();

    return allSucceded;
}

// PRIVATE

bool Compressor::compressToStream(const QString& path, QDataStream &stream){
    QFileInfo inf{path};
    if(inf.isDir())
    {
        QString root{getParentFolder(path)};
        return compressFolder(root,path,stream);
    }
    else if(inf.isFile())
    {
        return compressFile(path,stream);
    }

    if(!inf.exists())
    {
        qCritical() << __PRETTY_FUNCTION__ << ": File does not exists: " << inf.absoluteFilePath() << "   ->" << inf;
    }
    else
        qCritical() << __PRETTY_FUNCTION__ << ": Unknown input type: " << inf;
    return false;
}

bool Compressor::compressFolder(const QString& root, const QString& path, QDataStream& stream){
    if(QFileInfo{path}.isFile())
        return false;
    QDirIterator it(path,{"*"}, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        qDebug() << it.filePath();
        qDebug() << "\t\toutFilePath:" << fs::getFilePathFromRoot(root,it.filePath());
        auto tmp{fs::compressFile(it.filePath(),fs::getFilePathFromRoot(root,it.filePath()))};
        stream << tmp;
    }
    return true;
}

bool Compressor::compressFile(const QString& path, QDataStream &stream){
    auto root{getParentFolder(path)};
    qDebug() << "Will be extracted to:" << QFileInfo{path}.fileName();
    auto tmp{fs::compressFile(path,QFileInfo{path}.fileName())};
    stream << tmp;
    qDebug() << tmp.size();
    if(tmp.size() == 0)
    {
        qCritical() << "Could not add the file to datastream:" << path;
        return false;
    }
    return true;
}

} // namespace fs
} // namespace updt
