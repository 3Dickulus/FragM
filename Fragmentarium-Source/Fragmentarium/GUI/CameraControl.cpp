#include <QMenu>
#include <QStatusBar>
#include <QToolTip>
#include <QWheelEvent>
#include <QFileInfo>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "CameraControl.h"
#include "MainWindow.h"
#include "VariableWidget.h"

static glm::dvec3 operator*(const glm::dmat4 &m, const glm::dvec3 &v)
{
    return glm::dvec3(m * glm::dvec4(v, 0.0));
}

using namespace SyntopiaCore::Logging;

namespace Fragmentarium
{
namespace GUI
{

CameraControl::CameraControl() : askForRedraw(false)
{
    keyStatus.clear();
    stepSize = 1.0;
}

void CameraControl::releaseControl()
{
    keyStatus.clear();
    askForRedraw = false;
}

bool CameraControl::keyPressEvent(QKeyEvent *ev)
{
    int key = ev->key();
    if (ev->isAutoRepeat()) {
        ev->accept();
        return false;
    }
    keyStatus[key] = (ev->type() == QEvent::KeyPress);
    return parseKeys();
}

bool CameraControl::keyDown(int key)
{
    if (!keyStatus.contains(key)) {
        keyStatus[key] = false;
    }
    return keyStatus[key];
}

Camera3D::Camera3D(QStatusBar *statusBar) : statusBar(statusBar)
{

    height = 0;
    width = 0;
    stepSize = 0.0;
    statusBar = nullptr;
    eye = nullptr;
    target = nullptr;
    up = nullptr;
    fov = nullptr;
    eyeDown = glm::dvec3(0, 0, -1);
    targetDown = glm::dvec3(0, 0, -1);
    upDown = glm::dvec3(0, 0, -1);
    fovDown = 0.0;
    mouseDown = glm::dvec3(0, 0, -1);

    keyStatus.clear();
    stepSize = 0.1f;
}

Camera3D::~Camera3D() = default;

QVector<VariableWidget *> Camera3D::addWidgets(QWidget * /*group*/,
        QWidget * /*parent*/)
{
    QVector<VariableWidget *> w;
    return w;
}

void Camera3D::connectWidgets(VariableEditor *ve)
{
    eye = dynamic_cast<Float3Widget *>(ve->getWidgetFromName("Eye"));
    if (eye == nullptr) {
        WARNING(QCoreApplication::translate("Camera3D", "Could not find Eye interface widget"));
    }
    target = dynamic_cast<Float3Widget *>(ve->getWidgetFromName("Target"));
    if (target == nullptr) {
        WARNING(QCoreApplication::translate("Camera3D", "Could not find Target interface widget"));
    }
    up = dynamic_cast<Float3Widget *>(ve->getWidgetFromName("Up"));
    if (up == nullptr) {
        WARNING(QCoreApplication::translate("Camera3D", "Could not find Up interface widget"));
    }
    fov = dynamic_cast<FloatWidget *>(ve->getWidgetFromName("FOV"));
    if (fov == nullptr) {
        WARNING(QCoreApplication::translate("Camera3D", "Could not find FOV interface widget"));
    }
}

void Camera3D::printInfo()
{
    INFO(QCoreApplication::translate(
             "Camera3D", "Camera: Use W/S to fly. 1/3 adjusts speed. Q/E rolls. Click on 3D window for key focus. See Help Menu for more."));
}

void Camera3D::reset(bool fullReset)
{
    keyStatus.clear();
    if (fullReset) {
        stepSize = 0.1f;
    }
}

bool Camera3D::parseKeys()
{
    if (up == nullptr || target == nullptr || eye == nullptr || fov == nullptr) {
        return false;
    }

    //INFO("Parse keys...");
    glm::dvec3 direction = (target->getValue() - eye->getValue());
    glm::dvec3 dir = normalize(direction);
    glm::dvec3 right = normalize(cross(dir, up->getValue()));
    glm::dvec3 upV = up->getValue();

    double factor = stepSize * 10.0;

    bool keysDown = false;
    if (keyDown(Qt::Key_1)) {
        stepSize = stepSize * 0.5;
        INFO(QCoreApplication::translate("Camera3D", "Step size: %1").arg(stepSize));
        keyStatus[Qt::Key_1] = false; // only apply once
    }

    if (keyDown(Qt::Key_3)) {
        stepSize = stepSize * 2.0;
        INFO(QCoreApplication::translate("Camera3D", "Step size: %1").arg(stepSize));
        keyStatus[Qt::Key_3] = false; // only apply once
    }

    if (keyDown(Qt::Key_2)) {
        stepSize = stepSize * 10.0;
        INFO(QCoreApplication::translate("Camera3D", "Step size: %1").arg(stepSize));
        keyStatus[Qt::Key_2] = false; // only apply once
    }

    if (keyDown(Qt::Key_5)) {
        reset(true);
        INFO(QCoreApplication::translate("Camera3D", "Step size: %1").arg(stepSize));
        keyStatus[Qt::Key_5] = false; // only apply once
    }

    if (keyDown(Qt::Key_A)) {
        glm::dvec3 offset = -right * stepSize;
        eye->setValue(eye->getValue() + offset);
        target->setValue(target->getValue() + offset);
        keysDown = true;
    }

    if (keyDown(Qt::Key_D)) {
        glm::dvec3 offset = right * stepSize;
        eye->setValue(eye->getValue() + offset);
        target->setValue(target->getValue() + offset);
        keysDown = true;
    }

    if (keyDown(Qt::Key_W)) {
        glm::dvec3 offset = dir * stepSize;
        glm::dvec3 db2 = eye->getValue() + offset;
        eye->setValue(db2);
        target->setValue(target->getValue() + offset);
        keysDown = true;
    }

    if (keyDown(Qt::Key_S)) {
        glm::dvec3 offset = -dir * stepSize;
        eye->setValue(eye->getValue() + offset);
        target->setValue(target->getValue() + offset);
        keysDown = true;
    }

    if (keyDown(Qt::Key_R)) {
        glm::dvec3 offset = -upV * stepSize;
        eye->setValue(eye->getValue() + offset);
        target->setValue(target->getValue() + offset);
        keysDown = true;
    }

    if (keyDown(Qt::Key_F)) {
        glm::dvec3 offset = upV * stepSize;
        eye->setValue(eye->getValue() + offset);
        target->setValue(target->getValue() + offset);
        keysDown = true;
    }

    if (keyDown(Qt::Key_Y)) {
        glm::dmat4 m = glm::identity<glm::dmat4>();
        m = rotate(m, glm::radians(factor), upV);
        target->setValue(m * direction + eye->getValue());
        up->setValue(m * up->getValue());
        keysDown = true;
    }

    if (keyDown(Qt::Key_H)) {
        glm::dmat4 m = glm::identity<glm::dmat4>();
        m = rotate(m, glm::radians(-factor), upV);
        target->setValue(m * direction + eye->getValue());
        up->setValue(m * up->getValue());
        keysDown = true;
    }

    if (keyDown(Qt::Key_T)) {
        glm::dmat4 m = glm::identity<glm::dmat4>();
        m = rotate(m, glm::radians(factor), right);
        target->setValue(m * direction + eye->getValue());
        up->setValue(m * up->getValue());
        keysDown = true;
    }

    if (keyDown(Qt::Key_G)) {
        glm::dmat4 m = glm::identity<glm::dmat4>();
        m = rotate(m, glm::radians(-factor), right);
        target->setValue(m * direction + eye->getValue());
        up->setValue(m * up->getValue());
        keysDown = true;
    }

    if (keyDown(Qt::Key_E)) {
        glm::dmat4 m = glm::identity<glm::dmat4>();
        m = rotate(m, glm::radians(factor), dir);
        target->setValue(m * direction + eye->getValue());
        up->setValue(m * up->getValue());
        keysDown = true;
    }

    if (keyDown(Qt::Key_Q)) {
        glm::dmat4 m = glm::identity<glm::dmat4>();
        m = rotate(m, glm::radians(-factor), dir);
        target->setValue(m * direction + eye->getValue());
        up->setValue(m * up->getValue());
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

    if (stepSize <= 1.0E-5) {
        stepSize = 1.0E-5;
    }
    if (stepSize >= 10.0) {
        stepSize = 10.0;
    }

    return keysDown;
}

void Camera3D::orthogonalizeUpVector()
{
    if (up == nullptr || target == nullptr || eye == nullptr || fov == nullptr) {
        return;
    }
    glm::dvec3 dir = normalize(target->getValue() - eye->getValue());
    glm::dvec3 fixedUp = up->getValue() - dot(up->getValue(), dir) * dir;
    up->setValue(fixedUp);
}

bool Camera3D::mouseEvent(QMouseEvent *e, int w, int h)
{
    if (up == nullptr || target == nullptr || eye == nullptr || fov == nullptr) {
        return false;
    }

    glm::dvec3 pos = glm::dvec3(e->pos().x() / (float(w)), e->pos().y() / (float(h)), 0.0);

    // Store down params
    if (e->type() ==  QEvent::MouseButtonPress) {
        orthogonalizeUpVector();
        mouseDown = pos;
        upDown = up->getValue();
        targetDown = target->getValue();
        eyeDown = eye->getValue();
    } else if (e->type() ==  QEvent::MouseButtonRelease) {
        mouseDown = glm::dvec3(0, 0, -1);
    }

    if (mouseDown.z != -1 && e->buttons() != Qt::NoButton) {
        glm::dvec3 dp = mouseDown - pos;

        double mouseSpeed = stepSize * 10.0;
        glm::dvec3 directionDown = (targetDown - eyeDown);
        glm::dvec3 rightDown = normalize(cross(normalize(directionDown), upDown));

        if (e->buttons() == Qt::RightButton) {
            if (QApplication::keyboardModifiers() == Qt::NoModifier) {
                // Translate in screen plane
                glm::dvec3 offset = (-upDown * dp.y * mouseSpeed * 2.0) + (rightDown * dp.x * mouseSpeed);
                eye->setValue(eyeDown + offset);
                target->setValue(targetDown + offset);
                return true;
            }
        } else if (e->buttons() == (Qt::RightButton | Qt::LeftButton)) {
            // Zoom
            glm::dvec3 newEye = eyeDown - directionDown * dp.x * mouseSpeed;
            eye->setValue(newEye);
            target->setValue(directionDown + newEye);
            return true;
        } else { // Left mouse button

            if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
                // Rotate about origo
                glm::dmat4 mx = glm::identity<glm::dmat4>();
                mx = rotate(mx, glm::radians(-dp.x * mouseSpeed * 10.0), upDown);
                glm::dmat4 my = glm::identity<glm::dmat4>();
                my = rotate(my, glm::radians(-dp.y * mouseSpeed * 10.0), rightDown);
                glm::dvec3 oDir = (my * mx) * (-eyeDown);
                eye->setValue(-oDir);
                target->setValue((my * mx)*directionDown - oDir);
                up->setValue((my * mx)*upDown);
            } else if (QApplication::keyboardModifiers() == Qt::ShiftModifier + Qt::AltModifier) {
                // Rotate around target thanks to M.Benesi
                glm::dmat4 mx = glm::identity<glm::dmat4>();
                mx = rotate(mx, glm::radians(-dp.x * mouseSpeed * 10.0), upDown);
                glm::dmat4 my = glm::identity<glm::dmat4>();
                my = rotate(my, glm::radians(-dp.y * mouseSpeed * 10.0), rightDown);
                glm::dvec3 oDir = (my * mx) * (directionDown); //was -eyeDown
                eye->setValue(targetDown - oDir);
                up->setValue((my * mx)*upDown);
            } else if (QApplication::keyboardModifiers() == Qt::NoModifier) {
                // orient camera
                glm::dmat4 mx = glm::identity<glm::dmat4>();
                mx = rotate(mx, glm::radians(-dp.x * mouseSpeed * 100.0), upDown);
                glm::dmat4 my = glm::identity<glm::dmat4>();
                my = rotate(my, glm::radians(-dp.y * mouseSpeed * 100.0), rightDown);
                target->setValue((my * mx) * directionDown + eye->getValue()); // before: eyeDown
                up->setValue((my * mx)*upDown);
            }

            return true;
        }
    }
    return false;
}

glm::dvec3 Camera3D::transform(int width, int height)
{
    this->height = height;
    this->width = width;
    return {1.0, 1.0, 1.0};
}

bool Camera3D::wheelEvent(QWheelEvent *e)
{
    double steps = e->delta() / 120.0;
    if (up == nullptr || target == nullptr || eye == nullptr || fov == nullptr) {
        return false;
    }
    if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
        if (steps > 0.0) {
            stepSize = stepSize * 2.0f;
            INFO(QCoreApplication::translate("Camera3D", "Step size: %1").arg(stepSize));

        } else if (steps < 0.0) {
            stepSize = stepSize * 0.5f;
            INFO(QCoreApplication::translate("Camera3D", "Step size: %1").arg(stepSize));
        }
        if (stepSize < 1.0E-5) {
            stepSize = 1.0E-5;
        }
        if (stepSize > 10.0) {
            stepSize = 10.0;
        }
    } else {
        glm::dvec3 direction = (target->getValue() - eye->getValue());
        glm::dvec3 dir = normalize(direction);
        glm::dvec3 offset = dir * stepSize * (steps);
        eye->setValue(eye->getValue() + offset);
        target->setValue(target->getValue() + offset);
        return true;
    }

