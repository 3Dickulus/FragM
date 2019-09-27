#pragma once

#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#ifdef USE_OPEN_EXR
#ifndef Q_OS_MAC
#include <OpenEXRConfig.h>
#endif
#include <ImfForward.h>
#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfNamespace.h>
#include <ImfRgba.h>
#include <ImfRgbaFile.h>
#include <ImfTileDescription.h>
#include <ImfTiledOutputFile.h>
#include <ImfTiledRgbaFile.h>
#include <half.h>

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfPartHelper.h>
#include <ImfPartType.h>
#include <ImfTiledInputPart.h>
#include <ImfTiledOutputPart.h>

#include <Iex.h>

#endif

#include "SyntopiaCore/Logging/Logging.h"

    struct RGBAHALF
    {
        half r;
        half g;
        half b;
        half a;
        
        RGBAHALF () {}
        RGBAHALF (half r, half g, half b, half a = 1.f): r (r), g (g), b (b), a (a) {}

        RGBAHALF & operator = (const RGBAHALF &other)
        {
            r = other.r;
            g = other.g;
            b = other.b;
            a = other.a;

            return *this;
        }
    };
    
    struct RGBAFLOAT
    {
        float r;
        float g;
        float b;
        float a;
        
        RGBAFLOAT () {}
        RGBAFLOAT (float r, float g, float b, float a = 1.f): r (r), g (g), b (b), a (a) {}

        RGBAFLOAT & operator = (const RGBAFLOAT &other)
        {
            r = other.r;
            g = other.g;
            b = other.b;
            a = other.a;

            return *this;
        }
    };
    

    struct RGBAUINT
    {
        uint r;
        uint g;
        uint b;
        uint a;
        
        RGBAUINT () {}
        RGBAUINT (uint r, uint g, uint b, uint a = 1.f): r (r), g (g), b (b), a (a) {}

        RGBAUINT & operator = (const RGBAUINT &other)
        {
            r = other.r;
            g = other.g;
            b = other.b;
            a = other.a;

            return *this;
        }
    };
    
/// Small class for handling include paths
namespace Fragmentarium
{
namespace GUI
{

#ifdef USE_OPEN_EXR
using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
#endif

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
