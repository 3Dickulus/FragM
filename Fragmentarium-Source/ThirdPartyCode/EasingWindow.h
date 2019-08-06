#pragma once
/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include <QDialog>
#include <QDialogButtonBox>
#include <QGraphicsPixmapItem>
#include <QtGui>

#include "EasingAnimation.h"
#include "ui_EasingDialog.h"

namespace Fragmentarium
{
namespace GUI
{

class PixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY ( QPointF pos READ pos WRITE setPos )
public:
    PixmapItem ( const QPixmap &pix ) : QGraphicsPixmapItem ( pix ) {}
};

class EasingWindow : public QDialog
{
    Q_OBJECT
public:
    EasingWindow ( QWidget *parent, double min, double max, double start,
                   int animLength, int loops = 1, int pp = 0 );

    QEasingCurve curve;

    double getPeriod()
    {
        return period;
    }
    double getAmplitude()
    {
        return amplitude;
    }
    double getOvershoot()
    {
        return overshoot;
    }
    double getStartValue()
    {
        return startVal;
    }
    double getFinishValue()
    {
        return finishVal;
    }
    int getFirstFrame()
    {
        return firstFrame;
    }
    int getLastFrame()
    {
        return lastFrame;
    }
    int getDuration()
    {
        return ( lastFrame - firstFrame );
    }
    int getLoopCount()
    {
        return loopCount;
    }
    int getPong()
    {
        return pong;
    }

    void setStartValue ( double v )
    {
        m_ui.startSpinBox->setValue ( v );
        startChanged ( v );
    }
    void setFinishValue ( double v )
    {
        m_ui.finishSpinBox->setValue ( v );
        finishChanged ( v );
    }
    void setFirstFrame ( int f )
    {
        m_ui.firstframeSpinBox->setValue ( f );
        firstChanged ( f );
    }
    void setLastFrame ( int f )
    {
        m_ui.lastframeSpinBox->setValue ( f );
        lastChanged ( f );
    }
    void setCurve ( int c )
    {
        m_ui.easingCurvePicker->setCurrentRow ( c );
        curveChanged ( c );
    }
    void setAmplitude ( double a )
    {
        m_ui.amplitudeSpinBox->setValue ( a );
        amplitudeChanged ( a );
    }
    void setPeriod ( double p )
    {
        m_ui.periodSpinBox->setValue ( p );
        periodChanged ( p );
    }
    void setOvershoot ( double o )
    {
        m_ui.overshootSpinBox->setValue ( o );
        overshootChanged ( o );
    }
    void setAnimLength ( int len );
    void setValueRange ( QPair<double, double> vA );
    void setLoopCount ( int loops )
    {
        m_ui.loopSpinBox->setValue ( loops );
    }
    void setPong ( int pp )
    {
        pong = pp;
    }

    void setTitle ( QString t )
    {
        setWindowTitle ( t );
    }

private slots:
    void curveChanged ( int row );
    void pathChanged ( int index );
    void periodChanged ( double );
    void amplitudeChanged ( double );
    void overshootChanged ( double );
    void startChanged ( double v );
    void finishChanged ( double v );
    void firstChanged ( int f );
    void loopChanged ( int loop );
    void lastChanged ( int f );
    void directionChange();
    void frameRangeSliderChanged ( QPair<int, int> fR );
    void valueRangeSliderChanged ( QPair<double, double> vA );

private:
    void createCurveIcons();
    void startAnimation();

    Ui::EasingDialog m_ui;
    QGraphicsScene m_scene;
    PixmapItem *m_item;
    Animation *m_anim;
    QSize m_iconSize;

    double startVal = 0.0;
    double finishVal = 0.0;
    int firstFrame = 0;
    int lastFrame = 0;
    double period = 0.0;
    double amplitude = 0.0;
    double overshoot = 0.0;
    int animFrames = 0;
    int loopCount = 0;
    int pong = 0;
};
} // namespace GUI
} // namespace Fragmentarium
