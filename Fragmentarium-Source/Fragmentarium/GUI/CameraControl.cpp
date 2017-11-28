#include "CameraControl.h"
#include "MainWindow.h"
#include "VariableWidget.h"
#include <cmath>

using namespace SyntopiaCore::Logging;

#include <QWheelEvent>
#include <QToolTip>
#include <QStatusBar>
#include <QMenu>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix>
#include <QMatrix4x4>

#include <QFileInfo>
#define DBOUT qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__

namespace Fragmentarium {
    namespace GUI {

        CameraControl::CameraControl() : askForRedraw(false) {
            reset(true);
        }

        void CameraControl::releaseControl() {
            keyStatus.clear();
            askForRedraw = false;
        }

        bool CameraControl::keyPressEvent(QKeyEvent* ev) {
            int key = ev->key();
            if (ev->isAutoRepeat()) {
                ev->accept();
                return false;
            }
            keyStatus[key] = (ev->type() == QEvent::KeyPress);
            return parseKeys();
        }

        bool CameraControl::keyDown(int key) {
            if (!keyStatus.contains(key)) keyStatus[key] = false;
            return keyStatus[key];
        }

        Camera3D::Camera3D(QStatusBar* statusBar) : statusBar(statusBar) {
            mouseDown = QVector3D(0,0,-1);
            reset(true);

        }

        Camera3D::~Camera3D() {
        }

        QVector<VariableWidget*> Camera3D::addWidgets(QWidget* /*group*/, QWidget* /*parent*/) {
            QVector<VariableWidget*> w;
            return w;
        }

        void Camera3D::connectWidgets(VariableEditor* ve) {
            eye = dynamic_cast<Float3Widget*>(ve->getWidgetFromName("Eye"));
            if (!eye) WARNING(QCoreApplication::translate("Camera3D","Could not find Eye interface widget"));
            target = dynamic_cast<Float3Widget*>(ve->getWidgetFromName("Target"));
            if (!target) WARNING(QCoreApplication::translate("Camera3D","Could not find Target interface widget"));
            up = dynamic_cast<Float3Widget*>(ve->getWidgetFromName("Up"));
            if (!up) WARNING(QCoreApplication::translate("Camera3D","Could not find Up interface widget"));
            fov = dynamic_cast<FloatWidget*>(ve->getWidgetFromName("FOV"));
            if (!fov) WARNING(QCoreApplication::translate("Camera3D","Could not find FOV interface widget"));
        }

        void Camera3D::printInfo() {
          INFO(QCoreApplication::translate("Camera3D","Camera: Use W/S to fly. 1/3 adjusts speed. Q/E rolls. Click on 3D window for key focus. See Help Menu for more."));
        }

        void Camera3D::reset(bool fullReset) {
            keyStatus.clear();
            if (fullReset) stepSize = 0.1f;
        }

