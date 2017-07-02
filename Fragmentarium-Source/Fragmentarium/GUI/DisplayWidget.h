#pragma once

#include <QPropertyAnimation>
#include <QAnimationGroup>
#include <QVector>
#include <QtOpenGL>
#ifdef __APPLE__
#include <openGL/glext.h>
#else
#include <GL/glext.h>
#endif
#include <QGLFormat>
#include <QGLWidget>

#ifdef NVIDIAGL4PLUS
#include <QOpenGLFunctions_4_1_Core>
#else
#include <QOpenGLFunctions>
#endif // NVIDIAGL4PLUS

#include <QOpenGLVersionFunctions>

#include <QOpenGLFramebufferObject>
#include <QProgressDialog>
#include <QPoint>
#include <QList>
#include <QOpenGLShaderProgram>
#include <QMainWindow>
#include "SyntopiaCore/Logging/ListWidgetLogger.h"
#include "../Parser/Preprocessor.h"
#include "CameraControl.h"
#include "QtSpline.h"

#ifdef USE_OPEN_EXR
#include <half.h>
#include <ImfTileDescription.h>
#include <ImfTiledOutputFile.h>
#include <ImfChannelList.h>
#include <ImfRgba.h>
#include <ImfRgbaFile.h>
#include <ImfTiledRgbaFile.h>
#include <ImfArray.h>
#include <ImfNamespace.h>

#include <ImfMultiPartInputFile.h>
#include <ImfTiledInputPart.h>
#include <ImfPartHelper.h>
#include <ImfPartType.h>

#include <OpenEXRConfig.h>
#include <Iex.h>

#endif

#include <iostream>

namespace Fragmentarium {
  namespace GUI {
    
    #define DBOUT   qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__
    
