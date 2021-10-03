// Created for Fragmentarium by FractalForums.org user 3Dickulus 21/10/02
// Based on ggr2glsl code by FractalForums.org user patched 21/06/06
// https://fractalforums.org/index.php?action=downloads;sa=view;down=20
// https://github.com/ErikPrantare/ggr2glsl
// This file is covered by GNU PUBLIC LICENSE Version 3

// TODO: the ggr file is entirely ascii
// should not do any conversion to int or double etc.
// this will preserve values, no rounding, and make it faster

#include <QFile>
#include <cstdlib>
#include <QTextStream>
#include <QDebug>

#include "ggr2glsl.h"

Ggr2Glsl::Ggr2Glsl(QString filename) : fileName(filename)
{
    if( !fileName.isNull() && !fileName.isEmpty() )
        readFile(fileName);
}

bool Ggr2Glsl::readFile(QString filename)
{
    loadingSucceded = false;
    if (!fileName.isNull() && !filename.isEmpty()) {        // we have a filename
        QFile file(filename);
        if (file.open(QFile::ReadOnly | QFile::Text)) {     // the file exists and it's readable
            QTextStream in(&file);
            QString line = in.readLine();
            if(line.contains("GIMP Gradient")) {            // the file has gimp gradient identifier
                line = in.readLine();
                if(!line.isNull()) {                        // process gradient name line
                    gradientName = line.remove("Name: ");
                    if(gradientName.length()!=0) {          // check for segment count line
                        line = in.readLine();
                        if(!line.isNull()) {                // found segment count line
                            bool ok = false;
                            numSegments = line.toInt(&ok);
                            if(ok) {                        // numSegments successfully converted to integer value
                                int segCount = 0;
                                while( !in.atEnd() ) {      // process segment data
                                    QStringList t = in.readLine().split(" ");
                                    Segment s;
                                    if(t.count() > 12) {     // found enough to make a segment
                                        s.left = t[0].toFloat(&ok);
                                        if(!ok) break;
                                        s.middle = t[1].toFloat(&ok);
                                        if(!ok) break;
                                        s.right = t[2].toFloat(&ok);
                                        if(!ok) break;
                                        s.leftColor.setRedF(t[3].toFloat(&ok));
                                        if(!ok) break;
                                        s.leftColor.setGreenF(t[4].toFloat(&ok));
                                        if(!ok) break;
                                        s.leftColor.setBlueF(t[5].toFloat(&ok));
                                        if(!ok) break;
                                        s.leftColor.setAlphaF(t[6].toFloat(&ok));
                                        if(!ok) break;
                                        s.rightColor.setRedF(t[7].toFloat(&ok));
                                        if(!ok) break;
                                        s.rightColor.setGreenF(t[8].toFloat(&ok));
                                        if(!ok) break;
                                        s.rightColor.setBlueF(t[9].toFloat(&ok));
                                        if(!ok) break;
                                        s.rightColor.setAlphaF(t[10].toFloat(&ok));
                                        if(!ok) break;
                                        s.blendType = t[11].toInt(&ok);
                                        if(!ok) break;
                                        s.colorType = t[12].toInt(&ok);
                                        if(!ok) break;

                                        segments.add(s);
                                        ++segCount;
                                    }
                                }
                                // check if the number of segments found is the same as the count specified
                                if(ok && segCount == numSegments) {
                                    qDebug() << "Loaded file: " << filename;
                                    loadingSucceded = true;
                                } else qDebug() << "Segment count mismatch: " << "stated:" << numSegments << " vs found:" << segCount << " in file " << filename;
                            } else qDebug() << "Gradient number of segments error.";
                        } else qDebug() << "Cannot read Gradient number of segments.";
                    } else qDebug() << "Gradient Name error.";
                } else qDebug() << "Cannot read Gradient Name.";
            } else qDebug() << "GIMP Gradient identifier not found.";
        } else qDebug() << "Cannot read file " << filename << ": " << file.errorString();
    } else qDebug() << "No filename!";

    // return true if file has been loaded and setup for processing successfully
    return loadingSucceded;
}

