#pragma once

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QVector>
#include <QMap>


#include <glm/glm.hpp>

#include "../GUI/FileManager.h"

#include "../../SyntopiaCore/Exceptions/Exception.h"
#include "../../SyntopiaCore/Logging/Logging.h"

namespace Fragmentarium {
using namespace GUI;

namespace Parser {

using namespace SyntopiaCore::Logging;

enum Provenance { FromUnknown = 0, FromMainShader = 1, FromBufferShader = 2, FromBothShaders = 3 };

enum LockTypeInner { Locked, NotLocked, NotLockable, AlwaysLocked, Unknown } ;

class LockType {
public:
    LockType() {}
    LockType(QString s) {
        fromString(s);
    }
    LockType(LockTypeInner l) : inner(l) {}
    bool operator ==(const LockTypeInner lty) {
        return inner==lty;
    }
    bool operator !=(const LockTypeInner lty) {
        return inner!=lty;
    }
    bool operator ==(const LockType &lty) {
        return inner==lty.inner;
    }
    bool operator !=(const LockType &lty) {
        return inner!=lty.inner;
    }
    QString toString() {
        if (inner == Locked) {
            return "Locked";
        } else if (inner == NotLocked) {
            return "NotLocked";
        } else if (inner == NotLockable) {
            return "NotLockable";
        } else if (inner == AlwaysLocked) {
            return "AlwaysLocked";
        } else {
            return "???";
        }
    }

    void fromString(QString s) {
        s = s.toLower();
        if (s == "locked") {
            inner = Locked;
        } else if (s == "notlocked") {
            inner = NotLocked;
        } else if (s == "notlockable") {
            inner = NotLockable;
        } else if (s == "alwayslocked") {
            inner = AlwaysLocked;
        } else {
            inner = Unknown;
        }
    }

private:
    LockTypeInner inner = NotLocked;
};

enum SliderTypeInner { Linear, Logarithmic, UnknownSliderType } ;

class SliderType {
public:
    SliderType() {}
    SliderType(QString s) {
        fromString(s);
    }
    SliderType(SliderTypeInner l) : inner(l) {}
    bool operator ==(const SliderTypeInner lty) {
        return inner==lty;
    }
    bool operator !=(const SliderTypeInner lty) {
        return inner!=lty;
    }
    bool operator ==(const SliderType &lty) {
        return inner==lty.inner;
    }
    bool operator !=(const SliderType &lty) {
        return inner!=lty.inner;
    }
    QString toString() {
        if (inner == Linear) {
            return "";
        } else if (inner == Logarithmic) {
            return "Logarithmic";
        } else {
            return "";
        }
    }

    void fromString(QString s) {
        s = s.toLower();
        if (s == "" || s == "Linear") {
            inner = Linear;
        } else if (s == "Logarithmic") {
            inner = Logarithmic;
        } else {
            inner = UnknownSliderType;
        }
    }

private:
    SliderTypeInner inner = Linear;
};

class GuiParameter {
public:
    GuiParameter(QString group, QString name, QString tooltip) : lockType(Unknown), sliderType(UnknownSliderType), provenance(FromUnknown), group(group), name(name), tooltip(tooltip) {
    };

    QString getName() {
        return name;
    }
    QString getGroup() {
        return group;
    }
    virtual QString getUniqueName() = 0;
    QString getTooltip() {
        return tooltip;
    }
    LockType getLockType() {
        return lockType;
    }
    void setLockType(LockType l) {
        lockType = l;
    }
    SliderType getSliderType() {
        return sliderType;
    }
    void setSliderType(SliderType l) {
        sliderType = l;
    }
    Provenance getProvenance() {
        return provenance;
    }
    void setProvenance(Provenance l) {
        provenance = l;
    }
    void addProvenance(Provenance l) {
        provenance = Provenance(provenance | l);
    }
    void setIsDouble(bool v) {
        wantDouble = v;
    }
    bool isDouble() {
        return wantDouble;
    }

protected:
    LockType lockType;
    SliderType sliderType;
    Provenance provenance;
    QString group;
    QString name;
    QString tooltip;
    bool wantDouble=false;
};

class SamplerParameter : public GuiParameter {
public:
    SamplerParameter(QString group, QString name,QString tooltip, QString defaultValue, QString defaultChannel="") :
        GuiParameter(group, name, tooltip), defaultValue(defaultValue), defaultChannel(defaultChannel) {}

