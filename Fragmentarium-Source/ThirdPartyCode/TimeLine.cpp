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

#include "../../SyntopiaCore/Misc/Misc.h"
#include "MainWindow.h"
#include "TimeLine.h"
#include <QDir>
#include <QFileInfo>
#include <QImageWriter>
#include <QLayout>
#include <QSettings>

namespace Fragmentarium
{
namespace GUI
{

TimeLineDialog::TimeLineDialog(MainWindow *parent, QMap<int, Fragmentarium::GUI::KeyFrameInfo *> keyframemap) : mainWin(parent), keyframeMap(keyframemap)
{

    m_ui.setupUi(this);

    scene = new QGraphicsScene(this);
    scene->setPalette(mainWin->palette());

    m_ui.graphicsView->setScene(scene);

    greenBrush = QBrush(Qt::green);
    grayBrush = QBrush(Qt::lightGray);
    redBrush = QBrush(Qt::red);
    textColor = mainWin->palette().color(QPalette::ButtonText);
    outlinePen = QColor(Qt::gray);
    outlinePen.setWidth(1);
    
    readTimeLineSettings();

    renderKeyframeMap();

    renderEasingCurveMap();

    sceneMaxRect = scene->sceneRect();

    resize(sceneMaxRect.width() + 100, sceneMaxRect.height() + 50);

    connect(this, SIGNAL(accepted()), this, SLOT(saveTimeLineSettings()));
    connect(this, SIGNAL(rejected()), this, SLOT(restoreTimeLineSettings()));

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

    // for keyframes
    connect(scene, SIGNAL(selectionChanged()), this, SLOT(selectionChange()));
    // for easing curves
    connect(scene, SIGNAL(changed(QList<QRectF>)), this, SLOT(itemChange(QList<QRectF>)));
}

TimeLineDialog::~TimeLineDialog() = default;

void TimeLineDialog::addSceneText(QString text, qreal x, qreal y, qreal size)
{
        QGraphicsTextItem* msg = new QGraphicsTextItem();
        msg->setFont(QFont("Arial", size));
        msg->setPlainText(text);
        msg->setPos(x, y);
        msg->setDefaultTextColor(textColor);
        scene->addItem(msg);
}

// feeds this timeline editor all the required info, keyframes, easingcurves, total frames, FPS
void TimeLineDialog::readTimeLineSettings()
{

    renderFPS = mainWin->renderFPS;
    frames = mainWin->getTimeMax() * renderFPS;

    if (mainWin->getVariableEditor()->hasKeyFrames()) {
        // get a copy the currently listed keyframes from the variable editor
        // (presets)
        keyframeCount = mainWin->getVariableEditor()->getKeyFrameCount();
//         // make a map
//         for (int x = 0; x < keyframeCount; x++) {
//             // only gets the settings for a keyframe so we add the name
//             QString pName;
//             pName = QString("Keyframe.%1").arg(x + 1);
//             QStringList p;
//             p << pName << mainWin->getVariableEditor()->getPresetByName(pName);
//             keyframeMap.insert(x, new KeyFrameInfo(p));
//         }
        // initialize an empty map
        for (int x = 0; x < frames; x++) {
            rectMap[x];
        }

    } else {
        QGraphicsTextItem* msg = new QGraphicsTextItem();
        msg->setFont(QFont("Arial", 10));
        msg->setPlainText(tr(R"(No Keyframes. Use "F8" key while in "Progressive" mode.)"));
        msg->setPos(1, -30);
        msg->setDefaultTextColor(textColor);
        scene->addItem(msg);
    }

    yOff = 20;

    if (mainWin->getVariableEditor()->hasEasing()) {
        // get a copy the currently active easingcurves from the engine
        uSettings = mainWin->getEngine()->getCurveSettings();
        // make a map
        for (int i = 0; i < uSettings.count(); i++) {
            easingMap.insert(i, new EasingInfo(uSettings[i]));
            // render uniform names down the left side
            addSceneText(easingMap.value(i)->slidername, -100, (i * yOff), 6);
        }

    } else {
        QString txt = tr("No easing curves. \"Apply\" a preset that contains easing curve settings.\nOr create them with \"F7\" hotkey for the selected float slider."); 
        addSceneText(txt, 1, -60, 10);
    }

    vCount = easingMap.count() == 0 ? 10 : uSettings.count();

    QString txt = QString("%1 FR / %2 FPS = %3 SEC")
              .arg(frames)
              .arg(renderFPS)
              .arg(float(frames) / float(renderFPS));
    addSceneText(txt, -100, (yOff * vCount) + 5, 6);

}

void TimeLineDialog::saveTimeLineSettings()
{

    bool changed = false;
    QString name = "";

    if (easingMap.count() != 0) {

        // loop through easing curve settings
        QMapIterator<int, EasingInfo *> em(easingMap);
        while (em.hasNext()) {
            em.next();
            //             DBOUT << em.value()->rawsettings;
            QString newSettings =
                QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12")
                .arg(em.value()->slidername)
                .arg(em.value()->typeName)
                .arg(em.value()->typeNum)
                .arg(em.value()->startVal)
                .arg(em.value()->endVal)
                .arg(int(eGroupMap.value(em.key())->x() / 3))
                .arg(int(eGroupMap.value(em.key())->x() / 3) + int(eGroupMap.value(em.key())->boundingRect().width() / 3))
                .arg(em.value()->period)
                .arg(em.value()->amplitude)
                .arg(em.value()->overshoot)
                .arg(em.value()->loops)
                .arg(em.value()->pingpong);
            //             DBOUT << newSettings;

            TextEdit *te = mainWin->getTextEdit();
            QTextCursor to = te->textCursor();
            QTextCursor tc = te->textCursor();
            tc.setPosition(0);
            te->setTextCursor(tc);
            // find the old settings
            bool found = te->find(em.value()->rawsettings);
            // replace with new settings
            if (found) {
                tc = te->textCursor();
                tc.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
                te->setTextCursor(tc);
                qApp->clipboard()->setText(newSettings);
                te->paste();
                changed = true;
            }
            // find the preset name that contains the settings
            found = te->find("#preset", QTextDocument::FindBackward);
            if (found) {
                tc = te->textCursor();
                tc.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                tc.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
                te->setTextCursor(tc);
                te->copy();

                name = qApp->clipboard()->text().split(" ").at(1);
            }
        }
    }
    if (changed) {
        mainWin->initializeFragment();
        mainWin->getVariableEditor()->setPreset(name);
        mainWin->getVariableEditor()->applyPreset();
    }
}

void TimeLineDialog::restoreTimeLineSettings()
{
    //     DBOUT;
    if (uSettings.count() != 0) {
        mainWin->getEngine()->setCurveSettings(uSettings);
    }
}

void TimeLineDialog::renderKeyframeMap()
{
    if (mainWin->getVariableEditor()->hasKeyFrames()) {
        QMapIterator<int, QGraphicsRectItem *> r(rectMap);
        int o = (frames / (keyframeMap.count() - 1)) + 1;
        while (r.hasNext()) {
            r.next();

            /// calculate keyframe time per frame
            auto kfr = int(r.key() / o);
            bool myFrame = ((kfr == double(r.key()) / double(o)) || (r.key() == frames));
            // frame position markers
            rectMap.insert(r.key(), scene->addRect((r.key() * 3) + 1, 0, -3, yOff * vCount, outlinePen, myFrame ? greenBrush : grayBrush));

            if (myFrame) {

                rectMap.value(r.key())->setToolTip(QString("Key Frame: %1\nTime: %2").arg(kfr + 1).arg(float(r.key()) / renderFPS, -2, 'g', 2, '0'));
                int xPos = (r.key() * 3);
                
                QString txt = QString("%1").arg(kfr + 1);
                addSceneText( txt, xPos - (r.key() < 10 ? 8 : 12), -20, 10);
                
                rectMap.value(r.key())->setFlag(QGraphicsItem::ItemIsSelectable);
                rectMap.value(r.key())->setZValue(1000);

            } else {
                rectMap.value(r.key())->setToolTip(QString("Frame: %1\nTime: %2").arg(r.key() + 1).arg(float(r.key()) / renderFPS, -2, 'g', 2, '0'));
            }
        }

        rectMap.insert(frames, scene->addRect((frames * 3), 0, -3, yOff * vCount, outlinePen, greenBrush));
        rectMap.value(frames)->setToolTip(QString("Key Frame: %1\nTime: %2").arg(keyframeMap.count()).arg(float(frames) / renderFPS, -2, 'g', 2, '0'));
        rectMap.value(frames)->setFlag(QGraphicsItem::ItemIsSelectable);
        
        QString txt = QString("%1").arg(keyframeMap.count());
        addSceneText(txt, (frames * 3) - 8, -20, 10);

    } else {
        rectMap.insert(0, scene->addRect(0, 0, frames * 3, yOff * vCount, outlinePen, grayBrush));
    }

    scene->update();
}

// generate a path to render the curve settings
QPainterPath TimeLineDialog::createCurve(QSize sz, int t)
{
    QEasingCurve curve((QEasingCurve::Type)t);
    qreal curveScale = sz.height();
    QPoint start(0, curveScale - (curveScale * curve.valueForProgress(0)));
    QPoint end(sz.width(), curveScale - curveScale * curve.valueForProgress(1));

    QPainterPath curvePath;
    curvePath.moveTo(start);
    for (int len = 0; len <= curveScale; len++) {
        QPoint to;
        to.setX(sz.width() * (len / curveScale) );
        to.setY(curveScale - (curveScale * curve.valueForProgress(len / curveScale)));
        curvePath.lineTo(to);
    }

    return curvePath;
}

void TimeLineDialog::renderEasingCurveMap()
{

    if (mainWin->getVariableEditor()->hasEasing()) {
        QMapIterator<int, EasingInfo *> ec(easingMap);
        while (ec.hasNext()) {
            ec.next();
            QString name = ec.value()->slidername;
            QString u = name;
            u.truncate(name.length() - 1);
            int len = (ec.value()->lastFrame - ec.value()->firstFrame) * 3;
            int yPos = (ec.key() * yOff) + 6;
            int xPos = ((len - (name.length() * 4)) * 0.5);

            // render a curve that covers the range of frames for this easingcurve
            pathMap.insert(ec.key(), scene->addPath(createCurve(QSize(len, 6), ec.value()->typeNum), outlinePen, redBrush));
            // put a name on it
            textMap.insert(ec.key(), scene->addText(name, QFont("Arial", 6)));
            textMap.value(ec.key())->setPos(xPos, -5);
            textMap.value(ec.key())->setFlag(QGraphicsItem::ItemIsFocusable, false);
            // tie the name to the curve
            eGroupMap.insert(ec.key(), new QGraphicsItemGroup(nullptr));
            eGroupMap.value(ec.key())->addToGroup(pathMap.value(ec.key()));
            eGroupMap.value(ec.key())->addToGroup(textMap.value(ec.key()));

            eGroupMap.value(ec.key())->setFlag(QGraphicsItem::ItemIsSelectable);
            eGroupMap.value(ec.key())->setFlag(QGraphicsItem::ItemIsMovable);

            scene->addItem(eGroupMap.value(ec.key()));

            eGroupMap.value(ec.key())->setPos(ec.value()->firstFrame * 3, yPos);
            int startf = eGroupMap.value(ec.key())->x() / 3;
            int endf = startf + int(len / 3);

            eGroupMap.value(ec.key())->setToolTip(QString("Fr:%1~%2 Val:%3~%4")
                                                  .arg(startf)
                                                  .arg(endf)
                                                  .arg(ec.value()->startVal)
                                                  .arg(ec.value()->endVal));
        }
    }
    scene->update();
}

void TimeLineDialog::selectionChange()
{
    //     if(eGroupMap.count()) {
    //         QMapIterator<int, QGraphicsItemGroup*> ec(eGroupMap);
    //         while (ec.hasNext()) {
    //             ec.next();
    //
    //             if(ec.value()->isSelected()) {
    //
    //                 QString name = easingMap.value(ec.key())->slidername;
    //                 DBOUT << name;
    //
    //             }
    //         }
    //     }

    if (keyframeMap.count() != 0) {
        QMapIterator<int, QGraphicsRectItem *> kf(rectMap);
        int o = (frames / (keyframeMap.count() - 1)) + 1;
        while (kf.hasNext()) {
            kf.next();

            auto kfr = int(kf.key() / o)+1;
            bool myFrame = ((kfr == (double(kf.key()) / double(o))+1) || (kf.key() == frames));
            if (myFrame) {
                kf.value()->setBrush(greenBrush);
            }

            if (kf.value()->isSelected()) {
                //                 DBOUT << keyframeMap.value(kfr)->name;
                if (kf.key() + 1 > frames) {
                    kfr = keyframeMap.count();
                } // last frame
                // apply the selected keyframe in the engine
                mainWin->applyPresetByName(keyframeMap.value(kfr)->name);
                // select the keyframe preset in the editor
                mainWin->selectPreset();
                // update the item in the rectmap with color change to show it has been selected
                kf.value()->setBrush(redBrush);
            }
        }
    }
    scene->update();
}

void TimeLineDialog::itemChange(const QList<QRectF> &region)
{
    if (eGroupMap.count() != 0 && region.count() != 0) {
        QMapIterator<int, QGraphicsItemGroup *> ec(eGroupMap);
        while (ec.hasNext()) {
            ec.next();

            if (ec.value()->isSelected()) {

                // prevent recursion
                if (ec.value()->boundingRect().x() == region.first().x()) {
                    return;
                }

                QString name = easingMap.value(ec.key())->slidername;
                QString u = name;
                u.truncate(name.length() - 1);
                int yPos = (ec.key() * yOff) + 6;

                if (ec.value()->x() < 1) {
                    ec.value()->setPos(1, yPos);
                    return;
                }

                if (ec.value()->x() > ((frames) * 3) - ec.value()->boundingRect().width()) {
                    ec.value()->setPos(((frames) * 3) - ec.value()->boundingRect().width(), yPos);
                    return;
                }

                int startf = ec.value()->x() / 3;
                int endf = startf + (easingMap.value(ec.key())->lastFrame - easingMap.value(ec.key())->firstFrame);
                ec.value()->setToolTip(QString("Fr:%1~%2 Val:%3~%4")
                                       .arg(startf)
                                       .arg(endf)
                                       .arg(easingMap.value(ec.key())->startVal)
                                       .arg(easingMap.value(ec.key())->endVal));

                ec.value()->setPos(ec.value()->x(), yPos);
            }
        }
        scene->setSceneRect(sceneMaxRect);
    }
}

// edit selected item when RMB is clicked
void TimeLineDialog::customContextMenuRequested(QPoint p)
{
    Q_UNUSED(p)
    QString found = "";
    // loop through easing curve scene items
    QMapIterator<int, QGraphicsItemGroup *> ec(eGroupMap);
    while (ec.hasNext()) {
        ec.next();

        if (ec.value()->isSelected()) {
            // get a pointer to the slider
            auto *cs = mainWin->findChild<ComboSlider *>(easingMap.value(ec.key())->slidername);
            // set the current working slider
            mainWin->getVariableEditor()->setCurrentComboSlider(cs);
            // call the easing curve settings dialog, this applies settings in the engine
            mainWin->getVariableEditor()->setEasingCurve();
            // read back the new settings from the engine
            QStringList check = mainWin->getEngine()->getCurveSettings();
            int count = check.count();
            for (int i = 0; i < count; i++) {
                if (check.at(i).startsWith(cs->objectName())) {
                    found = check.at(i);
                }
            }
            // replace old easingMap item with new one if the new one is different
            if (!found.isEmpty() && (found != easingMap.value(ec.key())->rawsettings)) {
                easingMap.insert(ec.key(), new EasingInfo(found));
            } else {
                found = "";
            }
        }
    }

    // cleanup the scene and redraw the curve settings
    if (!found.isEmpty()) {
        // remove items from the list
        for (int i = 0; i < eGroupMap.count(); i++) {
            scene->removeItem(eGroupMap.value(i));
        }
        // make sure it's clean
        eGroupMap.clear();
        // redraw the easing curve map
        renderEasingCurveMap();
    }
}

void TimeLineDialog::mousePressEvent(QMouseEvent *ev)
{

    if (ev->buttons() == Qt::RightButton && keyframeMap.count() != 0) {
        int o = (frames / (keyframeMap.count() - 1)) + 1;
        QMapIterator<int, QGraphicsRectItem *> kf(rectMap);
        while (kf.hasNext()) {
            kf.next();

            /// calculate keyframe time per frame
            auto kfr = int(kf.key() / o);
            bool myFrame = ((kfr == double(kf.key()) / double(o)) || (kf.key() == frames));
            if (myFrame) {
                if (kf.value()->isUnderMouse()) {
                    kf.value()->setSelected(true);
                    // fudge to get the last one when total frames does not divide nice by keyframe count
                    if (kf.key() + 1 > frames) {
                        kfr = keyframeMap.count() - 1;
                    }
                    // get values from our keframe map
                    glm::dvec3 e = keyframeMap.value(kfr)->eye;
                    glm::dvec3 t = keyframeMap.value(kfr)->target;
                    glm::dvec3 u = keyframeMap.value(kfr)->up;
                    // put them in a stext string for display
                    QString dlist = QString("");
                    dlist += QString("EYE:\t X %1 Y %2 Z %3 \n")
                             .arg(e.x)
                             .arg(e.y)
                             .arg(e.z);
                    dlist += QString("TARGET:\t X %1 Y %2 Z %3 \n")
                             .arg(t.x)
                             .arg(t.y)
                             .arg(t.z);
                    dlist += QString("UP:\t X %1 Y %2 Z %3 \n")
                             .arg(u.x)
                             .arg(u.y)
                             .arg(u.z);
                    // fudge for last frame ?
                    if (kf.key() + 1 > frames) {
                        dlist += QString("\nLast Frame %1").arg(frames);
                    } else {
                        dlist += QString("\nBegin %1 End %2")
                                 .arg(kf.key() + 1)
                                 .arg(kf.key() + o > frames ? frames : kf.key() + o);
                    }

                    // show the info to the user
                    QMessageBox::information(this, keyframeMap.value(kfr)->name, dlist);
                    break;
                }
            }
        }
    }
}

} // namespace GUI
} // namespace Fragmentarium