    return false;

}

// thanks to M Benesi and FractalForums.com :D
glm::dvec3 Camera3D::screenTo3D(int sx, int sy, double sz)
{
    if(eye == nullptr || target == nullptr || up == nullptr) return glm::dvec3(0,0,0);
    
    glm::dvec3 eye2 = eye->getValue(), target2 = target->getValue(),
              up2 = up->getValue();
    double coordX =
        (double(sx) / double(height) * 2.0 - double(width) / double(height));
    double coordY = (double(height - sy) / double(height) * 2.0 - 1.0);

    glm::dvec3 dir2 = normalize(target2 - eye2);
    glm::dvec3 up3 = normalize(up2 - dot(up2, dir2) * dir2);
    glm::dvec3 right2 = normalize(cross(dir2, up3));

    dir2 = (coordX * right2 + coordY * up3) * fov->getValue() + dir2; //.4 = FOV

    glm::dvec3 ret = eye2 + dir2 / sz;
#ifndef Q_OS_WIN
    if (std::isinf(ret.x)) {
        ret.x=(1000.0);
    }
    if (std::isinf(ret.y)) {
        ret.y=(1000.0);
    }
    if (std::isinf(ret.z)) {
        ret.z=(1000.0);
    }
    if (std::isnan(ret.x)) {
        ret.x=(0.00001);
    }
    if (std::isnan(ret.y)) {
        ret.y=(0.00001);
    }
    if (std::isnan(ret.z)) {
        ret.z=(0.00001);
    }
#endif
    return ret;
}

double Camera3D::StepSize()
{
    return stepSize;
}

/// ----------------- Camera2D ---------------------

Camera2D::Camera2D(QStatusBar *statusBar) : statusBar(statusBar)
{
    center = nullptr;
    zoom = nullptr;
    enableTransform = nullptr;
    rotateAngle = nullptr;
    stretchAngle = nullptr;
    stretchAmount = nullptr;
    zoomDown = 0.0;
    mouseDown = glm::dvec3(0, 0, -1);
    keyStatus.clear();
    stepSize = 1.0;
    width = 1;
    height = 1;
}

QVector<VariableWidget *> Camera2D::addWidgets(QWidget * /*group*/,
        QWidget * /*parent*/)
{
    QVector<VariableWidget *>w;
    return w;
}

void Camera2D::printInfo()
{
    INFO(QCoreApplication::translate("Camera2D","Camera: Click on 2D window for key focus. See Help Menu for more."));
}

glm::dvec3 Camera2D::transform(int w, int h)
{
    width = w;
    height = h;
    return {1.0, 1.0, 1.0};
}

void Camera2D::connectWidgets(VariableEditor *ve)
{
    center = dynamic_cast<Float2Widget *>(ve->getWidgetFromName("Center"));
    if (center == nullptr) {
        WARNING(QCoreApplication::translate("Camera2D", "Could not find Center interface widget"));
    }
    zoom = dynamic_cast<FloatWidget *>(ve->getWidgetFromName("Zoom"));
    if (zoom == nullptr) {
        zoom = dynamic_cast<FloatWidget *>(ve->getWidgetFromName("log_Zoom"));
    }
    if (zoom == nullptr) {
        WARNING(QCoreApplication::translate("Camera2D", "Could not find Zoom interface widget"));
    }

    enableTransform = dynamic_cast<BoolWidget *>(ve->getWidgetFromName("EnableTransform"));
    rotateAngle = dynamic_cast<FloatWidget *>(ve->getWidgetFromName("RotateAngle"));
    stretchAngle = dynamic_cast<FloatWidget *>(ve->getWidgetFromName("StretchAngle"));
    stretchAmount = dynamic_cast<FloatWidget *>(ve->getWidgetFromName("StretchAmount"));
}

namespace
{
glm::dvec3 getModelCoord(glm::dvec3 mouseCoord, glm::dvec3 center, double zoom,
                        int w, int h)
{
    double ar = h / (double)w;
    glm::dvec3 coord = (mouseCoord / zoom + center);
    coord.x=(ar * coord.x);
    return coord;
}
} // namespace

glm::dmat2 Camera2D::getTransform()
{
    glm::dmat2 transform = glm::dmat2(1.0, 0.0, 0.0, 1.0);
    if (enableTransform && enableTransform->isChecked()) {
        double b = glm::radians(rotateAngle ? rotateAngle->getValue() : 0.0);
        double bc = std::cos(b);
        double bs = std::sin(b);
        double a = glm::radians(stretchAngle ? stretchAngle->getValue() : 0.0);
        double ac = std::cos(a);
        double as = std::sin(a);
        double s = std::sqrt(std::pow(2.0, stretchAmount ? stretchAmount->getValue() : 0.0));
        glm::dmat2 m1 = glm::dmat2(ac, as, -as, ac);
        glm::dmat2 m2 = glm::dmat2(s, 0.0, 0.0, 1.0 / s);
        glm::dmat2 m3 = glm::dmat2(ac, -as, as, ac);
        glm::dmat2 m4 = glm::dmat2(bc, bs, -bs, bc);
        transform = m1 * m2 * m3 * m4;
    }
    return transform;
}

bool Camera2D::parseKeys()
{
    if (center == nullptr || zoom == nullptr) {
        return false;
    }
    glm::dvec3 centerValue = glm::dvec3(center->getValue(), 0.0);
    double zoomValue = zoom->getValue();

    double factor = pow(1.05f, (double)stepSize);
    double zFactor = 0.1 / zoomValue;

    bool keysDown = false;

    // --------- step sizes ----------------------------

    if (keyDown(Qt::Key_1)) {
        stepSize = stepSize * 0.5;
        INFO(QCoreApplication::translate("Camera2D", "Step size: %1").arg(stepSize));
        keyStatus[Qt::Key_1] = false; // only apply once
    }

    if (keyDown(Qt::Key_3)) {
        stepSize = stepSize * 2.0;
        INFO(QCoreApplication::translate("Camera2D", "Step size: %1").arg(stepSize));
        keyStatus[Qt::Key_3] = false; // only apply once
    }

    if (keyDown(Qt::Key_2)) {
        stepSize = stepSize * 10.0;
        INFO(QCoreApplication::translate("Camera2D", "Step size: %1").arg(stepSize));
        keyStatus[Qt::Key_2] = false; // only apply once
    }

    if (keyDown(Qt::Key_X)) {
        stepSize = stepSize * 0.1;
        INFO(QCoreApplication::translate("Camera2D", "Step size: %1").arg(stepSize));
        keysDown = true;
        keyStatus[Qt::Key_X] = false; // only apply once
    }

    // ---------- Movement -----------------------------

    if (keyDown(Qt::Key_A)) {
        center->setValue(centerValue + glm::dvec3(getTransform() * glm::dvec2(-zFactor, 0.0), 0.0));
        keysDown = true;
    }

    if (keyDown(Qt::Key_D)) {
        center->setValue(centerValue + glm::dvec3(getTransform() * glm::dvec2(zFactor, 0.0), 0.0));
        keysDown = true;
    }


    if (keyDown(Qt::Key_W)) {
        center->setValue(centerValue + glm::dvec3(getTransform() * glm::dvec2(0.0, zFactor), 0.0));
        keysDown = true;
    }

    if (keyDown(Qt::Key_S)) {
        center->setValue(centerValue + glm::dvec3(getTransform() * glm::dvec2(0.0, -zFactor), 0.0));
        keysDown = true;
    }

    if (keyDown(Qt::Key_Q)) {
        zoom->setValue(zoomValue * factor);
        keysDown = true;
    }

    if (keyDown(Qt::Key_E)) {
        zoom->setValue(zoomValue / factor);
        keysDown = true;
    }

    askForRedraw = keysDown;
    return keysDown;
}

bool Camera2D::mouseEvent(QMouseEvent *e, int w, int h)
{
    if (center == nullptr || zoom == nullptr) {
        return false;
    }
    glm::dvec3 pos = glm::dvec3(e->pos().x() / (0.5 * double(w)) - 1.0, 1.0 - e->pos().y() / (0.5 * double(h)), 0.0);
    glm::dvec3 centerValue = glm::dvec3(center->getValue(), 0.0);
    double zoomValue = zoom->getValue();

    if (e->type() ==  QEvent::MouseButtonPress) {
        mouseDown = pos;
        zoomDown = zoomValue;
        centerDown = centerValue;
    } else if (e->type() ==  QEvent::MouseButtonRelease) {
        mouseDown = glm::dvec3(0, 0, -1);
    }

    double mouseSpeed = 1.0;
    if (mouseDown.z != -1 && e->buttons() != Qt::NoButton) {
        glm::dvec3 dp = mouseDown - pos;
        dp = glm::dvec3(getTransform() * glm::dvec2(dp), 0.0);
        if (e->buttons() == Qt::LeftButton) {
            center->setValue(centerDown + dp * mouseSpeed / zoomDown);
        } else if (e->buttons() == Qt::RightButton) {
            // Convert mouse down to model coordinates
            glm::dvec3 md = getModelCoord(mouseDown, centerDown, zoomDown, w, h);
            double newZoom = zoomDown + dp.y * (zoomDown) * mouseSpeed;
            double z = newZoom / zoomDown;
            center->setValue(md - (md - centerDown) / z);
            zoom->setValue(newZoom);
        }
        return true;
    }
    return false;
}

void Camera2D::reset(bool fullReset)
{
    keyStatus.clear();
    if (fullReset) {
        stepSize = 1.0;
    }
}

double Camera2D::StepSize()
{
    return stepSize;
}

bool Camera2D::wheelEvent(QWheelEvent *e)
{
    if (zoom == nullptr) {
        return false;
    }
    double steps = e->delta() / 120.0;
    double factor = 1.15;
    double zoomValue = zoom->getValue();
    // Convert mouse pos to model coordinates
    glm::dvec3 centerValue; centerValue.x = center->getValue().x; centerValue.y = center->getValue().y; centerValue.z=0.0;
    // traveling
    QSettings settings;
    if( settings.value("ddCameraMode").toBool() )
    {
        glm::dvec3 pos = glm::dvec3((e->pos().x() * (1.0 / double(width))) - 0.5, 0.5 - (e->pos().y() * (1.0 / double(height))), 0.0);
        pos = glm::dvec3(getTransform() * glm::dvec2(pos), 0.0);
        glm::dvec3 md = pos / zoomValue + centerValue;

        if (steps > 0.0) {
            center->setValue(md);
            zoom->setValue(zoomValue * factor);
        } else {
            center->setValue(md);
            zoom->setValue(zoomValue / factor);
        }
    }
    else // fixed
    {
    double g = steps > 0.0 ? factor : 1.0 / factor;

    double u = (e->pos().x() / double(width) - 0.5) * 2.0 * double(width) / double(height);
    double v = (e->pos().y() / double(height) - 0.5) * 2.0;

        glm::dvec3 pos = glm::dvec3(-u, v, 0.0);
        pos = glm::dvec3(getTransform() * glm::dvec2(pos), 0.0);
        glm::dvec3 md = centerValue + pos / (zoomValue * g) * (1.0 - g);

    center->setValue(md);
    zoom->setValue(zoomValue * g);

    }
    return true;
}
} // namespace GUI
} // namespace Fragmentarium