        bool Camera3D::parseKeys() {
            if (!up || !target || !eye || !fov) return false;

            //INFO("Parse keys...");
            QVector3D direction = (target->getValue()-eye->getValue());
            QVector3D dir = direction.normalized();
            QVector3D right = QVector3D::crossProduct(direction.normalized(), up->getValue()).normalized();
            QVector3D upV = up->getValue();

            double factor = stepSize*10.0;

            bool keysDown = false;
            if (keyDown(Qt::Key_1)) {
                stepSize = stepSize*0.5;
                INFO(QCoreApplication::translate("Camera3D","Step size: %1").arg(stepSize));
                keyStatus[Qt::Key_1] = false; // only apply once
            }

            if (keyDown(Qt::Key_3)) {
                stepSize = stepSize*2.0;
                INFO(QCoreApplication::translate("Camera3D","Step size: %1").arg(stepSize));
                keyStatus[Qt::Key_3] = false; // only apply once
            }

            if (keyDown(Qt::Key_2)) {
                stepSize = stepSize*10.0;
                INFO(QCoreApplication::translate("Camera3D","Step size: %1").arg(stepSize));
                keyStatus[Qt::Key_2] = false; // only apply once
            }

            if (keyDown(Qt::Key_5)) {
              reset(true);
              INFO(QCoreApplication::translate("Camera3D","Step size: %1").arg(stepSize));
              keyStatus[Qt::Key_5] = false; // only apply once
            }
            
            if (keyDown(Qt::Key_A)) {
                QVector3D offset = -right*stepSize;
                eye->setValue(eye->getValue()+offset);
                target->setValue(target->getValue()+offset);
                keysDown = true;
            }

            if (keyDown(Qt::Key_D)) {
                QVector3D offset = right*stepSize;
                eye->setValue(eye->getValue()+offset);
                target->setValue(target->getValue()+offset);
                keysDown = true;
            }

            if (keyDown(Qt::Key_W)) {
                QVector3D offset = dir*stepSize;
                QVector3D db2 = eye->getValue()+offset;
                eye->setValue(db2);
                target->setValue(target->getValue()+offset);
                keysDown = true;
            }

            if (keyDown(Qt::Key_S)) {
                QVector3D offset = -dir*stepSize;
                eye->setValue(eye->getValue()+offset);
                target->setValue(target->getValue()+offset);
                keysDown = true;
            }

            if (keyDown(Qt::Key_R)) {
                QVector3D offset = -upV*stepSize;
                eye->setValue(eye->getValue()+offset);
                target->setValue(target->getValue()+offset);
                keysDown = true;
            }

            if (keyDown(Qt::Key_F)) {
                QVector3D offset = upV*stepSize;
                eye->setValue(eye->getValue()+offset);
                target->setValue(target->getValue()+offset);
                keysDown = true;
            }

            if (keyDown(Qt::Key_Y)) {
                QMatrix4x4 m; m.rotate(factor, upV);
                target->setValue(m*direction+eye->getValue());
                up->setValue(m*up->getValue());
                keysDown = true;
            }

            if (keyDown(Qt::Key_H)) {
                QMatrix4x4 m; m.rotate(-factor,upV);
                target->setValue(m*direction+eye->getValue());
                up->setValue(m*up->getValue());
                keysDown = true;
            }

            if (keyDown(Qt::Key_T)) {
                QMatrix4x4 m; m.rotate(factor, right);
                target->setValue(m*direction+eye->getValue());
                up->setValue(m*up->getValue());
                keysDown = true;
            }

            if (keyDown(Qt::Key_G)) {
                QMatrix4x4 m; m.rotate(-factor, right);
                target->setValue(m*direction+eye->getValue());
                up->setValue(m*up->getValue());
                keysDown = true;
            }

            if (keyDown(Qt::Key_E)) {
                QMatrix4x4 m; m.rotate(factor, dir);
                target->setValue(m*direction+eye->getValue());
                up->setValue(m*up->getValue());
                keysDown = true;
            }

            if (keyDown(Qt::Key_Q)) {
                QMatrix4x4 m; m.rotate(-factor, dir);
                target->setValue(m*direction+eye->getValue());
                up->setValue(m*up->getValue());
                keysDown = true;
            }

            if (keyDown(Qt::Key_AsciiTilde)) {
              target->setValue(eye->getValue() * -1.0);
              keyStatus[Qt::Key_AsciiTilde] = false; // only apply once
              keysDown = true;
            }
            
            askForRedraw = false;
            if (keysDown) {
              orthogonalizeUpVector();
              askForRedraw = true;
            }
            
            if(stepSize <= 1.0E-5) stepSize = 1.0E-5;
            if(stepSize >= 10.0) stepSize = 10.0;
            
            return keysDown;
        }

        void Camera3D::orthogonalizeUpVector(){
            if (!up || !target || !eye || !fov) return;
            QVector3D dir = (target->getValue()-eye->getValue()).normalized();
            QVector3D fixedUp = up->getValue()-QVector3D::dotProduct(up->getValue(), dir)*dir;
            up->setValue(fixedUp);
	}

