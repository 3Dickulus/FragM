#pragma once

#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include "SyntopiaCore/Logging/Logging.h"


/// Small class for handling include paths
namespace Fragmentarium
{
namespace GUI
{

class FileManager : public QObject
{
    Q_OBJECT

public:

    FileManager() {}

    void setOriginalFileName ( QString f )
    {
        originalFileName = f;
    }

    void setIncludePaths ( QStringList paths )
    {
        includePaths = paths;
    }

    QString resolveName(QString fileName);
    QString resolveName(QString fileName, QString originalFileName);
    bool fileExists(QString fileName);
    QStringList getFiles ( QStringList filters );
    QStringList getImageFiles();

    QStringList getIncludePaths()
    {
        return includePaths;
    }

private:
    QString originalFileName;
    QStringList includePaths;
    QMap<QString, QStringList> cachedFilters;
};

} // namespace GUI
} // namespace Fragmentarium
