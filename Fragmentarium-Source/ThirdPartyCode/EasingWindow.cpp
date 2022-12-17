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
#include <QButtonGroup>

#include "EasingWindow.h"

namespace Fragmentarium
{
namespace GUI
{
#define DBOUT qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__

EasingWindow::EasingWindow(QWidget *parent, double min, double max, double start, int animLength, int loops, int pp)
    : QDialog(parent), m_iconSize(64, 64)
{
    animFrames = animLength;
    m_ui.setupUi(this);
    m_ui.easingCurvePicker->setIconSize(m_iconSize);
    m_ui.easingCurvePicker->setMinimumHeight(m_iconSize.height() + 50);
    auto *buttonGroup = this->findChild<QButtonGroup *>(); // ### workaround for uic in 4.4
    buttonGroup->setId(m_ui.lineRadio, 0);
    buttonGroup->setId(m_ui.circleRadio, 1);
    buttonGroup->setExclusive(true);

    connect(m_ui.easingCurvePicker, SIGNAL(currentRowChanged(int)), this, SLOT(curveChanged(int)));
    connect(m_ui.buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(pathChanged(int)));
    connect(m_ui.periodSpinBox, SIGNAL(valueChanged(double)), this, SLOT(periodChanged(double)));
    connect(m_ui.amplitudeSpinBox, SIGNAL(valueChanged(double)), this, SLOT(amplitudeChanged(double)));
    connect(m_ui.overshootSpinBox, SIGNAL(valueChanged(double)), this, SLOT(overshootChanged(double)));
    createCurveIcons();

    m_ui.startSpinBox->setMinimum(min);
    m_ui.startSpinBox->setMaximum(max);
    m_ui.startSpinBox->setSingleStep((max - min) / 100.0);
    m_ui.finishSpinBox->setMinimum(min);
    m_ui.finishSpinBox->setMaximum(max);
    m_ui.finishSpinBox->setSingleStep((max - min) / 100.0);
    connect(m_ui.startSpinBox, SIGNAL(valueChanged(double)), this, SLOT(startChanged(double)));
    connect(m_ui.finishSpinBox, SIGNAL(valueChanged(double)), this, SLOT(finishChanged(double)));
    m_ui.minvallabel->setText(QString("%1").arg(min));
    m_ui.maxvallabel->setText(QString("%1").arg(max));

    m_ui.firstframeSpinBox->setMinimum(1);
    m_ui.lastframeSpinBox->setMinimum(1);
    m_ui.firstframeSpinBox->setMaximum(animLength - 1);
    m_ui.lastframeSpinBox->setMaximum(animLength);
    connect(m_ui.firstframeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(firstChanged(int)));
    connect(m_ui.lastframeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(lastChanged(int)));

    connect(m_ui.loopSpinBox, SIGNAL(valueChanged(int)), this, SLOT(loopChanged(int)));

    QPixmap pix(QLatin1String(":/Misc/icon.jpg"));
    m_item = new PixmapItem(pix);
    m_scene.addItem(m_item);
    m_ui.graphicsView->setScene(&m_scene);

    m_anim = new Animation(m_item, "pos");
    m_anim->setEasingCurve(QEasingCurve::Linear);
    m_ui.easingCurvePicker->setCurrentRow(int(QEasingCurve::Linear));

    connect(m_ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    m_ui.frameRangeSlider->setCutoffRange(QPair<int, int>(1, animLength + 1));
    m_ui.valueRangeSlider->setCutoffRange(QPair<double, double>(min, max));
    m_ui.loopSpinBox->setValue(loops);
    m_ui.pongCheckBox->setChecked(pp != 0);

    if (start != __DBL_MAX__) { // set defaults
        QEasingCurve dummy;
        m_ui.periodSpinBox->setValue(dummy.period());
        m_ui.amplitudeSpinBox->setValue(dummy.amplitude());
        m_ui.overshootSpinBox->setValue(dummy.overshoot());
        m_ui.firstframeSpinBox->setValue(1);
        m_ui.lastframeSpinBox->setValue(1);
        m_ui.startSpinBox->setValue(start);
        m_ui.finishSpinBox->setValue(start);
        startVal = start;
        finishVal = start;
        firstFrame = 1;
        lastFrame = 1;
        pong = 0;
        m_ui.frameRangeSlider->setRange(QPair<int, int>(1, 1));
        m_ui.valueRangeSlider->setRange(QPair<double, double>(start, start));
        m_ui.loopSpinBox->setValue(1);
        m_ui.pongCheckBox->setChecked(false);
    }

    connect(m_ui.frameRangeSlider, SIGNAL(rangeChanged(QPair<int, int>)), this, SLOT(frameRangeSliderChanged(QPair<int, int>)));
    connect(m_ui.valueRangeSlider, SIGNAL(rangeChanged(QPair<double, double>)), this, SLOT(valueRangeSliderChanged(QPair<double, double>)));

    pathChanged(0);

    connect(m_anim, SIGNAL(finished()), this, SLOT(directionChange()));

    startAnimation();
}

void EasingWindow::createCurveIcons()
{
    QPixmap pix(m_iconSize);
    QPainter painter(&pix);
    QLinearGradient gradient(0, 0, 0, m_iconSize.height());
    gradient.setColorAt(0.0, QColor(240, 240, 240));
    gradient.setColorAt(1.0, QColor(224, 224, 224));
    QBrush brush(gradient);
    const QMetaObject &mo = QEasingCurve::staticMetaObject;
    QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("Type"));
    for (int i = 0; i < QEasingCurve::NCurveTypes - 3; ++i) {
        painter.fillRect(QRect(QPoint(0, 0), m_iconSize), brush);
        QEasingCurve curve((QEasingCurve::Type)i);
        painter.setPen(QColor(0, 0, 255, 64));
        qreal xAxis = m_iconSize.height() / 1.5;
        qreal yAxis = m_iconSize.width() / 3;
        painter.drawLine(0, xAxis, m_iconSize.width(), xAxis);
        painter.drawLine(yAxis, 0, yAxis, m_iconSize.height());

        qreal curveScale = (qreal)m_iconSize.height() / (qreal)2;

        painter.setPen(Qt::NoPen);

        // start point
        painter.setBrush(Qt::red);
        QPoint start(yAxis, xAxis - curveScale * curve.valueForProgress(0));
        painter.drawRect(start.x() - 1, start.y() - 1, 3, 3);
        // end point
        painter.setBrush(Qt::blue);
        QPoint end(yAxis + curveScale, xAxis - curveScale * curve.valueForProgress(1));
        painter.drawRect(end.x() - 1, end.y() - 1, 3, 3);

        QPainterPath curvePath;
        curvePath.moveTo(start);
        for (qreal t = 0; t <= 1.0; t += 1.0 / curveScale) {
            QPoint to;
            to.setX(yAxis + curveScale * t);
            to.setY(xAxis - curveScale * curve.valueForProgress(t));
            curvePath.lineTo(to);
        }
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.strokePath(curvePath, QColor(32, 32, 32));
        painter.setRenderHint(QPainter::Antialiasing, false);
        auto *item = new QListWidgetItem;
        item->setIcon(QIcon(pix));
        item->setText(metaEnum.key(i));
        m_ui.easingCurvePicker->addItem(item);
    }
}

void EasingWindow::startAnimation()
{
    m_anim->setStartValue(QPointF(0, 0));
    m_anim->setEndValue(QPointF(100, 100));
    m_anim->setDuration(2000);
    m_anim->setLoopCount(1); // forever?
    m_anim->start();
}

void EasingWindow::curveChanged(int row)
{
    auto curveType = (QEasingCurve::Type)row;
    m_anim->setEasingCurve(curveType);
    m_anim->setCurrentTime(0);

    bool isElastic = curveType >= QEasingCurve::InElastic && curveType <= QEasingCurve::OutInElastic;
    bool isBounce = curveType >= QEasingCurve::InBounce && curveType <= QEasingCurve::OutInBounce;
    m_ui.periodSpinBox->setEnabled(isElastic);
    m_ui.amplitudeSpinBox->setEnabled(isElastic || isBounce);
    m_ui.overshootSpinBox->setEnabled(curveType >= QEasingCurve::InBack && curveType <= QEasingCurve::OutInBack);
    curve = m_anim->easingCurve();
}

void EasingWindow::directionChange()
{
    static bool loop = true;

    if (m_ui.pongCheckBox->isChecked()) {
        QAbstractAnimation::Direction flip = loop ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
        pong = 1;
        m_anim->setDirection(flip);
        if (flip == QAbstractAnimation::Backward) {
            m_anim->setCurrentTime(m_anim->totalDuration() - m_anim->duration());
        } else {
            m_anim->setCurrentTime(0);
        }

    } else {
        pong = 0;
    }

    loop = !loop;
    m_anim->start();
}

void EasingWindow::pathChanged(int index)
{
    m_anim->setPathType((Animation::PathType)index);
}

void EasingWindow::periodChanged(double value)
{
    curve = m_anim->easingCurve();
    curve.setPeriod(value);
    m_anim->setEasingCurve(curve);
    period = value;
}

void EasingWindow::amplitudeChanged(double value)
{
    curve = m_anim->easingCurve();
    curve.setAmplitude(value);
    m_anim->setEasingCurve(curve);
    amplitude = value;
}

void EasingWindow::overshootChanged(double value)
{
    curve = m_anim->easingCurve();
    curve.setOvershoot(value);
    m_anim->setEasingCurve(curve);
    overshoot = value;
}

void EasingWindow::startChanged(double v)
{
    startVal = v;
    m_ui.valueRangeSlider->blockSignals(true);
    m_ui.valueRangeSlider->setRange(QPair<double, double>(startVal, finishVal));
    m_ui.valueRangeSlider->blockSignals(false);
}

void EasingWindow::finishChanged(double v)
{
    finishVal = v;
    m_ui.valueRangeSlider->blockSignals(true);
    m_ui.valueRangeSlider->setRange(QPair<double, double>(startVal, finishVal));
    m_ui.valueRangeSlider->blockSignals(false);
}

void EasingWindow::firstChanged(int f)
{
    firstFrame = f;
    lastFrame = f > lastFrame ? f : lastFrame;
    m_ui.frameRangeSlider->setRange(QPair<int, int>(firstFrame, lastFrame));

    if ((lastFrame - firstFrame) > 1) {
        m_ui.loopSpinBox->setMaximum((animFrames - firstFrame) / (lastFrame - firstFrame));
        if (m_ui.loopSpinBox->value() < 1) {
            m_ui.totalLoopFrames->setText(QString("00000"));
        } else {
            loopChanged(m_ui.loopSpinBox->value());
        }
    }
}

void EasingWindow::loopChanged(int loop)
{
    loopCount = loop;
    if (loop > 0) {
        m_ui.totalLoopFrames->setText(QString("%1").arg(loop * (lastFrame - (firstFrame-1))));
    }

    if (loop != 0 && loop != 1) {
        m_ui.pinglabel->setEnabled(true);
        m_ui.pongCheckBox->setEnabled(true);
    } else {
        m_ui.pinglabel->setEnabled(false);
        m_ui.pongCheckBox->setEnabled(false);
    }
}

void EasingWindow::lastChanged(int f)
{
    lastFrame = f;
    firstFrame = f < firstFrame ? f : firstFrame;
    m_ui.frameRangeSlider->setRange(QPair<int, int>(firstFrame, lastFrame));

    if ((lastFrame - firstFrame) > 1) {
        m_ui.loopSpinBox->setMaximum((animFrames - firstFrame) / (lastFrame - firstFrame));
        if (m_ui.loopSpinBox->value() < 0) {
            m_ui.totalLoopFrames->setText(QString("00000"));
        } else {
            loopChanged(m_ui.loopSpinBox->value());
        }
    }
}

void EasingWindow::frameRangeSliderChanged(QPair<int, int> fR)
{
    setFirstFrame(fR.first);
    setLastFrame(fR.second);
}

void EasingWindow::valueRangeSliderChanged(QPair<double, double> vA)
{
    setStartValue(vA.first);
    setFinishValue(vA.second);
}

void EasingWindow::setAnimLength(int len)
{
    m_ui.frameRangeSlider->setCutoffRange(QPair<int, int>(1, len));
    frameRangeSliderChanged(QPair<int, int>(1, len));
}

void EasingWindow::setValueRange(QPair<double, double> vA)
{
    m_ui.valueRangeSlider->setCutoffRange(vA);
    valueRangeSliderChanged(vA);
}
} // namespace GUI
} // namespace Fragmentarium
