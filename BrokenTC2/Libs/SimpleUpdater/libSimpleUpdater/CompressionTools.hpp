#ifndef FS_COMPRESSIONTOOLS_HPP
#define FS_COMPRESSIONTOOLS_HPP

#include <QByteArray>
#include <QString>
#include <QBuffer>
#include <QDataStream>
#include <QFileInfo>
#include <QDir>

#include <QDirIterator>

namespace updt{
namespace fs {

struct CompressedFileInfo{
    QString outputPath{};
    qint64 size{};//in bytes
};

inline
QString to_string(const CompressedFileInfo& inf){
    return QString{"CompressedFileInfo{%0,%1}"}
            .arg(inf.outputPath)
            .arg(inf.size);
}

int32_t getRecursiveFileCount(const QString& dir,const QString& workingDir = ".");

/*!
 * \brief Get an absolute file path from a relative path + working dir or an absolute given path
 * \param path: file path to analyse
 * \param workingDir: working dir used if the given path is relative
 * \return either "workingDir/path" for a relative path OR "path" for an absolute path
 */
QString getFilePathFromWorkingDir(const QString& path,const QString& workingDir);
qint64 getFileCountFromPathList(const QStringList& pathList, const QString &workingDir = ".");

QDataStream& operator<<(QDataStream& stream,const CompressedFileInfo& inf);
QDataStream& operator>>(QDataStream& stream,CompressedFileInfo& inf);

QByteArray compressFile(const QString& inFilePath, const QString &outFilePath);

bool saveBA(const QByteArray& ba,const QString& path);

// -- reading

CompressedFileInfo extractNextFileInfo(QDataStream& stream);
bool uncompressFile(QDataStream& iStream, std::optional<const CompressedFileInfo> fInfoOpt = {});


// -- Manager

QString getFilePathFromRoot(const QString& rootPath,const QString& filePath);

//TODO: Make a struct to store global info on the compressor used as one entity

//Compression format:
//  File:
//* CompressedFileInfo + qCompress(fileByteArray)
//
//  Using Compressor:
//  Archive format:
//      qCompress(
//          fileCount,
//          compressedFile0 -> CompressedFileInfo + qCompress(fileByteArray),
//          compressedFile1 -> CompressedFileInfo + qCompress(fileByteArray),
//          ...
//      )


/*!
 * \brief The Compressor class
 *
 * Usage: set customPaths to all files you want to compress. And use the "compress" function with the wanted output
 */
class Compressor : public QObject
{
Q_OBJECT

public:
    Compressor(QString compressedExtension = ".comp", QObject* parent = nullptr) :
        QObject(parent),
        m_compressedExtension{std::move(compressedExtension)}
    {
    }

    static QString getParentFolder(const QString& path){
        QFileInfo inf{path};
        return inf.absoluteDir().path();
    }

    QString getCompressedFilePath(const QString& toCompressPath){
        QFileInfo inf{toCompressPath};
        return inf.absoluteDir().path()+"/"+inf.completeBaseName()+m_compressedExtension;
    }

    auto& paths(){
        return m_customPaths;
    }
    void addPath(QString p){
        m_customPaths.emplace_back(std::move(p));
    }

    /*!
     * workingDir is used to find files defined from relative paths. If left empty, it won't be taken
     * into account (aka. the program cwd will be used)
    */
    QString compress(const QString& outPath, QString workingDir = {"."});

    bool uncompress(const QString& path, const QString& outFolderPath = ".");

private:
    bool compressToStream(const QString& path, QDataStream &stream);
    bool compressFolder(const QString &root, const QString& path, QDataStream &stream);
    bool compressFile(const QString& path, QDataStream &stream);

signals:
    void progress(int percentage);

private:
//    bool m_isBusy{false};
    QString m_compressedExtension;

    QStringList m_customPaths{};
};

} // namespace fs
} //namespace updt

#endif // FS_COMPRESSIONTOOLS_HPP
