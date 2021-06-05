#pragma once

#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#ifdef USE_OPEN_EXR
#ifndef Q_OS_MAC
#include <OpenEXR/OpenEXRConfig.h>
#endif
#include <OpenEXR/ImfForward.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfNamespace.h>
#include <OpenEXR/ImfRgba.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfTileDescription.h>
#include <OpenEXR/ImfTiledOutputFile.h>
#include <OpenEXR/ImfTiledRgbaFile.h>
#define COMBINED_OPENEXR_VERSION ((10000*OPENEXR_VERSION_MAJOR) + \
                                  (100*OPENEXR_VERSION_MINOR) + \
                                  OPENEXR_VERSION_PATCH)
#if COMBINED_OPENEXR_VERSION >= 20599 /* 2.5.99: pre-3.0 */
#   include <Imath/half.h>
#else
    // OpenEXR 2.x, use the old locations
#   include <OpenEXR/half.h>
#endif

#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfPartHelper.h>
#include <OpenEXR/ImfPartType.h>
#include <OpenEXR/ImfTiledInputPart.h>
#include <OpenEXR/ImfTiledOutputPart.h>

#include <OpenEXR/Iex.h>

#endif

#include "SyntopiaCore/Logging/Logging.h"

#ifdef USE_OPEN_EXR
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
#endif
    
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

    QString resolveName(QString fileName, bool verboseMessages = true);
    QString resolveName(QString fileName, QString originalFileName, bool verboseMessages = true);
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
