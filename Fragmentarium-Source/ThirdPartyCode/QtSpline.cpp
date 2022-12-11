/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QOpenGLWidget>
#include <qmath.h>


#include <glm/glm.hpp>

#include "QtSpline.h"

#define DBOUT qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__

namespace Fragmentarium
{
namespace GUI
{

static inline void qSetColor(GLfloat colorVec[], const QColor c)
{
    colorVec[0] = c.redF();
    colorVec[1] = c.greenF();
    colorVec[2] = c.blueF();
    colorVec[3] = c.alphaF();
}

void Geometry::appendVertex(const glm::dvec3 &a)
{
    vertices.append(a);
}

Patch::Patch(Geometry *g) : start(0), count(0), geom(g)
{
    pointSize = 1;
}

void Patch::addVertex(const glm::dvec3 &a)
{
    geom->appendVertex(a);
    count++;
}

class Vectoid
{
public:
    // No special Vectoid destructor - the parts are fetched out of this member
    // variable, and destroyed by the new owner
    QList<Patch *> parts;
};

class VectControlPoints : public Vectoid
{
public:
    VectControlPoints(Geometry *g, int num_ctrlpoints, glm::dvec3 *ctrlpoints)
    {
        auto *cp = new Patch(g);
        // control points
        for (int i = 0; i < num_ctrlpoints; ++i) {
            cp->addVertex(ctrlpoints[i]);
        }
        cp->pointSize = 6.0;

        cp->start = g->vertices.count() - num_ctrlpoints;

        parts << cp;
    };
};

class VectSpline : public Vectoid
{
public:
    VectSpline(Geometry *g, int num_ctrlpoints, int num_segments, bool looping);
public:
    void spline(const glm::dvec3 *cP, int num, double t, glm::dvec3 *v);
private:
    bool loop;
    double coefs[4][4] = {
        {-1.0, 2.0, -1.0, 0.0},
        {3.0, -5.0, 0.0, 2.0},
        {-3.0, 4.0, 1.0, 0.0},
        {1.0, -1.0, 0.0, 0.0}
    };
};

VectSpline::VectSpline(Geometry *g, int num_ctrlpoints, int num_segments, bool looping = false) : loop(looping)
{
    /// spline points
    auto *sp = new Patch(g);
    /// this bit of fudge lets the end points land on their respective
    /// controlpoints
    double enD = loop ? 1.0 / (num_segments - 1.0) :
        (1.0 / ((num_segments - 1.0) + ((num_segments - 1.0) * (1.0 / (num_ctrlpoints - 1.0)))));
        // fudge: if this loop isn't +1 then the last frame renders a blank
    for (int i = 0; i < num_segments+1; i++) {
        glm::dvec3 s(0.0, 0.0, 0.0);
        spline(g->vertices.constData(), num_ctrlpoints, enD*i, &s);
        sp->addVertex(s);
    }

    sp->pointSize = 1.0;
    sp->start = g->vertices.count() - num_segments;

    parts << sp;
}

// modified from
// http://www.iquilezles.org/www/articles/minispline/minispline.htm
void VectSpline::spline(const glm::dvec3 *cP, int num, double t, glm::dvec3 *v)
{
    // find control point
    double ki = (1.0 / num);
    int k = 0;
    while (ki * k < t) {
        k++;
    }

    // interpolant
    double kj = ki * k;
    double kk = kj - ki;
    double h = (t - kk) / (kj - kk);
    // add basis functions
    for (int i = 0; i < 4; i++) {
        int kn = k + i - 2;
        if (kn < 0) {
            loop ? kn = num - 1 : kn = 0;
        } else if (kn > (num - 1)) {
            loop ? kn -= num : kn = (num - 1);
        }
        double b = 0.5 * (((coefs[i][0] * h + coefs[i][1]) * h + coefs[i][2]) * h + coefs[i][3]);
        *(v) += b * cP[kn];
    }
}

// TODO add start frame end frame for control points
QtSpline::QtSpline(QOpenGLWidget *parent, int nctrl, int nsegs, glm::dvec3 *cv, bool loop)
    : prnt(parent), geom(new Geometry()), looping(loop)
    {
    buildGeometry(nctrl, nsegs, cv);
}

QtSpline::~QtSpline()
{
    qDeleteAll(parts);
    delete geom;
}

void QtSpline::buildGeometry(int nctrl, int nsegs, glm::dvec3 *cv)
{
    VectControlPoints ctrl(geom, nctrl, cv);
    parts << ctrl.parts;
    num_c = nctrl;
    num_s = nsegs;
    VectSpline curve(geom, nctrl, nsegs, looping);
    parts << curve.parts;
}

void QtSpline::setControlColor(QColor c) const
{
    qSetColor(parts[0]->color, c);
}

void QtSpline::setSplineColor(QColor c) const
{
    qSetColor(parts[1]->color, c);
}

glm::vec4 QtSpline::controlColor() const
{
    return glm::vec4(parts[0]->color[0],
                     parts[0]->color[1],
                     parts[0]->color[2],
                     parts[0]->color[3]);
}

glm::vec4 QtSpline::splineColor() const
{
    return glm::vec4(parts[1]->color[0],
                     parts[1]->color[1],
                     parts[1]->color[2],
                     parts[1]->color[3]);
}

glm::dvec3 QtSpline::getControlPoint( int n )
{
    return geom->vertices[n];
}

glm::dvec3 QtSpline::getSplinePoint(int n)
{
    return geom->vertices[n + num_c - 1];
}

void QtSpline::setControlPoint(int n, glm::dvec3 *p)
{

    /// new control point position
    geom->vertices[n]=*(p);
    /// this bit of fudge lets the end points land on their respective controlpoints
    double enD =  looping ? 1.0 / (num_s-1) :
     (1.0 / ((num_s+1.0) + ((num_s-1.0) * (1.0 / (parts[0]->count - 1.0)))-1));
    /// new spline curve
    for (int i = 0; i < num_s; i++) {
        glm::dvec3 s(0.0, 0.0, 0.0);
        ((VectSpline *)parts[1])->spline(geom->vertices.constData(), num_c, enD * i, &s);
        geom->vertices[i + num_c] = s;
    }
}

void QtSpline::recalc(int nc, int ns, glm::dvec3 *cv)
{
    parts.clear();
    buildGeometry(nc, ns, cv);
}
} // namespace GUI
} // namespace Fragmentarium
