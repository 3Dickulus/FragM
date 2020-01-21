#include "../../SyntopiaCore/Exceptions/Exception.h"
#include "FileManager.h"
#include <QDebug>
#include <QDir>
#include <QImageReader>

using namespace SyntopiaCore::Exceptions;

using namespace SyntopiaCore::Logging;
namespace Fragmentarium
{
namespace GUI
{

QStringList FileManager::getImageFiles()
{
    QStringList extensions;
    QList<QByteArray> a;
    a << "hdr";
// #ifdef USE_OPEN_EXR
  #ifdef Q_OS_WIN
    a << "exr";
  #endif
// #endif
    a << QImageReader::supportedImageFormats();
    foreach(QByteArray s, a) {
        extensions.append(QString("*."+s));
    }
    return getFiles(extensions);
}

QStringList FileManager::getFiles(QStringList filters)
{
    QString key = filters.join(";;");
    if (cachedFilters.contains(key)) {
        return cachedFilters[key];
    }
    QStringList entries;

    // Check relative to current file
    if (!originalFileName.isEmpty()) {
        QDir d = QFileInfo(originalFileName).absolutePath();
        entries = d.entryList(filters,QDir::Files) ;
    }

    // Check relative to files in include path
    foreach (QString p, includePaths) {
        QDir d(p);
        entries += d.entryList(filters,QDir::Files) ;
    }

    cachedFilters[key] = entries;
    return entries;
}

bool FileManager::fileExists(QString fileName)
{
    // First check absolute filenames
    if (QFileInfo(fileName).isAbsolute()) {
        return QFileInfo(fileName).exists();
    }

    // Check relative to current file
    if (!originalFileName.isEmpty()) {
        QDir d = QFileInfo(originalFileName).absolutePath();
        QString path = d.absoluteFilePath(fileName);
        if (QFileInfo(path).exists()) {
            return true;
        }
    }

    // Check relative to files in include path
    foreach (QString p, includePaths) {
        QString path = QDir(p).absoluteFilePath(fileName);
        if (QFileInfo(path).exists()) {
            return true;
        }
    }

    return false;
}

QString FileManager::resolveName(QString fileName, bool verboseMessages)
{
    return resolveName(fileName, originalFileName, verboseMessages);
}

QString FileManager::resolveName(QString fileName, QString originalFileName, bool verboseMessages)
{

        // First check absolute filenames
    if (QFileInfo(fileName).isAbsolute()) {
        return fileName;
    }
        QStringList pathsTried;

        // Check relative to current file
        if (!originalFileName.isEmpty()) {
            QDir d = QFileInfo(originalFileName).absolutePath();
            QString path = d.absoluteFilePath(fileName);
        if (QFileInfo(path).exists() && QFileInfo(path).isFile()) {
            return path;
        }
            pathsTried.append(path);
        }

        // Check relative to files in include path
        foreach (QString p, includePaths) {
            QDir d(p);
            QString path = d.absoluteFilePath(fileName);
        if (QFileInfo(path).exists() && QFileInfo(path).isFile()) {
            return path;
        }
            pathsTried.append(path);
        }

        // We failed.
        if (verboseMessages) {
            foreach (QString s, pathsTried) {
              INFO(QCoreApplication::translate("FileManager","Tried path: ") + s);
            }
        }
    throw Exception(QCoreApplication::translate("FileManager", "Could not resolve path for file: ") + fileName);
}

} // namespace GUI
} // namespace Fragmentarium