    #ifdef USE_OPEN_EXR
    using namespace Imf_2_1;
    using namespace Imath_2_1;
    #endif
    using namespace Parser;
    class MainWindow;
    class VariableWidget;
    class CameraControl;
    
    
    #ifdef NVIDIAGL4PLUS
    class DisplayWidget : public QGLWidget, protected QOpenGLFunctions_4_1_Core
    #else
    class DisplayWidget : public QGLWidget, protected QOpenGLFunctions
    #endif // NVIDIAGL4PLUS
    {
      Q_OBJECT
    public:
      
      enum DrawingState { Progressive, Animation, Tiled };

      enum FeedbackModifier {
        NoFeedbackModifier = Qt::NoModifier,
        AddModifier = Qt::ControlModifier,
        MinusModifier = Qt::ControlModifier + Qt::AltModifier,
        DeleteModifier = Qt::ControlModifier + Qt::ShiftModifier,
        ClearModifier = Qt::ControlModifier + Qt::AltModifier + Qt::ShiftModifier,
        FeedbackModifierMax // this one gets auto-set to count-1
      };
      Q_DECLARE_FLAGS(FeedbackModifiers,FeedbackModifier)
      
      /// Constructor
      DisplayWidget( QGLFormat format, GUI::MainWindow* mainWin, QWidget* parent);
      
      /// Destructor
      ~DisplayWidget();
      
      void clearTileBuffer();
      QImage renderTile(double pad, double time, int subframes, int w, int h, int tile, int tileMax, QProgressDialog* progress, int* steps);
      
      /// Use this whenever a redraw is required.
      /// Calling this function multiple times will still only result in one redraw
      void requireRedraw(bool clear);
      void updateRefreshRate();
      void setState(DrawingState state);
      DrawingState getState() {
        return drawingState;
      }
      bool isContinuous() {
        return continuous;
      }
      void reset();
      void setContextMenu(QMenu* contextMenu) {
        this->contextMenu = contextMenu;
      }
      void resetCamera(bool fullReset);
      void setDisabled(bool disabled) {
        this->disabled = disabled;
      }
      void setFragmentShader(FragmentSource fs);
      bool hasShader() {
        return (shaderProgram!=0);
      }
      bool hasBufferShader() {
        return (bufferShaderProgram!=0);
      }
      void setupFragmentShader();
      void setupBufferShader();
      void setContinuous(bool value) {
        continuous = value;
      }
      void setDisableRedraw(bool value) {
        disableRedraw = value;
      }
      bool isRedrawDisabled() {
        return disableRedraw;
      }
      CameraControl* getCameraControl() {
        return cameraControl;
      }
      void resetTime() {
        time = QTime::currentTime();
      }
      void setViewFactor(int val);
      void setPreviewFactor(int val);
      FragmentSource* getFragmentSource() {
        return &fragmentSource;
      }
      QOpenGLShaderProgram* getShader() {
        return shaderProgram;
      }
      QOpenGLShaderProgram* getBufferShader() {
        return bufferShaderProgram;
      }
      void keyReleaseEvent(QKeyEvent* ev);
      void keyPressEvent(QKeyEvent* ev) Q_DECL_OVERRIDE; 
      void setMaxSubFrames(int i ) {
        maxSubFrames = i;
      }
      
      void uniformsHasChanged();
      void setClearOnChange(bool v) {
        clearOnChange = v;
      }
      bool wantsDepthToAlpha(){return depthToAlpha; };
      
      // M Benesi "Spray gun" public
      bool wantZappaAdd;
      bool wantZappaMinus;
      bool wantZappaDelete;
      bool wantZappaClear;
      bool zapLocked;
      void resetZappaStatus();
      void setZapLock( bool lok ) { zapLocked = lok; };
      bool getZapLock() { return zapLocked; };
      bool buttonDown;
      
      void clearTextureCache(QMap< QString, bool >* textureCacheUsed);
      
      QStringList getCurveSettings();
      void setCurveSettings(QStringList cset);
      
      void updatePerspective();
      void drawLookatVector();
      void drawSplines();
      QVector3D *getControlPoints(int i);
      void addControlPoint(QVector3D eP, QVector3D tP, QVector3D uP);
      void clearControlPoints();
      #ifdef NVIDIAGL4PLUS
      QStringList shaderAsm(bool w);
      #endif // NVIDIAGL4PLUS
      /// should make these private?
      QVector<QVector3D> eyeControlPoints;
      QVector<QVector3D> targetControlPoints;
      QVector<QVector3D> upControlPoints;
      QtSpline *eyeSpline;
      QtSpline *targetSpline;
      QtSpline *upSpline;
      bool foundnV;
      QString vendor;
      QString renderer;
      
      QString renderETA;
      int renderAVG;
      int framesToRender;
      int tileAVG;
      
      #ifdef USE_OPEN_EXR
      void setEXRmode(bool m) {
        exrMode = m;
      }

      void getRGBAFtile(Imf_2_1::Array2D< Imf_2_1::Rgba >& array, int w, int h);
      #endif
    public slots:
      void updateBuffers();
      void clearPreviewBuffer();
      void timerSignal();
      void updateEasingCurves( int currentframe );
      void setRenderFPS(int fps) {
        renderFPS = fps;
      }
      int getRenderFPS() {
        return renderFPS;
      }
      int isPending() {
        return pendingRedraws;
      }
      void setHasKeyFrames(bool yn) {
        hasKeyFrames = yn;
      }
      
      double getZAtMXY() {
        return ZAtMXY;
      };
      QPoint getMXY() {
        return mouseXY;
      };
      
      QString cameraID() { return cameraControl->getID(); }
      
      bool init() {
        #ifdef NVIDIAGL4PLUS
        bool ret = initializeOpenGLFunctions();
        #else
        initializeOpenGLFunctions();
        #endif // NVIDIAGL4PLUS
        vendor = QString ( ( char * ) glGetString ( GL_VENDOR ) );
        renderer = QString ( ( char * ) glGetString ( GL_RENDERER ) );
        /// test for nVidia card and set the nV flag
        foundnV = vendor.contains ( "NVIDIA", Qt::CaseInsensitive );
        #ifndef NVIDIAGL4PLUS
        bool ret = true;
        #endif // NVIDIAGL4PLUS
        return ret;
      };

/// BEGIN 3DTexture
//       void init3DTexture();
//       void set3DTextureFileName( QString vfn ){ voxelFileName = vfn; };
//       void setObjFileName( QString ofn ){ objFileName = ofn; };
//       void saveObjFile(float *vxls );
/// END 3DTexture
      
    protected:
      void drawFragmentProgram(int w,int h, bool toBuffer);
      void drawToFrameBufferObject(QOpenGLFramebufferObject* buffer, bool drawLast);
/// BEGIN 3DTexture
//       void draw3DTexture();
/// END 3DTexture
      void mouseMoveEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
      void contextMenuEvent (QContextMenuEvent* ev) Q_DECL_OVERRIDE;
      void mouseReleaseEvent ( QMouseEvent * ev) Q_DECL_OVERRIDE;
      void mousePressEvent ( QMouseEvent * ev) Q_DECL_OVERRIDE;
//       void initializeGL() Q_DECL_OVERRIDE;
//       void paintEvent(QPaintEvent * ev) Q_DECL_OVERRIDE;
      void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;
      
      int pixelWidth() {
        return width() * devicePixelRatio();
      }
      
      int pixelHeight() {
        return height() * devicePixelRatio();
      }
      
      /// Actual drawing is implemented here
      void paintGL() Q_DECL_OVERRIDE;
      
      void clearGL();
      /// Triggers a perspective update and a redraw
      void resizeGL(int w, int h) Q_DECL_OVERRIDE;
      void wheelEvent(QWheelEvent* e) Q_DECL_OVERRIDE;
      
      // M Benesi "Spray gun" protected
      void setZappaStatus(Qt::KeyboardModifiers km);
      
    private:
      QOpenGLFramebufferObject* previewBuffer;
      QOpenGLFramebufferObject* backBuffer;
      QOpenGLFramebufferObject* hiresBuffer;
      bool continuous;
      bool disableRedraw;
      QOpenGLShaderProgram* shaderProgram;
      QOpenGLShaderProgram* bufferShaderProgram;
      
      void setShaderUniforms(QOpenGLShaderProgram* shaderProg);
      
      void setGlTexParameter(QMap<QString, QString> map);
      void clearBackBuffer();
      void setViewPort(int w, int h);
      void makeBuffers();
      void setPerspective();
      int pendingRedraws; // the number of times we must redraw
      int requiredRedraws;
      QColor backgroundColor;
      int subframeCounter;
      
      QMenu* contextMenu;
      
      bool disabled;
      bool fitWindow;
      
      MainWindow* mainWindow;
      CameraControl* cameraControl;
      FragmentSource fragmentSource;
      QTime time;
      QTime fpsTimer;
      int fpsCounter;
      double padding;
      int tilesCount;
      int tiles;
      int viewFactor;
      int previewFactor;
      QString outputFile;
      //enum BufferType { None, RGBA8, RGBA16, RGBA16F, RGBA32F };
      GLenum bufferType;
      
      QDateTime tileRenderStart;
      QMap<QString, int> TextureCache;
      bool doClearBackBuffer;
      QTimer* timer;
      int maxSubFrames;
      QString bufferString;
      QString oldBufferString;
      
      bool clearOnChange;
      int iterationsBetweenRedraws;
      int bufferSizeX;
      int bufferSizeY;
      DrawingState drawingState;
      
      QStringList curveSettings;
      bool hasKeyFrames;
      int renderFPS;
      bool exrMode;
      double ZAtMXY;
      QPoint mouseXY;
      bool depthToAlpha;
      
      
      /// BEGIN 3DTexture
//       QMatrix4x4 texMatrix;
//       GLuint m3DTexId;
//       QString voxelFileName;
//       QString objFileName;
      /// END 3DTexture
      
    };
  }
}