    virtual QString getUniqueName() {
        return QString("%1:%2:%3:%4").arg(group).arg(getName());
    }
    QString getDefaultValue() {
        return defaultValue;
    }
    QString getDefaultChannelValue() {
        return defaultChannel;
    }
private:
    QString defaultValue;
    QString defaultChannel;
};

class FloatParameter : public GuiParameter {
public:
    FloatParameter(QString group, QString name,QString tooltip,  double from, double to, double defaultValue) :
        GuiParameter(group, name, tooltip), from(from), to(to), defaultValue(defaultValue) {}

    virtual QString getUniqueName() {
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(from).arg(to);
    }
    double getFrom() {
        return from;
    }
    double getTo() {
        return to;
    }
    double getDefaultValue() {
        return defaultValue;
    }
private:
    double from;
    double to;
    double defaultValue;
};

class Float2Parameter : public GuiParameter {
public:
    Float2Parameter(QString group, QString name,QString tooltip,  glm::dvec2 from, glm::dvec2 to, glm::dvec2 defaultValue) :
        GuiParameter(group, name, tooltip), from(from), to(to), defaultValue(defaultValue) {}

    virtual QString getUniqueName() {
        QString f = QString("[%1 %2]").arg(from.x).arg(from.y);
        QString t = QString("[%1 %2]").arg(to.x).arg(to.y);
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
    }
    glm::dvec2 getFrom() {
        return from;
    }
    glm::dvec2 getTo() {
        return to;
    }
    glm::dvec2 getDefaultValue() {
        return defaultValue;
    }
private:
    glm::dvec2 from;
    glm::dvec2 to;
    glm::dvec2 defaultValue;
};

class Float3Parameter : public GuiParameter {
public:
    Float3Parameter(QString group, QString name,QString tooltip,  glm::dvec3 from, glm::dvec3 to, glm::dvec3 defaultValue) :
        GuiParameter(group, name, tooltip), from(from), to(to), defaultValue(defaultValue) {}

    virtual QString getUniqueName() {
        QString f = QString("[%1 %2 %3]").arg(from.x).arg(from.y).arg(from.z);
        QString t = QString("[%1 %2 %3]").arg(to.x).arg(to.y).arg(to.z);
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
    }
    glm::dvec3 getFrom() {
        return from;
    }
    glm::dvec3 getTo() {
        return to;
    }
    glm::dvec3 getDefaultValue() {
        return defaultValue;
    }
private:
    glm::dvec3 from;
    glm::dvec3 to;
    glm::dvec3 defaultValue;
};

class Float4Parameter : public GuiParameter {
public:
    Float4Parameter(QString group, QString name,QString tooltip,  glm::dvec4 from, glm::dvec4 to, glm::dvec4 defaultValue) :
        GuiParameter(group, name, tooltip), from(from), to(to), defaultValue(defaultValue) {}

    virtual QString getUniqueName() {
        QString f = QString("[%1 %2 %3 %4]").arg(from.x).arg(from.y).arg(from.z).arg(from.w);
        QString t = QString("[%1 %2 %3 %4]").arg(to.x).arg(to.y).arg(to.z).arg(to.w);
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
    }
    glm::dvec4 getFrom() {
        return from;
    }
    glm::dvec4 getTo() {
        return to;
    }
    glm::dvec4 getDefaultValue() {
        return defaultValue;
    }
private:
    glm::dvec4 from;
    glm::dvec4 to;
    glm::dvec4 defaultValue;
};

class ColorParameter : public GuiParameter {
public:
    ColorParameter(QString group, QString name,QString tooltip, glm::dvec3 defaultValue) :
        GuiParameter(group,name, tooltip), defaultValue(defaultValue) {}

    virtual QString getUniqueName() {
        return QString("%1:%2").arg(group).arg(getName());
    }
    glm::dvec3 getDefaultValue() {
        return defaultValue;
    }
private:
    glm::dvec3 defaultValue;
};


class FloatColorParameter : public GuiParameter {
public:
    FloatColorParameter(QString group, QString name,QString tooltip, float defaultValue, float from, float to, glm::dvec3 defaultColorValue) :
        GuiParameter(group,name, tooltip), defaultValue(defaultValue), from(from), to(to), defaultColorValue(defaultColorValue) {}