template<typename T>
QString Ggr2Glsl::printJoined (
    T begin,
    T const end,
    QString const& delim )
{
    if ( begin == end ) {
        return "";
    }
    QString out;
    QVariant v = *begin;
    QColor c = v.value<QColor>();
    if ( c.isValid() ) {
        double r,g,b,a;
        c.getRgbF(&r,&g,&b,&a);
        out = QString ( "%1,%2,%3,%4" ).arg (r,0,'f',6,'0').arg (g,0,'f',6,'0').arg (b,0,'f',6,'0').arg (a,0,'f',6,'0');
    } else {
        out = (v.userType() == QMetaType::Int) ? v.toString() : QString("%1").arg(v.toFloat(),0,'f',6,'0');
    }

    while ( ++begin != end ) {
        v = *begin;
        c = v.value<QColor>();
        if ( c.isValid() ) {
            double r,g,b,a;
            c.getRgbF(&r,&g,&b,&a);
            out += delim + QString ( "%1,%2,%3,%4" ).arg (r,0,'f',6,'0').arg (g,0,'f',6,'0').arg (b,0,'f',6,'0').arg (a,0,'f',6,'0');
        } else {
            QString t = (v.userType() == QMetaType::Int) ? v.toString() : QString("%1").arg(v.toFloat(),0,'f',6,'0');
            t.truncate(8);
            out += delim + t;
        }
    }
    return out;
}


QString Ggr2Glsl::printIntData(QVector<int> data, QString name, QString type) {

    QString out = QString("    const %1 %2 [%3] = %1[%3](\n").arg(type).arg(name).arg(numSegments);
    out += "        " + printJoined(data.begin(), data.end(), ", ");
    out += "\n    );\n";
    return out;
}
QString Ggr2Glsl::printColorData(QVector<QColor> data, QString name, QString type) {
            
    QString out = QString("    const %1 %2 [%3] = %1[%3](\n").arg(type).arg(name).arg(numSegments);
    out += "        vec4(" + printJoined(data.begin(), data.end(), "),\n        vec4(");
    out += ")\n    );\n";
    return out;
}
QString Ggr2Glsl::printFloatData(QVector<float> data, QString name, QString type) {
            
    QString out = QString("    const %1 %2 [%3] = %1[%3](\n").arg(type).arg(name).arg(numSegments);
    out += "        " + printJoined(data.begin(), data.end(), ", ");
    out += "\n    );\n";
    return out;
}

void Ggr2Glsl::ggr2glsl(QStringList &glslText, bool fg) {
    
// set the header and group tab for sliders
    glslText << R"(#donotrun

/*
 * Generated by Fragmentarium
 * Based on ggr2glsl code by FractalForums.org user patched 21/06/06
 * https://fractalforums.org/index.php?action=downloads;sa=view;down=20
 * https://github.com/ErikPrantare/ggr2glsl
 *
 */

#group GradientColoring)";

    if(fg) {
// if we want foreground color (object) add these sliders
        glslText << R"(
#define providesColor
// some controls for Obj
// mix is orbit trap under Color tab
uniform float objColorOff; slider[-10,0.5,10]
uniform vec3 objColorRotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float objColorRotAngle; slider[-180,0,180]
uniform float objColorScale; slider[-10,1,10])";
    } else {
// if we want background color (sky) add these sliders
        glslText << R"(#define providesBackground
// some controls for Background
uniform float backColorMix; slider[0,0.5,1]
uniform float backColorOff; slider[-10,0.5,10]
uniform vec3 backColorRotVector; slider[(0,0,0),(1,1,1),(1,1,1)]
uniform float backColorRotAngle; slider[-180,0,180]
uniform float backColorScale; slider[-10,1,10])";
    }
// add required color conversion and blending functions
    glslText << R"(
float ggr2glsl_linearFactor(float pos, float mid) {
    if(pos < mid) return mix(0.0, 0.5, pos / mid);

    return mix(0.5, 1.0, (pos - mid) / (1.0 - mid));
}

vec4 ggr2glsl_hsv2rgb(vec4 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return vec4(c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y), c.a);
}

vec4 ggr2glsl_rgb2hsv(vec4 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return
        vec4(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x, c.a);
}
)";

    if(fg) { // add foreground adjustments object relative
        glslText << R"(
	vec3 baseColor(vec3 point, vec3 normal){
	// init orientation
	normal *= rotationMatrix3(objColorRotVector, objColorRotAngle);
	normal *= objColorScale;
	float value = clamp((normal.y+objColorOff)*.5, 0.0, 1.0);
)";
} else {
// add background adjustments world relative
    glslText << R"(vec3 backgroundColor( vec3 dir ) {
	// init orientation
	dir *= rotationMatrix3(backColorRotVector, backColorRotAngle);
	float value = clamp((dir.y/backColorScale)+backColorOff, 0.0, 1.0);
)";
    }