	bool Camera3D::mouseEvent(QMouseEvent* e, int w, int h) {
          if (!up || !target || !eye || !fov) return false;
          QVector3D pos = QVector3D(e->pos().x()/(float(w)),e->pos().y()/(float(h)),0.0);
          //QVector3D direction = (target->getValue()-eye->getValue());
          //QVector3D right = QVector3D::cross(direction.normalized(), up->getValue()).normalized();
          // Store down params
          if (e->type() ==  QEvent::MouseButtonPress) {
            orthogonalizeUpVector();
            mouseDown = pos;
            upDown = up->getValue();
            targetDown = target->getValue();
            eyeDown = eye->getValue();
          } else if (e->type() ==  QEvent::MouseButtonRelease) {
            mouseDown = QVector3D(0,0,-1);
          }
          
          if (mouseDown.z()!=-1 && e->buttons()!=Qt::NoButton) {
            QVector3D dp = mouseDown-pos;
            
            double mouseSpeed = stepSize*10.0;
            QVector3D directionDown = (targetDown-eyeDown);
            QVector3D rightDown = QVector3D::crossProduct(directionDown.normalized(), upDown).normalized();
            
            if (e->buttons() == Qt::RightButton) {
              if (QApplication::keyboardModifiers() == Qt::NoModifier) {
                // Translate in screen plane
                QVector3D offset = (-upDown*dp.y()*mouseSpeed) + (rightDown*dp.x()*mouseSpeed);
                eye->setValue(eyeDown+offset);
                target->setValue(targetDown+offset);
                return true;
              } else {
                return true;
              }
            } else if (e->buttons() == (Qt::RightButton | Qt::LeftButton)  ) {
              // Zoom
              QVector3D newEye = eyeDown -directionDown*dp.x()*mouseSpeed;
              eye->setValue(newEye);
              target->setValue(directionDown +newEye);
              return true;
            } else { // Left mouse button
              
              if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
                // Rotate about origo
                QMatrix4x4 mx; mx.rotate(-dp.x()*mouseSpeed*10.0, upDown);
                QMatrix4x4 my; my.rotate(-dp.y()*mouseSpeed*10.0, rightDown);
                QVector3D oDir = (my*mx)*(-eyeDown);
                eye->setValue(-oDir);
                target->setValue( (my*mx)*directionDown-oDir);
                up->setValue((my*mx)*upDown);
              } else if (QApplication::keyboardModifiers() == Qt::ShiftModifier + Qt::AltModifier) {
                // Rotate around target thanks to M.Benesi 
                QMatrix4x4 mx; mx.rotate(-dp.x()*mouseSpeed*10.0, upDown);
                QMatrix4x4 my; my.rotate(-dp.y()*mouseSpeed*10.0, rightDown);
                QVector3D oDir = (my*mx)*(directionDown);   //was -eyeDown
                eye->setValue(targetDown-oDir);
                // target->setValue( (my*mx)*directionDown-oDir);
                up->setValue((my*mx)*upDown);
              } else if (QApplication::keyboardModifiers() == Qt::NoModifier) {
                // orient camera
                QMatrix4x4 mx; mx.rotate(-dp.x()*mouseSpeed*100.0, upDown);
                QMatrix4x4 my; my.rotate(-dp.y()*mouseSpeed*100.0, rightDown);
                target->setValue((my*mx)*directionDown+eye->getValue()); // before: eyeDown
                up->setValue((my*mx)*upDown);
              }
              
              return true;
            }
          }
          return false;
        }
 
        QVector3D Camera3D::transform(int width, int height) {
            this->height = height;
            this->width = width;

            // -- Modelview
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            return QVector3D(1.0,1.0,1.0);
        }

        bool Camera3D::wheelEvent(QWheelEvent* e) {
            double steps = e->delta()/120.0;
            if (!up || !target || !eye || !fov) return false;
            if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
                if (steps>0.0) {
                    stepSize = stepSize*2.0f;
                    INFO(QCoreApplication::translate("Camera3D","Step size: %1").arg(stepSize));

                } else if (steps<0.0) {
                    stepSize = stepSize*0.5f;
                    INFO(QCoreApplication::translate("Camera3D","Step size: %1").arg(stepSize));

                }
                if(stepSize < 1.0E-5) stepSize = 1.0E-5;
                if(stepSize > 10.0) stepSize = 10.0;
            } else {
                QVector3D direction = (target->getValue()-eye->getValue());
                QVector3D dir = direction.normalized();
                QVector3D offset = dir*stepSize*(steps);
                QVector3D db2 = eye->getValue()+offset;
                eye->setValue(db2);
                target->setValue(target->getValue()+offset);
                return true;
            }

