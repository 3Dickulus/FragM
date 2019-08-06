/*
 * TimeLineDialog : gui representation of easingcurves for Fragmentarium
 * Copyright (C) 2016  R P <digilanti@hotmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TIMELINE_H
#define TIMELINE_H

#include <QColor>
#include <QtGui>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGraphicsEllipseItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsItemGroup>
#include <QtWidgets/QGraphicsRectItem>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsTextItem>
#include <QtWidgets/QGraphicsView>

#include "TextEdit.h"
#include "ui_TimeLineDialog.h"

class MainWindow;

namespace Fragmentarium
{
namespace GUI
{

class TimeLineDialog : public QDialog
{
    Q_OBJECT
public:
    TimeLineDialog ( Fragmentarium::GUI::MainWindow *parent );
    ~TimeLineDialog();
public slots:

protected slots:
    void customContextMenuRequested ( QPoint p );
    void itemChange ( const QList<QRectF> &region );
    virtual void mousePressEvent ( QMouseEvent *ev );

private slots:
    void selectionChange();
    void readTimeLineSettings();
    void saveTimeLineSettings();
    void restoreTimeLineSettings();
    void renderKeyframeMap();
    void renderEasingCurveMap();
    QPainterPath createCurve ( QSize sz, int t );

private:
    Ui::TimeLineDialog m_ui;
    MainWindow *mainWin;
    QBrush greenBrush;
    QBrush grayBrush;
    QBrush redBrush;
    QPen outlinePen;
    int frames;
    int keyframeCount;
    QStringList uSettings;
    int vCount;
    int yOff;
    int renderFPS;
    // TimeLineScene
    QGraphicsScene *scene;

    QMap<int, QGraphicsTextItem *> textMap;    // easingcurve labels
    QMap<int, QGraphicsRectItem *> rectMap;    // keyframe markers
    QMap<int, QGraphicsPathItem *> pathMap;    // easingcurve path representation
    QMap<int, QGraphicsItemGroup *> eGroupMap; // easingcurve group

    QMap<int, KeyFrameInfo *> keyframeMap;
    QMap<int, EasingInfo *> easingMap;
    QRectF sceneMaxRect;
};

} // namespace GUI
} // namespace Fragmentarium

#endif
