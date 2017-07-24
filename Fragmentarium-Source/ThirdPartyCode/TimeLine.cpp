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

#include "MainWindow.h"
#include "TimeLine.h"
#include "../../SyntopiaCore/Misc/Misc.h"
#include <QFileInfo>
#include <QLayout>
#include <QDir>
#include <QImageWriter>
#include <QSettings>

namespace Fragmentarium {
namespace GUI {

TimeLineDialog::TimeLineDialog(MainWindow* parent) : mainWin(parent) {

    m_ui.setupUi(this);

    scene = new QGraphicsScene(this);
    m_ui.graphicsView->setScene(scene);

    greenBrush = QBrush(Qt::green);
    grayBrush = QBrush(Qt::lightGray);
    redBrush = QBrush(Qt::red);
    outlinePen = QColor(Qt::gray);
    outlinePen.setWidth(1);

    renderFPS = mainWin->renderFPS;
    frames = mainWin->getTimeMax()*renderFPS;
    keyframeCount = mainWin->getVariableEditor()->getKeyFrameCount();
    uNames = mainWin->getVariableEditor()->getWidgetNames();
    vCount = uNames.count();
    yOff = 10;

    readTimeLineSettings();

    createKeyframeMap();

    createEasingCurveMap();

    sceneMaxRect = scene->sceneRect();

    connect(scene, SIGNAL(changed(QList<QRectF>)), this, SLOT(itemChange(QList<QRectF>)));
    connect(this, SIGNAL(accepted()), this, SLOT( saveTimeLineSettings() ));
}

TimeLineDialog::~TimeLineDialog() {
}

void TimeLineDialog::readTimeLineSettings() {

    if(mainWin->getVariableEditor()->hasKeyFrames()) {
        TextEdit* te = mainWin->getTextEdit();
        QTextCursor to = te->textCursor();

        // capture the current list of keyframes from the text editor
        for(int x=0; x<keyframeCount; x++) {
            QString pName;

            pName.sprintf("#preset keyframe.%.3d", x+1);

            QTextCursor tc= te->textCursor();
            tc.setPosition(0);

            te->setTextCursor(tc);

            bool found = te->find( pName );
            if(found ) {
                tc = te->textCursor();
                tc.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
                found = te->find("#endpreset");
                if(found) {
                    tc.setPosition(te->textCursor().position()+1, QTextCursor::KeepAnchor);
                    te->setTextCursor(tc);
                } else mainWin->statusBar()->showMessage(tr("#endpreset not found!"));
            } else mainWin->statusBar()->showMessage(tr( QString(pName + " not found!").toStdString().c_str() ));

            if(found ) {
                te->copy();
                keyframeMap[x] = new KeyFrameInfo( qApp->clipboard()->text() );
            }
        }
        /// put the cursor back to it's original position
        te->setTextCursor(to);

    } else scene->addText(tr("No Keyframes?"), QFont("Arial", 20) )->setPos(1,-40);

    // render uniform names down the left side
    for( int i = 0; i<vCount; i++) {
        scene->addText( uNames[i] , QFont("Arial", 6))->setPos( -100 , (i*yOff)-(yOff*0.5) );
    }

    scene->addText( QString("%1 FR / %2 FPS = %3 SEC").arg(frames).arg(renderFPS).arg( float(frames)/float(renderFPS) ) ,
                    QFont("Arial", 6))->setPos( -100 , (yOff*vCount) +5 );

    if(mainWin->getVariableEditor()->hasEasing()) {
        // get acopy the currently active easingcurves
        QStringList cs = mainWin->getEngine()->getCurveSettings();
        // make a map
        for ( int i = 0; i < cs.count(); i++ ) {
            easingMap[i] = new EasingInfo(cs[i]);
        }

    }
    else scene->addText(tr("No Easing curves?"), QFont("Arial", 20) )->setPos(1,40);

    if(!mainWin->getVariableEditor()->hasKeyFrames() || !mainWin->getVariableEditor()->hasEasing()) {
        text = scene->addText("http://www.digilanti.org/fragmentarium", QFont("Arial", 20) );
        text->setPos(0,0);
        /// if no keyframes or easingcurves render a bit of movable text
        text->setFlag(QGraphicsItem::ItemIsMovable);
    }
}

void TimeLineDialog::saveTimeLineSettings() {

    bool changed = false;
    if(mainWin->getVariableEditor()->hasEasing())
    for ( int i = 0; i < easingMap.count(); i++ ) {

        QString newSettings = QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12")
                              .arg(easingMap[i]->slidername)
                              .arg(easingMap[i]->typeName)
                              .arg(easingMap[i]->typeNum)
                              .arg(easingMap[i]->startVal)
                              .arg(easingMap[i]->endVal)
                              .arg(int(eGroupMap[i]->x()/3))
                              .arg(int(eGroupMap[i]->x()/3) + int(eGroupMap[i]->boundingRect().width()/3))
                              .arg(easingMap[i]->period)
                              .arg(easingMap[i]->amplitude)
                              .arg(easingMap[i]->overshoot)
                              .arg(easingMap[i]->loops)
                              .arg(easingMap[i]->pingpong);

        TextEdit* te = mainWin->getTextEdit();
        QTextCursor to = te->textCursor();
        QTextCursor tc= te->textCursor();
        tc.setPosition(0);

        te->setTextCursor(tc);
        bool found = te->find( "#preset " + mainWin->getVariableEditor()->getPresetName() );
        if(found) {
            tc = te->textCursor();
            found = te->find("#endpreset");
            if(found) {
                te->setTextCursor(tc);
                found = te->find( easingMap[i]->rawsettings );
                if(found && (easingMap[i]->rawsettings != newSettings) ) {
                    qApp->clipboard()->setText(newSettings);
                    te->paste();
                    changed = true;
                }
            }
        }
        te->setTextCursor(to);
    }

    if(changed) {
        mainWin->initializeFragment();
        mainWin->getVariableEditor()->applyPreset();
    }
}

void TimeLineDialog::createKeyframeMap() {
    ///--------------render keyframe timeline
    float fudge = ((mainWin->getTimeMax()*renderFPS)/(keyframeMap.count()-1));
    int o = (frames/(keyframeMap.count()-1));

    for(int x = 0; x<frames+1; x++) {
        /// calculate keyframe time per frame
        int myFrame = int((x/o)*fudge);
        /// render frame position markers
        rectMap[x] = scene->addRect((x*3)+1, 0, 3, yOff*vCount, outlinePen, ( myFrame == x ) ? greenBrush:grayBrush);
        if( myFrame == x && mainWin->getVariableEditor()->hasKeyFrames()) {
            QStringList dlist =  keyframeMap[int(x/o)]->rawsettings.split("\n");
            QString name = dlist.at(1).split(" ").at(1);
            int xPos = (x*3)-(name.length()*5);
            name += QString(" @ %1 sec").arg( float( (myFrame/renderFPS ) ) );
            /// render the keyframe data, eye taget and up
            scene->addText( name, QFont("Arial", 10))->setPos(xPos,-60);
            scene->addText( dlist.at(3) , QFont("Arial", 6))->setPos(xPos,-45);
            scene->addText( dlist.at(4) , QFont("Arial", 6))->setPos(xPos,-35);
            scene->addText( dlist.at(5) , QFont("Arial", 6))->setPos(xPos,-25);
            scene->addText( QString("%1").arg(x) , QFont("Arial", 6))->setPos( (x*3)-3 , -15 );
        }
    }
}

QPainterPath TimeLineDialog::createCurve(QSize sz, int t)
{
        QEasingCurve curve((QEasingCurve::Type)t);
        qreal curveScale = sz.height();
        QPoint start(0, curveScale - (curveScale * curve.valueForProgress(0)));
        QPoint end(sz.width(), curveScale - curveScale * curve.valueForProgress(1));

        QPainterPath curvePath;
        curvePath.moveTo(start);
        for (qreal t = 0; t <= 1.0; t+=1.0/curveScale) {
            QPoint to;
            to.setX(sz.width() * t);
            to.setY(curveScale - (curveScale * curve.valueForProgress(t)));
            curvePath.lineTo(to);
        }
        
        return curvePath;
}

void TimeLineDialog::createEasingCurveMap() {

    if(mainWin->getVariableEditor()->hasEasing()) {
        for ( int i = 0; i < easingMap.count(); i++ ) {
            QString name = easingMap[i]->slidername;
            QString u = name;
            u.truncate(name.length()-1);
            int len = (easingMap[i]->lastFrame - easingMap[i]->firstFrame)*3;
            int yPos = (uNames.indexOf( u )*yOff)+1;
            int xPos = ((len-(name.length()*4))*0.5);

            // render a curve that covers the range of frames for this easingcurve
            pathMap[i] = scene->addPath( createCurve( QSize(len,6), easingMap[i]->typeNum ), outlinePen );
            // put a name on it
            textMap[i] = scene->addText( name , QFont("Arial", 6));
            textMap[i]->setPos( xPos , -5);
            // tie the name to the curve
            eGroupMap[i] = new QGraphicsItemGroup(0);
            eGroupMap[i]->addToGroup(pathMap[i]);
            eGroupMap[i]->addToGroup(textMap[i]);

            eGroupMap[i]->setFlag(QGraphicsItem::ItemIsMovable);
            eGroupMap[i]->setFlag(QGraphicsItem::ItemIsSelectable);

            scene->addItem(eGroupMap[i]);

            eGroupMap[i]->setPos(easingMap[i]->firstFrame*3, yPos);
            int startf = eGroupMap[i]->x()/3;
            int endf = startf + int(len/3);

            eGroupMap[i]->setToolTip(QString("Fr:%1~%2 Val:%3~%4").arg( startf ).arg( endf ).arg( easingMap[i]->startVal ).arg( easingMap[i]->endVal ));
        }
    }
}

void TimeLineDialog::itemChange(const QList< QRectF >& region)
{
    for ( int i = 0; i < easingMap.count(); i++ ) {
        if(eGroupMap[i]->isSelected()) {

            QString name = easingMap[i]->slidername;
            QString u = name;
            u.truncate(name.length()-1);
            int yPos = (uNames.indexOf( u )*yOff)+1;

            if(eGroupMap[i]->x() < 3) {
                eGroupMap[i]->setPos(3,yPos);
                return;
            }

            if(eGroupMap[i]->x() > ((frames+1)*3) - eGroupMap[i]->boundingRect().width() ) {
                eGroupMap[i]->setPos(((frames+1)*3) - eGroupMap[i]->boundingRect().width(), yPos );
                return;
            }

            int startf = eGroupMap[i]->x()/3;
            int endf = startf + (easingMap[i]->lastFrame - easingMap[i]->firstFrame);
            eGroupMap[i]->setToolTip(QString("%1,%2").arg( startf ).arg( endf ));
            
            eGroupMap[i]->setPos(eGroupMap[i]->x(), yPos);
            scene->setSceneRect(sceneMaxRect);
        }
    }
}

}
}