    virtual QString getUniqueName() {
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(from).arg(to);
    }
    glm::dvec3 getDefaultColorValue() {
        return defaultColorValue;
    }
    double getFrom() {
        return from;
    }
    double getTo() {
        return to;
    }
    double getDefaultValue() {
        return defaultValue;
    }
private:
    double defaultValue;
    double from;
    double to;
    glm::dvec3 defaultColorValue;
};

class BoolParameter : public GuiParameter {
public:
    BoolParameter(QString group, QString name, QString tooltip,bool defaultValue) :
        GuiParameter(group, name, tooltip), defaultValue(defaultValue) {}

    virtual QString getUniqueName() {
        return QString("%1:%2").arg(group).arg(getName());
    }
    bool getDefaultValue() {
        return defaultValue;
    }
private:
    bool defaultValue;
};


class IntParameter : public GuiParameter {
public:
    IntParameter(QString group, QString name, QString tooltip, int from, int to, int defaultValue) :
        GuiParameter(group, name, tooltip), from(from), to(to), defaultValue(defaultValue) {}

    virtual QString getUniqueName() {
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(from).arg(to);
    }
    int getFrom() {
        return from;
    }
    int getTo() {
        return to;
    }
    int getDefaultValue() {
        return defaultValue;
    }
private:
    int from;
    int to;
    int defaultValue;
};

class FragmentSource {
public:
    FragmentSource();
    ~FragmentSource();

    QString getText() {
        return source.join("\n");
    }
    QStringList source;
    QStringList vertexSource;
    QList<QString> sourceFileNames;
    QList<int> lines;
    QList<int> sourceFile;

    bool hasPixelSizeUniform;
    QString camera;
    QString buffer;
    QVector<GuiParameter*> params;
    QMap<QString, QString> textures; // "Uniform name" -> "File"
    QMap<QString, QString> presets;
    QMap<QString, QMap<QString, QString> > textureParams; // foreach texturename, store parameters

    FragmentSource* bufferShaderSource;
    bool clearOnChange;
    int iterationsBetweenRedraws;
    int subframeMax;
    bool depthToAlpha;
    bool autoFocus;
};

/// The preprocessor is responsible for including files and resolve user uniform variables
    const QString sliderTypeString = "\\s*(Linear|Logarithmic)?";
    const QString lockTypeString = "\\s*(Locked|NotLocked|NotLockable)?\\s*.?$";

    // Look for patterns like 'uniform float varName; slider[0.1,1,2.0]'
    static QRegExp float4Slider ( "^\\s*uniform\\s+([d]{0,1}vec4)\\s+(\\S+)\\s*;\\s*slider\\[\\((\\S+),(\\S+),(\\S+),(\\S+)\\),\\((\\S+),(\\S+),(\\S+),(\\S+)\\),\\((\\S+),(\\S+),(\\S+),(\\S+)\\)\\]"+sliderTypeString+lockTypeString );
    static QRegExp float3Slider ( "^\\s*uniform\\s+([d]{0,1}vec3)\\s+(\\S+)\\s*;\\s*slider\\[\\((\\S+),(\\S+),(\\S+)\\),\\((\\S+),(\\S+),(\\S+)\\),\\((\\S+),(\\S+),(\\S+)\\)\\]"+sliderTypeString+lockTypeString );
    static QRegExp float2Slider ( "^\\s*uniform\\s+([d]{0,1}vec2)\\s+(\\S+)\\s*;\\s*slider\\[\\((\\S+),(\\S+)\\),\\((\\S+),(\\S+)\\),\\((\\S+),(\\S+)\\)\\]"+sliderTypeString+lockTypeString );
    static QRegExp float1Slider ( "^\\s*uniform\\s+([float|double]{1,6})\\s+(\\S+)\\s*;\\s*slider\\[(\\S+),(\\S+),(\\S+)\\]"+sliderTypeString+lockTypeString );

    static QRegExp colorChooser ( "^\\s*uniform\\s+([d]{0,1}vec3)\\s+(\\S+)\\s*;\\s*color\\[(\\S+),(\\S+),(\\S+)\\]"+lockTypeString );
    static QRegExp floatColorChooser ( "^\\s*uniform\\s+([d]{0,1}vec4)\\s+(\\S+)\\s*;\\s*color\\[(\\S+),(\\S+),(\\S+),(\\S+),(\\S+),(\\S+)\\]"+lockTypeString );

