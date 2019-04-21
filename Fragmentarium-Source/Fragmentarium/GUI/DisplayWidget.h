#pragma once

#include <iostream>

#include <QPropertyAnimation>
#include <QAnimationGroup>
#include <QVector>

#include <QSurfaceFormat>
#include <QOpenGLWidget>

#include <QOpenGLFunctions_1_0>
#include <QOpenGLFunctions_1_1>
#include <QOpenGLFunctions_1_2>
#include <QOpenGLFunctions_1_3>
#include <QOpenGLFunctions_1_4>
#include <QOpenGLFunctions_1_5>
#include <QOpenGLFunctions_2_0>
#include <QOpenGLFunctions_2_1>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLFunctions_3_1>
#include <QOpenGLFunctions_3_2_Compatibility>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLFunctions_3_3_Compatibility>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_4_0_Compatibility>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLFunctions_4_1_Compatibility>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLFunctions_4_2_Compatibility>
#include <QOpenGLFunctions_4_2_Core>
#include <QOpenGLFunctions_4_3_Compatibility>
#include <QOpenGLFunctions_4_3_Core>
#include <QOpenGLFunctions_4_4_Compatibility>
#include <QOpenGLFunctions_4_4_Core>
#include <QOpenGLFunctions_4_5_Compatibility>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLFunctions_ES2>

#include <QOpenGLFunctions>
#include <QOpenGLVersionFunctions>

#include <QOpenGLFramebufferObject>
#include <QProgressDialog>
#include <QPoint>
#include <QList>
#include <QMainWindow>

#include <QOpenGLShaderProgram>
#include "SyntopiaCore/Logging/ListWidgetLogger.h"
#include "../Parser/Preprocessor.h"
#include "CameraControl.h"
#include "QtSpline.h"

#ifdef USE_OPEN_EXR
#ifndef Q_OS_MAC
#include <OpenEXRConfig.h>
#endif
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

#include <Iex.h>

#endif


namespace Fragmentarium {
  namespace GUI {
    
    #define DBOUT   qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__
    
    #ifdef USE_OPEN_EXR
    using namespace OPENEXR_IMF_NAMESPACE;
    using namespace IMATH_NAMESPACE;
    #endif

    using namespace SyntopiaCore::Logging;
    using namespace Parser;
    class MainWindow;
    class VariableWidget;
    class CameraControl;
    
#ifdef Q_OS_MAC
    class DisplayWidget : public QOpenGLWidget, protected QOpenGLFunctions
#else
    class DisplayWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Compatibility
#endif
    {
      Q_OBJECT
    public:
      
      enum DrawingState { Progressive, Animation, Tiled };

      /// Constructor
      DisplayWidget( GUI::MainWindow* mainWin, QWidget* parent);
      
      /// Destructor
      ~DisplayWidget();
      
      void clearTileBuffer();
      void renderTile(double pad, double time, int subframes, int w, int h, int tile, int tileMax, QProgressDialog* progress, int* steps, QImage *im, const QTime &totalTime);
      
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
      void initFragmentShader();
      void initFragmentTextures();
      void initBufferShader();
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
      FragmentSource* getFragmentSource() {
        return &fragmentSource;
      }
      QOpenGLShaderProgram* getShader() {
        return shaderProgram;
      }
      QOpenGLShaderProgram* getBufferShader() {
        return bufferShaderProgram;
      }
      void keyReleaseEvent(QKeyEvent* ev) Q_DECL_OVERRIDE;
      void keyPressEvent(QKeyEvent* ev) Q_DECL_OVERRIDE; 
      void setMaxSubFrames(int i ) {
        maxSubFrames = i;
      }
      
      void uniformsHasChanged();
      void setClearOnChange(bool v) {
        clearOnChange = v;
      }
      bool wantsDepthToAlpha(){return depthToAlpha; };
      
      bool buttonDown;
      
      void clearTextureCache(QMap<QString, bool>* textureCacheUsed);
      
      QStringList getCurveSettings();
      void setCurveSettings(QStringList cset);
      
      void updatePerspective();
      void drawLookatVector();
      void drawSplines();
      void createSplines(int numberOfControlPoints, int numberOfFrames);

      void addControlPoint(QVector3D eP, QVector3D tP, QVector3D uP);
      void clearControlPoints();

      QStringList shaderAsm(bool w);

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
      QString glvers;
      
      QString renderETA;
      int renderAVG;
      int framesToRender;
      int tileAVG;
      
      int subframeCounter;
      int tilesCount;
      #ifdef USE_OPEN_EXR
      void setEXRmode(bool m) {
        exrMode = m;
      }

      bool getRGBAFtile(Imf::Array2D< Imf::Rgba >& array, int w, int h);
      #endif
      
      void setVerbose(bool v) { verbose = v; };
      
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
      void initializeGL() Q_DECL_OVERRIDE;
      void paintEvent(QPaintEvent * ev) Q_DECL_OVERRIDE;
      void showEvent(QShowEvent * ) Q_DECL_OVERRIDE;
      
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
      QColor backgroundColor;
      
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
      int tiles;
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
      int verbose;
      
      /// BEGIN 3DTexture
//       QMatrix4x4 texMatrix;
//       GLuint m3DTexId;
//       QString voxelFileName;
//       QString objFileName;
      /// END 3DTexture
      
    };
  }
}
