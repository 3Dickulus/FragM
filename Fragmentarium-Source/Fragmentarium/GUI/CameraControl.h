#pragma once

#include "SyntopiaCore/Logging/ListWidgetLogger.h"
#include <QList>
#include <QMouseEvent>
#include <QPoint>
#include <QStatusBar>
#include <QVector3D>
#include <QVector>
#include <QWheelEvent>

namespace Fragmentarium
{
namespace GUI
{

class VariableWidget;
class VariableEditor;
class Float3Widget;
class Float2Widget;
class FloatWidget;

// CameraControl maintains camera position, and respond to user control.
class CameraControl : public QObject
{
    Q_OBJECT

public:

    CameraControl();
    virtual ~CameraControl() {}
    virtual QVector3D transform(int width, int height) = 0;
    virtual QVector3D screenTo3D(int sx, int sy, double sz) = 0;
    virtual void printInfo() = 0;
    virtual QString getID() =0;
    virtual QVector<VariableWidget *> addWidgets ( QWidget *group,
    QWidget *parent ) = 0;
    virtual void connectWidgets(VariableEditor* /*ve*/) {}
    virtual bool mouseEvent ( QMouseEvent * /*e*/, int /*w*/, int /*h*/ )
    {
        return false;
    }
    virtual bool wheelEvent ( QWheelEvent * /*e*/ )
    {
        return false;
    }
            virtual bool keyPressEvent(QKeyEvent* /*ev*/);
    virtual bool wantsRedraw()
    {
        return askForRedraw;
    }
    virtual void updateState()
    {
        parseKeys();
    }
    virtual void reset(bool /*fullReset*/){}
    virtual bool parseKeys() = 0;
    virtual void releaseControl();
    virtual double StepSize()=0;

protected:
    bool keyDown(int key);
    QMap<int, bool> keyStatus;
    bool askForRedraw;
    double stepSize;
};

class Camera3D : public CameraControl
{
    Q_OBJECT
public:
    Camera3D(QStatusBar* statusBar);
    ~Camera3D();
    virtual QVector<VariableWidget*> addWidgets(QWidget* group, QWidget* parent);
    virtual QString getID()
    {
        return "3D";
    }
    virtual void printInfo();
    virtual QVector3D screenTo3D(int sx, int sy, double sz);
    virtual QVector3D transform(int width, int height);
    virtual void connectWidgets(VariableEditor* ve);
    virtual bool mouseEvent(QMouseEvent* e, int w, int h);
    virtual bool wheelEvent(QWheelEvent* /*e*/);
    virtual bool parseKeys();
    virtual void reset(bool fullReset);
    virtual double StepSize();
private:
    int height;
    int width;
    QStatusBar* statusBar;
    Float3Widget* eye ;
    Float3Widget* target ;
    Float3Widget* up ;
    FloatWidget* fov;
    QVector3D eyeDown ;
    QVector3D targetDown ;
    QVector3D upDown ;
    double fovDown;
    QVector3D mouseDown;
protected:
    void orthogonalizeUpVector();
        };

class Camera2D : public CameraControl
{
    Q_OBJECT
public:
    Camera2D(QStatusBar* statusBar);
    virtual QVector<VariableWidget*> addWidgets(QWidget* group, QWidget* parent);
    virtual QVector3D screenTo3D ( int sx, int sy, double sz )
    {
        return QVector3D ( sx, sy, sz );
    };
            virtual void connectWidgets(VariableEditor* ve);
    virtual QString getID()
    {
        return "2D";
    }
    virtual void printInfo();
    virtual QVector3D transform ( int w, int h );
    virtual bool mouseEvent(QMouseEvent* e, int w, int h);
    virtual bool wheelEvent(QWheelEvent* /*e*/);
    virtual bool parseKeys();
    virtual void reset(bool fullReset);
    virtual double StepSize();
private:
    int height;
    int width;
    Float2Widget* center;
    FloatWidget* zoom;
    QStatusBar* statusBar;
    QVector3D mouseDown;
    double zoomDown;
    QVector3D centerDown;
};
} // namespace GUI

} // namespace Fragmentarium
