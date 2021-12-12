#ifndef GGR2GLSL_H
#define GGR2GLSL_H
// Created for Fragmentarium by FractalForums.org user 3Dickulus 21/10/02
// Based on ggr2glsl code by FractalForums.org user patched 21/06/06
// https://fractalforums.org/index.php?action=downloads;sa=view;down=20
// https://github.com/ErikPrantare/ggr2glsl
// This file is covered by GNU PUBLIC LICENSE Version 3

#include <QVector>
#include <QVector4D>
#include <QObject>
#include <QColor>
#include <QGradient>

typedef enum {
  SEGMENT_RGB,
  SEGMENT_HSV_CCW,
  SEGMENT_HSV_CW
} SegmentColorType;

typedef enum {
  SEGMENT_LINEAR,
  SEGMENT_CURVED,
  SEGMENT_SINE,
  SEGMENT_SPHERE_INCREASING,
  SEGMENT_SPHERE_DECREASING,
  SEGMENT_STEPPED
} SegmentBlendType;

struct Segment {
    float left;
    float middle;
    float right;
    QColor leftColor;
    QColor rightColor;
    SegmentBlendType blendType;
    SegmentColorType colorType;
};

struct Segments {
    QVector<float> left;
    QVector<float> middle;
    QVector<float> right;
    QVector<QColor> leftColor;
    QVector<QColor> rightColor;
    QVector<SegmentBlendType> blendType;
    QVector<SegmentColorType> colorType;

    void add(Segment const& segment){
    left.push_back(segment.left);
    middle.push_back(segment.middle);
    right.push_back(segment.right);
    leftColor.push_back(segment.leftColor);
    rightColor.push_back(segment.rightColor);
    blendType.push_back(segment.blendType);
    colorType.push_back(segment.colorType);
    };

    // this should be fine as long as processing was successful
    int count() {
        return left.count();
    }
};


class Ggr2Glsl : public QObject
{
    Q_OBJECT
public:

    /// Constructor
    Ggr2Glsl(QString filename = QString());

    /// Destructor
    ~Ggr2Glsl(){};

public slots:
 
    // Read a GIMP gradient file and process into arrays
    // input: filename
    // return: status success/fail
    bool readFile(QString filename);
    
    // check status
    bool isLoaded() { return loadingSucceded; }
    
    // process the arrays
    // input: stinglist pointer to hold the generated glsl code
    // input: bool flag to generate foreground (object)
    //        or background (sky) code and sliders
    void ggr2glsl(QStringList &glslText, bool fgbg);

    QString gradName() { return gradientName; }
    int segmentCount() { return numSegments;  }
    QGradient getGradient();
    void setGradient(QGradient *g);

protected:
    template<typename T>
    QString printData(QVector<T> data, QString name, QString type, QString pre, QString delim, QString post = "");

private:
    template<typename T>
    QString printJoined( T begin, T const end, QString const& delim);

    QString fileName;
    QString gradientName;
    int numSegments;
    Segments segments;
    bool loadingSucceded;
};

#endif
