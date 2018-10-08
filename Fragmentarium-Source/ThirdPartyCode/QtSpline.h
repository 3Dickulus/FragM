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

#ifndef QTSPLINE_H
#define QTSPLINE_H
#include <QtOpenGL>
#include <QObject>
#include <QColor>
#include <QVector3D>

#include <QDebug>

namespace Fragmentarium {
namespace GUI {

struct Geometry
{
    QVector<QVector3D> vertices;
    void appendVertex(const QVector3D &a);
    void loadArrays() const;
};

class Patch
{
public:
    Patch(Geometry *);
    void draw(int n = 0, int p = 0) const;
    void addVertex(const QVector3D &a);
    GLuint start;
    GLuint count;
    GLfloat pointSize;
    GLfloat color[4];
    Geometry *geom;
};

class QtSpline : public QObject
{
public:
    QtSpline(QOpenGLWidget* parent, int nctrl = 0, int nsegs = 0, QVector3D* cv = 0);
    ~QtSpline();

    void setSplineColor(QColor c) const;
    void setControlColor(QColor c) const;
    void drawControlPoints(int n = 0) const;
    void drawSplinePoints() const;

    QVector3D getControlPoint(int n);
    QVector3D getSplinePoint(int n);
    void setControlPoint(int n, QVector3D* p);
    void recalc(int nc, int ns, QVector3D *cv);
private:
    void buildGeometry(int d, int s, QVector3D* cv);
    QOpenGLWidget *prnt;
    Geometry *geom;
    QList<Patch *> parts;
    int num_c, num_s;
};
}

}
#endif // QTSPLINE_H
