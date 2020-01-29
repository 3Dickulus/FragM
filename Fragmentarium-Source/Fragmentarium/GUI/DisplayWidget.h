#pragma once

#include <iostream>

#include <QAnimationGroup>
#include <QPropertyAnimation>
#include <QVector>

#include <QOpenGLWidget>
#include <QSurfaceFormat>

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

#ifdef USE_OPENGL_4
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
#endif

#ifdef USE_OPENGL_ES2
#include <QOpenGLFunctions_ES2>
#endif

#include <QOpenGLFunctions>

#include <QList>
#include <QMainWindow>
#include <QOpenGLFramebufferObject>
#include <QPoint>
#include <QProgressDialog>

#include "../Parser/Preprocessor.h"
#include "CameraControl.h"
#include "QtSpline.h"
#include "SyntopiaCore/Logging/ListWidgetLogger.h"
#include <QOpenGLShaderProgram>


#ifdef USE_OPEN_EXR
#ifndef Q_OS_MAC
#include <OpenEXRConfig.h>
#endif
#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfNamespace.h>
#include <ImfRgba.h>
#include <ImfRgbaFile.h>
#include <ImfTileDescription.h>
#include <ImfTiledOutputFile.h>
#include <ImfTiledRgbaFile.h>
#include <half.h>

#include <ImfInputFile.h>
#include <ImfPartHelper.h>
#include <ImfPartType.h>
#include <ImfTiledInputPart.h>

#include <Iex.h>

#endif

#include "FileManager.h"