    static QRegExp intSlider ( "^\\s*uniform\\s+int\\s+(\\S+)\\s*;\\s*slider\\[(\\S+),(\\S+),(\\S+)\\]"+lockTypeString );
    static QRegExp boolChooser ( "^\\s*uniform\\s+bool\\s+(\\S+)\\s*;\\s*checkbox\\[(\\S+)\\]"+lockTypeString );
    static QRegExp main ( "^\\s*void\\s+main\\s*\\(.*$" );
    static QRegExp replace ( "^#replace\\s+\"([^\"]+)\"\\s+\"([^\"]+)\"\\s*$" ); // Look for #replace "var1" "var2"
    
    static QRegExp sampler2D (        "^\\s*uniform\\s+sampler2D\\s+(\\S+);\\s*file\\[(\\S+)\\].*$" );
    static QRegExp sampler2DChannel ( "^\\s*uniform\\s+sampler2D\\s+(\\S+);\\s*file\\[(\\S+),\\s+(\\S+)\\].*$" );
    static QRegExp isampler2D (        "^\\s*uniform\\s+isampler2D\\s+(\\S+);\\s*file\\[(\\S+)\\].*$" );
    static QRegExp isampler2DChannel ( "^\\s*uniform\\s+isampler2D\\s+(\\S+);\\s*file\\[(\\S+),\\s+(\\S+)\\].*$" );
    static QRegExp usampler2D (        "^\\s*uniform\\s+usampler2D\\s+(\\S+);\\s*file\\[(\\S+)\\].*$" );
    static QRegExp usampler2DChannel ( "^\\s*uniform\\s+usampler2D\\s+(\\S+);\\s*file\\[(\\S+),\\s+(\\S+)\\].*$" );

    static QRegExp samplerCube ( "^\\s*uniform\\s+samplerCube\\s+(\\S+)\\s*;\\s*file\\[(.*)\\].*$" );
    static QRegExp doneClear ( "^\\s*#define\\s+dontclearonchange$",Qt::CaseInsensitive );
    static QRegExp iterations ( "^\\s*#define\\s+iterationsbetweenredraws\\s*(\\d+)\\s*$",Qt::CaseInsensitive );
    static QRegExp subframeMax ( "^\\s*#define\\s+subframemax\\s*(\\d+)\\s*$",Qt::CaseInsensitive );
    static QRegExp pixelSizeCommand ( "^\\s*uniform\\s+vec2\\s+pixelSize.*$", Qt::CaseInsensitive ); // Look for 'uniform vec2 pixelSize'
    static QRegExp depthToAlpha ( "^\\s*uniform\\s+bool\\s+depthtoalpha.*$", Qt::CaseInsensitive );
    static QRegExp autoFocus ( "^\\s*uniform\\s+bool\\s+autofocus.*$", Qt::CaseInsensitive );

class Preprocessor : public QObject {
    Q_OBJECT
public:
    Preprocessor(FileManager* fileManager) : fileManager(fileManager), isCreatingAutoSave(false), inVertex(false) {}
    FragmentSource parse(QString input, QString fileName, bool moveMain);
    FragmentSource createAutosaveFragment(QString input, QString fileName);
    void parseSource(FragmentSource* fs,QString input, QString fileName, bool dontAdd);
    void parsePreset(FragmentSource *fs, QString s, int i);
    void parseSpecial(FragmentSource *fs, QString s, int i, bool moveMain );
    void parseReplacement(FragmentSource *fs, QString s, int i);
    void parseSampler2D( FragmentSource* fs, int i, QString file);
    void parseSampler2DChannel( FragmentSource* fs, int i, QString file);
    void parseSamplerCube( FragmentSource* fs, int i, QString file );
    void parseFloatColorChooser( FragmentSource* fs, int i);
    void parseFloat1Slider( FragmentSource* fs, int i);
    void parseFloat2Slider( FragmentSource* fs, int i);
    void parseFloat3Slider( FragmentSource* fs, int i);
    void parseFloat4Slider( FragmentSource* fs, int i);
    void parseColorChooser( FragmentSource* fs, int i);
    void parseIntSlider( FragmentSource* fs, int i);
    void parseBoolChooser( FragmentSource* fs, int i);
    void parseIterations( FragmentSource* fs);
    void parseMaxSubFrames( FragmentSource* fs);

    QStringList getDependencies() {
        return dependencies;
    }

private:
    FileManager* fileManager;
    QStringList dependencies;
    bool isCreatingAutoSave;
    QMap<QString, QString> replaceMap;
    bool inVertex;
    QString versionLine;

    QString lastComment;
    QString currentGroup;
};

}
}