            return false;
            
        }

        // thanks to M Benesi and FractalForums.com :D
        QVector3D Camera3D::screenTo3D(int sx, int sy, double sz) {

          QVector3D eye2 = eye->getValue(),target2 = target->getValue(), up2 = up->getValue();
          double coordX = ( double(sx)/double(height) * 2.0 - double(width) / double(height) );
          double coordY = ( double(height - sy) / double(height) * 2.0 - 1.0 );
          
          QVector3D dir2 = (target2-eye2).normalized();
          QVector3D up3 = (up2 - QVector3D::dotProduct(up2, dir2)*dir2).normalized();
          QVector3D right2 = QVector3D::crossProduct(dir2, up3).normalized();
          
          dir2 = (coordX*right2 + coordY*up3 )*fov->getValue()+dir2;   //.4 = FOV

          QVector3D ret = eye2 + dir2 / sz;
#ifndef Q_OS_WIN
          if( std::isinf(ret.x()) != 0 ) { ret.setX(1000.0); }
          if( std::isinf(ret.y()) != 0 ) { ret.setY(1000.0); }
          if( std::isinf(ret.z()) != 0 ) { ret.setZ(1000.0); }
          if( std::isnan(ret.x()) != 0 ) { ret.setX(0.00001); }
          if( std::isnan(ret.y()) != 0 ) { ret.setY(0.00001); }
          if( std::isnan(ret.z()) != 0 ) { ret.setZ(0.00001); }
#endif          
          return ret;
        }
        
        double Camera3D::StepSize(){return stepSize;}
        
        /// ----------------- Camera2D ---------------------


        Camera2D::Camera2D(QStatusBar* statusBar) : statusBar(statusBar) {
            center = 0;
            zoom = 0;
            rotation = 0;
            mouseDown = QVector3D(0,0,-1);
            reset(true);
        }

        QVector<VariableWidget*> Camera2D::addWidgets(QWidget* /*group*/, QWidget* /*parent*/) {
            QVector<VariableWidget*>w;
            return w;
        }

        void Camera2D::printInfo() {
          INFO(QCoreApplication::translate("Camera2D","Camera: Click on 2D window for key focus. See Help Menu for more."));
        }

        QVector3D Camera2D::transform(int /*width*/, int /*height*/) {
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            return QVector3D(1.0,1.0,1.0);
        }

        void Camera2D::connectWidgets(VariableEditor* ve) {
            center = dynamic_cast<Float2Widget*>(ve->getWidgetFromName("Center"));
            if (!center) WARNING(QCoreApplication::translate("Camera2D","Could not find Center interface widget"));
            zoom = dynamic_cast<FloatWidget*>(ve->getWidgetFromName("Zoom"));
            if (!zoom) WARNING(QCoreApplication::translate("Camera2D","Could not find Zoom interface widget"));
            rotation = dynamic_cast<FloatWidget*>(ve->getWidgetFromName("Rotation"));
            if (!rotation) WARNING(QCoreApplication::translate("Camera2D","Could not find Rotation interface widget"));
        }

        namespace {
            QVector3D getModelCoord(QVector3D mouseCoord, QVector3D center, double zoom, int w, int h) {
                double ar = h/((double)(w));
                QVector3D coord = (mouseCoord/zoom+center);
                coord.setX(ar*coord.x());
                return coord;
            }
        }

        bool Camera2D::parseKeys() {
            if (!center || !zoom) return false;
            QVector3D centerValue = center->getValue();
            double zoomValue = zoom->getValue();
            double rotationValue = rotation->getValue();

            double factor = pow(1.05f,(double)stepSize);
            double zFactor = 0.1/zoomValue;

            bool keysDown = false;

            // --------- step sizes ----------------------------

            if (keyDown(Qt::Key_1)) {
                stepSize = stepSize*0.5;
                INFO(QCoreApplication::translate("Camera2D","Step size: %1").arg(stepSize));
                keyStatus[Qt::Key_1] = false; // only apply once
            }

            if (keyDown(Qt::Key_3)) {
                stepSize = stepSize*2.0;
                INFO(QCoreApplication::translate("Camera2D","Step size: %1").arg(stepSize));
                keyStatus[Qt::Key_3] = false; // only apply once
            }

            if (keyDown(Qt::Key_2)) {
                stepSize = stepSize*10.0;
                INFO(QCoreApplication::translate("Camera2D","Step size: %1").arg(stepSize));
                keyStatus[Qt::Key_2] = false; // only apply once
            }

            if (keyDown(Qt::Key_X)) {
                stepSize = stepSize*0.1;
                INFO(QCoreApplication::translate("Camera2D","Step size: %1").arg(stepSize));
                keysDown = true;
                keyStatus[Qt::Key_X] = false; // only apply once
            }

            // ---------- Movement -----------------------------
            // A: move camera left (-x)
            if (keyDown(Qt::Key_A)) {
                center->setValue(centerValue + QVector3D(-zFactor * cos(rotationValue), -zFactor * sin(rotationValue), 0.0));
                keysDown = true;
            }

            // D: move camera right (+x)
            if (keyDown(Qt::Key_D)) {
                center->setValue(centerValue + QVector3D(zFactor * cos(rotationValue), zFactor * sin(rotationValue), 0.0));
                keysDown = true;
            }

            // shouldn't W and S be reversed??
            // W: move camera down (+y)
            if (keyDown(Qt::Key_W)) {
                center->setValue(centerValue + QVector3D(-zFactor * -sin(rotationValue),-zFactor * cos(rotationValue),0.0));
                keysDown = true;
            }

            // S: move camera up (-y)
            if (keyDown(Qt::Key_S)) {
                center->setValue(centerValue+QVector3D(zFactor * -sin(rotationValue), zFactor * cos(rotationValue),0.0));
                keysDown = true;
            }

            // Q: zoom in (move camera towards 2D plane)
            if (keyDown(Qt::Key_Q)) {
                zoom->setValue(zoomValue*factor);
                keysDown = true;
            }

            // E: zoom out (move camera away from 2D plane)
            if (keyDown(Qt::Key_E)) {
                zoom->setValue(zoomValue/factor);
                keysDown = true;
            }

            askForRedraw = keysDown;
            return keysDown;
        }

        bool Camera2D::mouseEvent(QMouseEvent* e, int w, int h) {
            if (!center || !zoom) return false;
            QVector3D pos = QVector3D(e->pos().x()/(0.5*double(w))-1.0,1.0-e->pos().y()/(0.5*double(h)),0.0);
            QVector3D centerValue = center->getValue();
            double zoomValue = zoom->getValue();
            double rotationValue = rotation->getValue();

            if (e->type() ==  QEvent::MouseButtonPress) {
                mouseDown = pos;
                zoomDown = zoomValue;
                centerDown = centerValue;
                rotationDown = rotationValue;
            } else if (e->type() ==  QEvent::MouseButtonRelease) {
                mouseDown = QVector3D(0,0,-1);
            }

            double mouseSpeed = 1.0;
            if (mouseDown.z()!=-1 && e->buttons()!=Qt::NoButton) {
                QVector3D dp = mouseDown-pos;
                float xtemp = dp.x();
                float ytemp = dp.y();
                dp.setX((cos(rotationDown) * xtemp) - (sin(rotationDown) * ytemp));
                dp.setY((sin(rotationDown) * xtemp) + (cos(rotationDown) * ytemp));
                if (e->buttons() == Qt::LeftButton) {
                    // move camera
                    center->setValue(centerDown + dp*mouseSpeed/zoomDown);
                } else if (e->buttons() == Qt::RightButton) {
                    // scale around mousedown coords (move camera in/out from 2D plane)
                    // Convert mouse down to model coordinates
                    QVector3D md = getModelCoord(mouseDown, centerDown, zoomDown, w,h);
                    double newZoom = zoomDown +dp.y()*(zoomDown)*mouseSpeed;
                    double z = newZoom/zoomDown;
                    center->setValue(md-(md-centerDown)/z);
                    zoom->setValue( newZoom);
                }
                return true;
            }
            return false;
        }

        void Camera2D::reset(bool fullReset) {
            keyStatus.clear();
            if (fullReset) stepSize = 1.0;
        }

        double Camera2D::StepSize(){return stepSize;}
        
        bool Camera2D::wheelEvent(QWheelEvent* e) {
            double steps = e->delta()/120.0;
            double factor = 1.15f;
            if (!zoom) return false;
            if (steps>0.0) {
                zoom->setValue(zoom->getValue()*factor);
            } else {
                zoom->setValue(zoom->getValue()/factor);
            }
            return true;
        }
    }
}