// generate the segment data arrays for our gradient
    glslText << printFloatData(segments.left, "left", "float");
    glslText << printFloatData(segments.middle, "middle", "float");
    glslText << printFloatData(segments.right, "right", "float");
    glslText << printColorData(segments.leftColor, "leftColor", "vec4");
    glslText << printColorData(segments.rightColor, "rightColor", "vec4");
    glslText << printIntData(segments.blendType, "blendType", "int");
    glslText << printIntData(segments.colorType, "colorType", "int");

    glslText
        << "    int first = 0;"
        << QString("    int last = %1;").arg(segments.count()-1);

// add the blend type, color type and conversion/mixing code
    glslText <<
R"(
    while(first != last) {
        int i = (first+last) / 2;
        if(value <= right[i]) {
            last = i;
        }
        else {
            first = i + 1;
        }
    }

    int index = first;

    float spread = right[index] - left[index];
    float mid = (middle[index] - left[index]) / spread;
    float pos = (value - left[index]) / spread;
    
    const float pi = 3.14159;
    float factor;
    switch(blendType[index]) {
        //linear
        case 0: {
            factor = ggr2glsl_linearFactor(pos, mid);
        }   break;

        //curved
        case 1: {
            factor = exp(-log2(pos) / log2(mid));
        }   break;

        //sinusoidal
        case 2: {
            factor = 0.5 * (sin((pi * 0.5) + pi * ggr2glsl_linearFactor(pos, mid)) + 1.0);
        }   break;

        //sherical (increasing)
        case 3: {
            float newPos = ggr2glsl_linearFactor(pos, mid);
            factor = sqrt(1.0 - newPos * newPos);
        }   break;

        //sherical (decreasing)
        case 4: {
            float newPos = ggr2glsl_linearFactor(pos, mid);
            factor = 1.0 - sqrt(1.0 - newPos * newPos);
        }   break;

        //step
        case 5: {
            factor = float(pos <= mid);
        }   break;
    }

    vec4 a;

    switch(colorType[index]) {
        //rgb
        case 0: {
            vec4 lGammaCorrected = pow(leftColor[index], vec4(1.0/2.2));
            vec4 rGammaCorrected = pow(rightColor[index], vec4(1.0/2.2));
            a = pow(mix(lGammaCorrected, rGammaCorrected, factor), vec4(2.2));
        }   break;

        //hsv ccw
        case 1: {
            vec4 leftHsv = ggr2glsl_rgb2hsv(leftColor[index]);
            vec4 rightHsv = ggr2glsl_rgb2hsv(rightColor[index]);
            
            vec4 res = leftHsv;
            res.yzw = mix(leftHsv.yzw, rightHsv.yzw, factor);
            
            if(leftHsv.x < rightHsv.x) {
                res.x += factor * (rightHsv.x - leftHsv.x);
            }
            else {
                res.x += factor * (1.0 - (leftHsv.x - rightHsv.x));
            }

            if(res.x > 1.0) {
                res.x -= 1.0;
            }

            a = ggr2glsl_hsv2rgb(mix(leftHsv, rightHsv, factor));
        }   break;

        //hsv cw
        case 2: {
            vec4 leftHsv = ggr2glsl_rgb2hsv(leftColor[index]);
            vec4 rightHsv = ggr2glsl_rgb2hsv(rightColor[index]);
            
            vec4 res = leftHsv;
            res.yzw = mix(leftHsv.yzw, rightHsv.yzw, factor);
            
            if(rightHsv.x < leftHsv.x) {
                res.x -= factor * (leftHsv.x - rightHsv.x);
            }
            else {
                res.x -= factor * (1.0 - (rightHsv.x - leftHsv.x));
            }

            if(res.x < 0.0) {
                res.x += 1.0;
            }

            a = ggr2glsl_hsv2rgb(mix(leftHsv, rightHsv, factor));
        }   break;
    }
)";

// this integrates the background with existing features
// GradientBackground from the Color tab controls vignette blending
// Fog requires integration too, this is based on the default raytracer

    if(!fg) {
        glslText << R"(
	a.rgb = mix(a, BackgroundColor, backColorMix);
	if(GradientBackground>0.0) { // from the Color tab
		float t = length(coord);
		a = mix(a, vec3(0), (t*GradientBackground));
	}
	// integrate OpenGL GL_EXP2 like fog as per DE-Raytracer.frag
	float f = MaxDistance;
	a.rgb = mix(a, BackgroundColor, 1.0-exp(-pow(Fog,4.0)*f*f));
)";
    }
        glslText << "   return a.rgb;\n";

    // Closing function bracket
    glslText << "}\n";

}