namespace Fragmentarium
{
namespace GUI
{

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
#ifdef USE_OPENGL_4
class DisplayWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Compatibility
#else
class DisplayWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Compatibility
#endif
#endif
{
    Q_OBJECT
public:

    enum DrawingState { Progressive, Animation, Tiled };

    /// Constructor
    DisplayWidget ( GUI::MainWindow* mainWin, QWidget* parent );

    /// Destructor
    ~DisplayWidget();

    void clearTileBuffer();
    void renderTile ( double pad, double time, int subframes, int w, int h,
                      int tile, int tileMax, QProgressDialog *progress, int *steps,
                      QImage *im, const QTime &totalTime );

    /// Use this whenever a redraw is required.
    /// Calling this function multiple times will still only result in one redraw
    void requireRedraw ( bool clear );
    void updateRefreshRate();
    void setState ( DrawingState state );
    DrawingState getState()
    {
        return drawingState;
    }
    bool isContinuous()
    {
        return continuous;
    }
    void reset();
    void setContextMenu ( QMenu *contextMenu )
    {
        this->contextMenu = contextMenu;
    }
    void resetCamera ( bool fullReset );
    void setDisabled ( bool disabled )
    {
        this->disabled = disabled;
    }
    void setFragmentShader ( FragmentSource fs );
    bool hasShader()
    {
        return ( shaderProgram!=nullptr );
    }
    bool hasBufferShader()
    {
        return ( bufferShaderProgram!=0 );
    }
    void initFragmentShader();
    void initFragmentTextures();
    void initBufferShader();
    void setContinuous ( bool value )
    {
        continuous = value;
    }
    void setDisableRedraw ( bool value )
    {
        disableRedraw = value;
    }
    bool isRedrawDisabled()
    {
        return disableRedraw;
    }
    CameraControl *getCameraControl()
    {
        return cameraControl;
    }
    void resetTime()
    {
        time = QTime::currentTime();
    }
    FragmentSource *getFragmentSource()
    {
        return &fragmentSource;
    }
    QOpenGLShaderProgram *getShader()
    {
        return shaderProgram;
    }
    QOpenGLShaderProgram *getBufferShader()
    {
        return bufferShaderProgram;
    }
    void keyReleaseEvent ( QKeyEvent* ev ) Q_DECL_OVERRIDE;
    void keyPressEvent ( QKeyEvent* ev ) Q_DECL_OVERRIDE;
    void setMaxSubFrames ( int i )
    {
        maxSubFrames = i;
    }

    void uniformsHaveChanged( bool bshaderonly = false );
    void setClearOnChange ( bool v )
    {
        clearOnChange = v;
    }
    bool wantsDepthToAlpha()
    {
        return depthToAlpha;
    };

    bool buttonDown;

    void clearTextureCache ( QMap<QString, bool>* textureCacheUsed );

    QStringList getCurveSettings();
    void setCurveSettings ( const QStringList cset );

    void updatePerspective();
    void drawLookatVector();
    void drawSplines();
    void createSplines ( int numberOfControlPoints, int numberOfFrames );

    void addControlPoint ( glm::dvec3 eP, glm::dvec3 tP, glm::dvec3 uP );
    void clearControlPoints();

    QStringList shaderAsm ( bool w );

    /// should make these private?
    QVector<glm::dvec3> eyeControlPoints;
    QVector<glm::dvec3> targetControlPoints;
    QVector<glm::dvec3> upControlPoints;
    QtSpline *eyeSpline;
    QtSpline *targetSpline;
    QtSpline *upSpline;
    bool foundnV;
    QString vendor;
    QString renderer;
    QString glvers;

    QString renderETA;
    int renderAVG;
    int renderToFrame;
    int tileAVG;

    int subframeCounter;
    int tilesCount;
// #ifdef USE_OPEN_EXR
    void setEXRmode ( bool m )
    {
        exrMode = m;
    }
#ifdef USE_OPEN_EXR
    bool getRGBAFtile ( Imf::Array2D<RGBAFLOAT>& pixels, int w, int h );
#endif

    void setVerbose ( bool v )
    {
        verbose = v;
    };

    GLuint spline_program;

public slots:
    void updateBuffers();
    void timerSignal();
    void updateEasingCurves ( int currentframe );
    void setRenderFPS ( int fps )
    {
        renderFPS = fps;
    }
    int getRenderFPS()
    {
        return renderFPS;
    }

    void setHasKeyFrames ( bool yn )
    {
        hasKeyFrames = yn;
    }

    double getZAtMXY()
    {
        return ZAtMXY;
    };
    QPoint getMXY()
    {
        return mouseXY;
    };

    QString cameraID()
    {
        return cameraControl->getID();
    }
    
    bool isCompat(){ return format().profile() == QSurfaceFormat::CompatibilityProfile; };
    void useCompat(bool c){ compatibilityProfile = c; };

/// Spline Shaders /////////////////////////////////////////////////////////
    void delete_buffer_objects();
    void init_arrays();
/// Spline Shaders /////////////////////////////////////////////////////////

protected:
    void drawFragmentProgram ( int w,int h, bool toBuffer );
    void drawToFrameBufferObject ( QOpenGLFramebufferObject* buffer, bool drawLast );
    void mouseMoveEvent ( QMouseEvent* ev ) Q_DECL_OVERRIDE;
    void contextMenuEvent ( QContextMenuEvent* ev ) Q_DECL_OVERRIDE;
    void mouseReleaseEvent ( QMouseEvent * ev ) Q_DECL_OVERRIDE;
    void mousePressEvent ( QMouseEvent * ev ) Q_DECL_OVERRIDE;
    void initializeGL() Q_DECL_OVERRIDE;
    void paintEvent ( QPaintEvent * ev ) Q_DECL_OVERRIDE;
    void showEvent ( QShowEvent *ev ) Q_DECL_OVERRIDE;
    void createErrorLineLog( QString message, QString log, LogLevel priority, bool bS );
    int pixelWidth()
    {
        return width() * devicePixelRatio();
    }

    int pixelHeight()
    {
        return height() * devicePixelRatio();
    }

    /// Actual drawing is implemented here
    void paintGL() Q_DECL_OVERRIDE;

    void clearGL();
    /// Triggers a perspective update and a redraw
    void resizeGL ( int w, int h ) Q_DECL_OVERRIDE;
    void wheelEvent ( QWheelEvent *ev ) Q_DECL_OVERRIDE;

/// Spline Shaders /////////////////////////////////////////////////////////
    void render_array(int number, double size);
    uint compile_shader( const char* vsource, const char* fsource );
    void init_shader( int h, int w );
    double pixel_scale;
/// Spline Shaders /////////////////////////////////////////////////////////

private:
    QOpenGLFramebufferObject* previewBuffer;
    QOpenGLFramebufferObject* backBuffer;
    QOpenGLFramebufferObject* hiresBuffer;
    bool continuous;
    bool disableRedraw;
    QOpenGLShaderProgram* shaderProgram;
    QOpenGLShaderProgram* bufferShaderProgram;

/// for Main Shaders
	GLuint vbo;
	GLuint vao;
/// for Spline Shaders
    GLuint svbo;
    GLuint svao;

	GLfloat points[9] = { -1.0f, -1.0f, 0.0f, 3.0f, -1.0f, 0.0f, -1.0f, 3.0f, 0.0f };

    void setShaderUniforms ( QOpenGLShaderProgram* shaderProg );

    void setGlTexParameter ( QMap<QString, QString> map );
    void setViewPort ( int w, int h );
    void makeBuffers();
    void setPerspective();

    bool initPreviewBuffer();

    bool loadHDRTexture(QString texturePath, GLenum type, GLuint textureID, QString uniformName="");
// #ifdef USE_OPEN_EXR
    bool loadEXRTexture(QString texturePath, GLenum type, GLuint textureID, QString uniformName="");
// #endif
    bool loadQtTexture(QString texturePath, GLenum type, GLuint textureID, QString uniformName="");

    bool setTextureParms(QString textureUniformName, GLenum type);
    void checkForSpecialCase(QString uniformName, QString &uniformValue);
    void get32Type(GLenum type, QString &tp);
    bool checkShaderProg(GLuint programID);
#ifdef USE_OPENGL_4
    void get64Type(GLuint programID, GLenum type, QString uniformName, QString uniformValue, bool &foundDouble, QString &tp);
#endif

    void setupShaderVars(QOpenGLShaderProgram *shaderProg, int w, int h);
    void draw3DHints();
    bool FBOcheck();
    
    GLenum glCheckError_(const char *file, int line, const char *func);
    
    int pendingRedraws; // the number of times we must redraw
    bool bufferUniformsHaveChanged;
    
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
    bool verbose;
    bool bufferShaderOnly;
    bool glDebugEnabled;
    bool compatibilityProfile;
/// Spline Shaders /////////////////////////////////////////////////////////
// GLSL legacy code works here for now, modern code will require adjustments

#define STRINGIFY(A) #A
// vertex shader
const char *vertexShader = STRINGIFY(
uniform vec3 posEye;
uniform float FOV;
uniform float pointRadius;  // point size in world space
uniform float pointScale;   // scale to calculate size in pixels
void main()
{
    // calculate window-space point size
    float dist = length(posEye)*FOV;
    gl_PointSize = pointRadius * (pointScale / dist);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);
    gl_FrontColor = gl_Color;
}
);

const char *spherePixelShader = STRINGIFY(
void main()
{
    const vec3 lightDir = vec3(0.0, 0.0, 1.0);

    // calculate normal
    vec3 N;
    N.xy = gl_PointCoord.xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(N.xy, N.xy);
    if (mag > 1.0) discard;   // kill pixels outside circle
    N.z = sqrt(mag);
    // calculate lighting
    float diffuse = max(0.0, dot(lightDir, N));
    gl_FragColor = vec4(gl_Color.rgb, diffuse);
}
);

glm::mat4 m_pvmMatrix;

/// GL4
// STRINGIFY does not allow #version statement
QString vertexShader4 = QString("#version 450 core\n"
"layout(location = 0) in vec4 vertex_position;\n"
"uniform vec4 vertex_colour;\n"
"out vec4 colour;\n"
"uniform mat4 pvmMatrix;\n"
"uniform vec3 posEye;\n"
"uniform float FOV;\n"
"uniform float pointRadius;\n"
"uniform float pointScale;\n"
"void main() {\n"
"    float dist = length(posEye)*FOV;\n"
"    gl_PointSize = pointRadius * (pointScale / dist);\n"
"    gl_Position = pvmMatrix * vec4(vertex_position.xyz, 1.0);\n"
"    colour = vertex_colour;\n"
"}\n");

QString spherePixelShader4 = QString("#version 450 core\n"
"in vec4 colour;\n"
"out vec4 frag_colour;\n"
"void main() {\n"
"    const vec3 lightDir = vec3(0.0, 0.0, 1.0);\n"
"    vec3 N;\n"
"    N.xy = gl_PointCoord.xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);\n"
"    float mag = dot(N.xy, N.xy);\n"
"    if (mag > 1.0) discard;\n"
"    N.z = sqrt(mag);\n"
"    float diffuse = max(0.0, dot(lightDir, N));\n"
"    frag_colour = vec4(colour.rgb, diffuse);\n"
"}\n");

/// Spline Shaders /////////////////////////////////////////////////////////

};
}
} // namespace Fragmentarium
