#include <stdio.h>

#include <QWheelEvent>
#include <QStatusBar>
#include <QMenu>
#include <QVector2D>
#include <QFileInfo>
#include <QMatrix4x4>
#include <QVector3D>

#include "DisplayWidget.h"
#ifdef __APPLE__
#include <openGL/glext.h>
#else
#include <GL/glext.h>
#endif
#include "MainWindow.h"
#include "VariableWidget.h"
#include "../../ThirdPartyCode/hdrloader.h"

#ifdef USE_OPEN_EXR
using namespace Imf_2_1;
using namespace Imath_2_1;
#endif

using namespace SyntopiaCore::Logging;

namespace Fragmentarium {
namespace GUI {

using namespace Parser;

namespace {
// Literal names for OpenGL flags
QStringList GetOpenGLFlags() {
    QGLFormat::OpenGLVersionFlags f = QGLFormat::openGLVersionFlags ();
    QStringList s;
    s.append ( "OpenGL " );
    if ( f & QGLFormat::OpenGL_Version_1_1 ) s.append ( "1.1" );
    if ( f & QGLFormat::OpenGL_Version_1_2 ) s.append ( "1.2" );
    if ( f & QGLFormat::OpenGL_Version_1_3 ) s.append ( "1.3" );
    if ( f & QGLFormat::OpenGL_Version_1_4 ) s.append ( "1.4" );
    if ( f & QGLFormat::OpenGL_Version_1_5 ) s.append ( "1.5" );
    if ( f & QGLFormat::OpenGL_Version_2_0 ) s.append ( "2.0" );
    if ( f & QGLFormat::OpenGL_Version_2_1 ) s.append ( "2.1" );
    if ( f & QGLFormat::OpenGL_Version_3_0 ) s.append ( "3.0" );
    if ( f & QGLFormat::OpenGL_Version_3_2 ) s.append ( "3.2" );
    if ( f & QGLFormat::OpenGL_Version_3_3 ) s.append ( "3.3" );
    if ( f & QGLFormat::OpenGL_Version_4_0 ) s.append ( "4.0" );
    if ( f & QGLFormat::OpenGL_Version_4_1 ) s.append ( "4.1" );
    if ( f & QGLFormat::OpenGL_Version_4_2 ) s.append ( "4.2" );
    if ( f & QGLFormat::OpenGL_Version_4_3 ) s.append ( "4.3" );
    if ( f & QGLFormat::OpenGL_ES_CommonLite_Version_1_0 ) s.append ( "ES_CL_1.0" );
    if ( f & QGLFormat::OpenGL_ES_Common_Version_1_0 ) s.append ( "ES_C_1.0" );
    if ( f & QGLFormat::OpenGL_ES_CommonLite_Version_1_1 ) s.append ( "ES_CL_1.1" );
    if ( f & QGLFormat::OpenGL_ES_Common_Version_1_1 ) s.append ( "ES_C_1.1" );
    if ( f & QGLFormat::OpenGL_ES_Version_2_0 ) s.append ( "ES_2,0" );
    return s;
}
}


DisplayWidget::DisplayWidget ( QGLFormat format, MainWindow* mainWin, QWidget* parent )
    : QGLWidget ( format, parent, 0, 0 ), mainWindow ( mainWin ) {
    clearOnChange = true;
    drawingState = Progressive;
    hiresBuffer = 0;
    iterationsBetweenRedraws = 0;
    previewBuffer = 0;
    doClearBackBuffer = true;
    backBuffer = 0;
    subframeCounter = 0;
    bufferShaderProgram = 0;
    shaderProgram = 0;
    bufferType = 0;
    viewFactor = 0;
    previewFactor = 0.0;
    tilesCount = 0;
    fitWindow = true;
    bufferSizeX=0;
    bufferSizeY=0;
    resetTime();
    fpsTimer = QTime::currentTime();
    fpsCounter = 0;
    continuous = false;
    disableRedraw = false;
    cameraControl = new Camera2D ( mainWindow->statusBar() );
    disabled = false;
    updatePerspective();
    pendingRedraws = 0;
    requiredRedraws = 1;
    setMouseTracking ( true );
    backgroundColor = QColor ( 30,30,30 );
    contextMenu = 0;
    setFocusPolicy ( Qt::WheelFocus );
    timer = 0;
    maxSubFrames = 0;
    eyeSpline = targetSpline = upSpline = NULL;
    setAutoFillBackground ( false );
    foundnV = false;
    requireRedraw ( true );
    exrMode = false;
    depthToAlpha = false;
    ZAtMXY=0.0;
    // M Benesi "Spray gun" DisplayWidget
    zapLocked = false;
    wantZappaMinus = false;
    wantZappaDelete = false;
    wantZappaAdd = false;
    wantZappaClear = false;
    /// BEGIN 3DTexture
    //     m3DTexId = 0;
    /// END 3DTexture
    buttonDown = false;
    }

void DisplayWidget::updateRefreshRate() {
    QSettings settings;
    int i = settings.value ( "refreshRate", 20 ).toInt();
    if ( !timer ) {
        timer = new QTimer();
        connect ( timer, SIGNAL ( timeout() ), this, SLOT ( timerSignal() ) );
    }
    timer->start ( i );
    renderFPS = settings.value ( "fps", 25 ).toInt();
    INFO ( tr( "Setting display update timer to %1 ms (max %2 FPS)." ).arg ( i ).arg ( 1000.0/i,0,'f',2 ) );

}

// let the system handle paint events,
// void DisplayWidget::paintEvent(QPaintEvent * ev) {
//     QGLWidget::paintEvent(ev);
// }

DisplayWidget::~DisplayWidget() {
}

void DisplayWidget::contextMenuEvent(QContextMenuEvent* ev ) {
    if(ev->modifiers() == Qt::ShiftModifier)
        if( mainWindow->isFullScreen()) {
            contextMenu->show();
            QPoint myPos((width()-contextMenu->width())/2,(height()-contextMenu->height())/2);
            contextMenu->move( mapToGlobal( myPos ));
        }
}

void DisplayWidget::setFragmentShader ( FragmentSource fs ) {

    fragmentSource = fs;
    clearOnChange = fs.clearOnChange;
    iterationsBetweenRedraws = fs.iterationsBetweenRedraws;
    if ( fs.subframeMax != -1 ) {
        mainWindow->setSubframeMax ( fs.subframeMax );
    }
    
    // Camera setup
    if ( fragmentSource.camera == "" ) {
        fragmentSource.camera = "2D";
    }
    if ( cameraControl->getID() != fragmentSource.camera ) {
        if ( fragmentSource.camera == "2D" ) {
            delete ( cameraControl );
            cameraControl = new Camera2D ( mainWindow->statusBar() );
        } else if ( fragmentSource.camera == "3D" ) {
            delete ( cameraControl );
            cameraControl = new Camera3D ( mainWindow->statusBar() );
        } else {
            WARNING ( tr("Unknown camera type: ") + fragmentSource.camera );
        }
    }

    cameraControl->printInfo();

    // Buffer setup
    QString b = fragmentSource.buffer.toUpper();
    if ( b=="" || b=="NONE" ) {
        bufferType = 0;
    } else if ( b == "RGBA8" ) {
        bufferType = GL_RGBA8;
    } else if ( b == "RGBA16" ) {
        bufferType = GL_RGBA16;
    } else if ( b == "RGBA16F" ) {
        bufferType = GL_RGBA16F;
    } else if ( b == "RGBA32F" ) {
        bufferType = GL_RGBA32F;
    } else {
        WARNING (tr("Unknown buffertype requested: ") + b + tr(".\n Type must be: NONE, RGBA8, RGBA16, RGBA16F, RGBA32F\nInitialized as GL_RGBA8") );
        bufferType = GL_RGBA8;
    }

    makeBuffers();
    INFO ( tr("Created front and back buffers as ") + b );
    int s;
    glGetIntegerv ( GL_MAX_TEXTURE_SIZE, &s ); // GL_MAX_3D_TEXTURE_SIZE GL_MAX_CUBE_MAP_TEXTURE_SIZE
    INFO ( tr( "Maximum texture size: %1x%1" ).arg ( s ) );

    requireRedraw ( true );
    setupFragmentShader();    
}

void DisplayWidget::requireRedraw ( bool clear ) {
    if ( disableRedraw ) return;
    pendingRedraws = requiredRedraws;
    if ( clear ) {
        clearBackBuffer();
    } else {
        subframeCounter = 0;
    }
}

void DisplayWidget::uniformsHasChanged() {
  if(fragmentSource.depthToAlpha) {
    BoolWidget *btest = dynamic_cast<BoolWidget*>(mainWindow->getVariableEditor()->getWidgetFromName("DepthToAlpha"));
    if(btest != NULL) {
      // widget detected
      depthToAlpha = btest->isChecked();
    }
  }
  requireRedraw ( clearOnChange );
}

/// /* Texture mapping */
/// at some point I would like to make these selectable in the gui
/// http://www.informit.com/articles/article.aspx?p=770639&seqNum=4 may help
// GL_TEXTURE_ENV
// GL_TEXTURE_ENV_MODE
// GL_TEXTURE_1D
// GL_TEXTURE_2D
// GL_TEXTURE_WRAP_S
// GL_TEXTURE_WRAP_T
// GL_TEXTURE_MAG_FILTER
// GL_TEXTURE_MIN_FILTER
// GL_TEXTURE_ENV_COLOR
// GL_TEXTURE_GEN_S
// GL_TEXTURE_GEN_T
// GL_TEXTURE_GEN_R
// GL_TEXTURE_GEN_Q
// GL_TEXTURE_GEN_MODE
// GL_TEXTURE_BORDER_COLOR
// GL_TEXTURE_WIDTH
// GL_TEXTURE_HEIGHT
// GL_TEXTURE_BORDER
// GL_TEXTURE_COMPONENTS
// GL_TEXTURE_RED_SIZE
// GL_TEXTURE_GREEN_SIZE
// GL_TEXTURE_BLUE_SIZE
// GL_TEXTURE_ALPHA_SIZE
// GL_TEXTURE_LUMINANCE_SIZE
// GL_TEXTURE_INTENSITY_SIZE
// GL_NEAREST_MIPMAP_NEAREST
// GL_NEAREST_MIPMAP_LINEAR
// GL_LINEAR_MIPMAP_NEAREST
// GL_LINEAR_MIPMAP_LINEAR
// GL_OBJECT_LINEAR
// GL_OBJECT_PLANE
// GL_EYE_LINEAR
// GL_EYE_PLANE
// GL_SPHERE_MAP
// GL_DECAL
// GL_MODULATE
// GL_NEAREST
// GL_REPEAT
// GL_CLAMP
// GL_S
// GL_T
// GL_R
// GL_Q

/*
/// in a frag this should create a mipmapped texture

uniform sampler2D myTexture; file[test.png]
#TexParameter myTexture GL_TEXTURE_MAX_LEVEL 1000
#TexParameter myTexture GL_TEXTURE_WRAP_S GL_REPEAT
#TexParameter myTexture GL_TEXTURE_WRAP_T GL_REPEAT
#TexParameter myTexture GL_TEXTURE_MAG_FILTER GL_LINEAR
#TexParameter myTexture GL_TEXTURE_MIN_FILTER GL_LINEAR_MIPMAP_LINEAR

*/

void DisplayWidget::setGlTexParameter ( QMap<QString, QString> map ) {

    /// as per https://www.opengl.org/wiki/Common_Mistakes#Automatic_mipmap_generation
    /// this has to get done before the rest of the GL texture parameters are set
    if ( map.values().contains ( "GL_LINEAR_MIPMAP_LINEAR" ) ||
            map.values().contains ( "GL_LINEAR_MIPMAP_NEAREST" ) ||
            map.values().contains ( "GL_NEAREST_MIPMAP_LINEAR" ) ||
            map.values().contains ( "GL_NEAREST_MIPMAP_NEAREST" )
       ) {
        if ( map.keys().contains ( "GL_TEXTURE_MAX_LEVEL" ) ) {
            bool ok;
            GLint levels;
            GLint wantedLevels = map["GL_TEXTURE_MAX_LEVEL"].toInt ( &ok,10 );
            if ( !ok ) wantedLevels = 128; // just an arbitrary small number, GL default = 1000
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, wantedLevels );

#ifdef NVIDIAGL4PLUS
            glGenerateMipmap ( GL_TEXTURE_2D ); //Generate mipmaps here!!!
#else
            glTexParameteri ( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE ); //Generate mipmaps here!!!
#endif // NVIDIAGL4PLUS

            // read back and test our value
            glGetTexParameteriv ( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, &levels );

            if ( wantedLevels != levels ) {
              WARNING( tr("Asked gpu for %1 mip-map levels.").arg(wantedLevels) );
              WARNING( tr("GPU returned %1 mip-map levels.").arg(levels));
            }
        }
    }

    if ( map.keys().contains ( "GL_TEXTURE_MAX_ANISOTROPY" ) ) {
        bool ok;
        GLfloat fLargest;
        glGetFloatv ( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest ); // max max
        GLfloat fWanted = map["GL_TEXTURE_MAX_ANISOTROPY"].toFloat ( &ok );
        if ( !ok ) fWanted = 1.0;
        glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ( fWanted > 1.0 ) ? ( fWanted<fLargest ) ?fWanted:fLargest:1.0 );
        glGetTexParameterfv ( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest );
        if ( fLargest != fWanted ) {
          WARNING( tr("Asked gpu for %1 anisotropic filter.").arg(fWanted));
          WARNING( tr("GPU returned %1 anisotropic filter.").arg(fLargest));
        }
    }

    QMapIterator<QString, QString> i ( map );

    while ( i.hasNext() ) {
        i.next();
        QString key = i.key();
        QString value = i.value();
        GLenum k = 0;
        if ( key == "GL_TEXTURE_WRAP_S" ) {
            k = GL_TEXTURE_WRAP_S;
        } else if ( key == "GL_TEXTURE_WRAP_T" ) {
            k = GL_TEXTURE_WRAP_T;
        } else if ( key == "GL_TEXTURE_MIN_FILTER" ) {
            k = GL_TEXTURE_MIN_FILTER;
        } else if ( key == "GL_TEXTURE_MAG_FILTER" ) {
            k = GL_TEXTURE_MAG_FILTER;
        } else if ( key == "GL_TEXTURE_MAX_LEVEL" ) {
            continue;
        } else if ( key == "GL_TEXTURE_MAX_ANISOTROPY" ) {
            continue;
        } else {
            WARNING ( tr("Unable to parse TexParameter key: ") + key );
            continue;
        }

        GLint v = 0;
        if ( value == "GL_CLAMP" ) {
            v = GL_CLAMP;
        } else if ( value == "GL_REPEAT" ) {
            v = GL_REPEAT;
        } else if ( value == "GL_NEAREST" ) {
            v = GL_NEAREST;
        } else if ( value == "GL_LINEAR" ) {
            v = GL_LINEAR;
        } else if ( value == "GL_LINEAR_MIPMAP_LINEAR" ) {
            v = GL_LINEAR_MIPMAP_LINEAR;
        } else if ( value == "GL_LINEAR_MIPMAP_NEAREST" ) {
            v = GL_LINEAR_MIPMAP_NEAREST;
        } else if ( value == "GL_NEAREST_MIPMAP_LINEAR" ) {
            v = GL_NEAREST_MIPMAP_LINEAR;
        } else if ( value == "GL_NEAREST_MIPMAP_NEAREST" ) {
            v = GL_NEAREST_MIPMAP_NEAREST;
        } else {
            WARNING ( tr("Unable to parse TexParameter value: ") + value );
            continue;
        }

        glTexParameteri ( GL_TEXTURE_2D, k, v );
    }
}

#ifdef NVIDIAGL4PLUS
// this function parses the assembler text and returns a string list containing the lines
QStringList DisplayWidget::shaderAsm ( bool w ) {
    GLuint progId = w ? shaderProgram->programId() : bufferShaderProgram->programId();
    GLint formats = 0;
    glGetIntegerv ( GL_NUM_PROGRAM_BINARY_FORMATS, &formats );
    GLint binaryFormats[formats];
    glGetIntegerv ( GL_PROGRAM_BINARY_FORMATS, binaryFormats );

    GLint len=0;
    glGetProgramiv ( progId, GL_PROGRAM_BINARY_LENGTH, &len );
    uchar binary[len];
    glGetProgramBinary ( progId, len, NULL, ( GLenum* ) binaryFormats, binary );

    QString asmTxt = "";
    QStringList asmList;

    for ( int x=0; x<len-1; x++ ) {
        if ( isprint ( binary[x] ) ) {
            asmTxt += binary[x];
        }

        if ( !asmList.isEmpty() && asmList.last() == "END" && !asmTxt.startsWith ( "!" ) ) {
            asmTxt="";
            x++;
        }

        if ( binary[x] == ';' || binary[x] == 10 || binary[x] == 0 ) {
            if ( asmTxt.length() > 2 ) {
                asmList << asmTxt;
            }
            asmTxt="";
        }
    }

    // tidy head and tail
    while ( !fragmentSource.getText().contains ( asmList.first() ) ) asmList.removeFirst();
    while ( asmList.last() != "END" ) asmList.removeLast();

    return asmList;
}
#endif // NVIDIAGL4PLUS

void DisplayWidget::setupFragmentShader() {

    QMap<QString, bool> textureCacheUsed;
    QImage im;

    if ( shaderProgram ) {
        shaderProgram->release();
    }
    delete ( shaderProgram );
    shaderProgram = new QOpenGLShaderProgram ( this );

    // Vertex shader
    bool s = false;
    s = shaderProgram->addShaderFromSourceCode ( QOpenGLShader::Vertex,fragmentSource.vertexSource.join ( "\n" ) );
    if ( fragmentSource.vertexSource.count() == 0 ) {
        WARNING ( tr("No vertex shader found!") );
        s = false;
    }

    if ( !s ) {
        WARNING ( tr("Could not create vertex shader: ") + shaderProgram->log() );
        delete ( shaderProgram );
        shaderProgram = 0;
        return;
    }
    if ( !shaderProgram->log().isEmpty() ) INFO ( tr("Vertex shader compiled with warnings: ") + shaderProgram->log() );

    // Fragment shader
    s = shaderProgram->addShaderFromSourceCode ( QOpenGLShader::Fragment, fragmentSource.getText() );

    if ( s ) s = shaderProgram->create();

    if ( !s ) {
        WARNING ( tr("Could not create fragment shader: ") + shaderProgram->log() );
        delete ( shaderProgram );
        shaderProgram = 0;
        return;
    }

    if ( !shaderProgram->log().isEmpty() ) INFO ( tr("Fragment shader compiled with warnings: ") + shaderProgram->log() );

    s = shaderProgram->link();

    if ( !s ) {
        QStringList tmp = shaderProgram->log().split ( '\n' );
        QStringList logStrings; // the first 5 lines
        for ( int i=0; i<5; i++ ) logStrings << tmp.at ( i );
        logStrings << "";
        WARNING ( tr("Could not link fragment shaders: ") );
        CRITICAL ( logStrings.join ( '\n' ) );
        delete ( shaderProgram );
        shaderProgram = 0;
        return;
    }
    if ( !shaderProgram->log().isEmpty() ) INFO ( tr("Fragment shader compiled with warnings: ") + shaderProgram->log() );

    s = shaderProgram->bind();
    if ( !s ) {
        WARNING ( tr("Could not bind shaders: ") + shaderProgram->log() );
        delete ( shaderProgram );
        shaderProgram = 0;
        return;
    }

    // Setup textures.
    int u = 0;
    // Bind first texture to backbuffer
    int l = shaderProgram->uniformLocation ( "backbuffer" );
    if ( l != -1 ) {
        if ( bufferType != 0 ) {
            glActiveTexture ( GL_TEXTURE0+u ); // non-standard (>OpenGL 1.3) gl extension
            GLuint i = backBuffer->texture();
            glBindTexture ( GL_TEXTURE_2D,i );
            if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
            }
            shaderProgram->setUniformValue ( l, ( GLuint ) u );
            //INFO(QString("Binding back buffer (ID: %1) to active texture %2").arg(backBuffer->texture()).arg(u));
            //INFO(QString("Setting uniform backbuffer to active texture %2").arg(u));
            u++;
        } else {
            WARNING ( tr("Trying to use a backbuffer, but no bufferType set.") );
            WARNING ( tr("Use the buffer define, e.g.: '#buffer RGBA8' ") );
        }
    } else {
        // Apparently we must always bind the backbuffer texture.
        // FIX: this indicates an error in the later binding of user textures.
        glActiveTexture ( GL_TEXTURE0+u ); // non-standard (>OpenGL 1.3) gl extension
        u++;
    }

    GLuint textureID=u;

    for ( QMap<QString, QString>::iterator it = fragmentSource.textures.begin(); it!=fragmentSource.textures.end(); it++ ) {

        QString textureName = it.key();
        QString texturePath = it.value();
        bool loaded = false;
        int l = shaderProgram->uniformLocation ( textureName );
        if ( l != -1 ) { // found named texture in shader program
            
            // 2D or Cube ?
            GLsizei bufSize = 256;
            GLsizei length;
            GLint size;
            GLenum type;
            GLchar name[bufSize];

            GLuint index = shaderProgram->uniformLocation ( textureName );
            glGetActiveUniform( shaderProgram->programId(), index, bufSize, &length, &size, &type, name);
            
            // check cache first
            if ( TextureCache.contains ( texturePath ) ) {
                textureID = TextureCache[texturePath];
                textureCacheUsed[texturePath] = true;
                glBindTexture ((type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID );
                loaded = true;
            } else { // if not in cache then create one and try to load and add to cache
                // set current texture
                glActiveTexture ( GL_TEXTURE0+u ); // non-standard (>OpenGL 1.3) gl extension
                glPixelStorei ( GL_UNPACK_ALIGNMENT, 4 ); // byte alignment 4 bytes = 32 bits
                // allocate a texture id
                glGenTextures ( 1, &textureID );
//                 INFO ( QString ( "Allocated texture ID: %1" ).arg ( textureID ) );
                glBindTexture ( (type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID );

                if ( texturePath.endsWith ( ".hdr", Qt::CaseInsensitive ) ) { // is HDR format image ?
                    HDRLoaderResult result;
                    if ( HDRLoader::load ( texturePath.toLatin1().data(), result ) ) {
                        if(type == GL_SAMPLER_CUBE) {
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*0*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*1*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*2*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*3*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*4*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*5*3] );
                        } else
                            glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, result.width, result.height, 0, GL_RGB, GL_FLOAT, result.cols );
                        loaded = true;
                    }
                }
#ifdef USE_OPEN_EXR
#ifdef WIN32
                else if ( texturePath.endsWith ( ".exr", Qt::CaseInsensitive ) ) { // is EXR format image ?
                    //
                    // Read an RGBA image using class RgbaInputFile:
                    //
                    //    - open the file
                    //    - allocate memory for the pixels
                    //    - describe the memory layout of the pixels
                    //    - read the pixels from the file
                    //
                    RgbaInputFile file ( texturePath.toLatin1().data() );
                    Box2i dw = file.dataWindow();

                    if ( file.isComplete() ) {
                        int w  = dw.max.x - dw.min.x + 1;
                        int h = dw.max.y - dw.min.y + 1;
                        int s;
                        glGetIntegerv ( GL_MAX_TEXTURE_SIZE, &s );
                        s /= 4;
                        if ( w>s || h>s ) {
                            WARNING ( tr( "Exrloader found EXR image: %1 x %2 is too large! max %3x%3" ).arg ( w ).arg ( h ).arg ( s ) );
                            continue;
                        }
                        Array2D<Rgba>pixels ( w, 1 );

                        float *cols = new float[w*h*4];
                        while ( dw.min.y <= dw.max.y ) {
                            file.setFrameBuffer ( &pixels[0][0] - dw.min.x - dw.min.y * w, 1, w );
                            file.readPixels ( dw.min.y, dw.min.y );
                            // process scanline (pixels)
                            for ( int i = 0; i<w; i++ ) {
                                // convert 3D array to 1D
                                int indx = ( dw.min.y*w+i ) *4;
                                cols[indx]=pixels[0][i].r;
                                cols[indx+1]=pixels[0][i].g;
                                cols[indx+2]=pixels[0][i].b;
                                cols[indx+3]=pixels[0][i].a;
                            }
                            dw.min.y ++;
                        }
                        glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA , w, h, 0, GL_RGBA, GL_FLOAT, cols );
                        loaded = true;
                        delete [] cols;
                        cols = 0;
                    } else
                        WARNING ( tr("Exrloader found EXR image: %1 is not complete").arg ( texturePath ) );
                }
#endif
#endif
                else if ( im.load ( texturePath ) ) { // Qt format image, Qt 5+ loads EXR format on linux
                    QImage t = convertToGLFormat ( im.mirrored(true,true) );
                    
                    if(type == GL_SAMPLER_CUBE) {
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*0) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*1) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*2) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*3) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*4) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*5) );
                    } else {
                      glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
                    }
                    
                    loaded = true;
                }
                // add to cache
                if ( loaded ) {
                    TextureCache[texturePath] = textureID;
                    textureCacheUsed[texturePath] = true;
                }
            }
            if ( loaded ) {
/*
#define providesBackground
#group Skybox
uniform samplerCube skybox; file[cubemap.png]
vec3  backgroundColor(vec3 dir) {
    return textureCube(skybox, dir).rgb;
}
*/
                if(type == GL_SAMPLER_CUBE) {
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

                } else if ( fragmentSource.textureParams.contains ( textureName ) ) { // set texparms
                    setGlTexParameter ( fragmentSource.textureParams[textureName] );
                } else { // User did not set texparms Disable mip-mapping per default.
                    QMap<QString, QString> map;
                    map.insert ( "GL_TEXTURE_MAG_FILTER","GL_LINEAR" );
                    map.insert ( "GL_TEXTURE_MIN_FILTER","GL_LINEAR" );
                    setGlTexParameter ( map );
                }
                //INFO(QString("Setting uniform %1 to active texture %2").arg(textureName).arg(u));
                shaderProgram->setUniformValue ( l, ( GLuint ) u );
                u++;
            } else  WARNING ( tr("Not a valid texture: ") + QFileInfo ( texturePath ).absoluteFilePath() );
        } else WARNING ( tr("Unused sampler uniform: ") + textureName );
        
    }
    setupBufferShader();
    clearTextureCache ( &textureCacheUsed );
}

void DisplayWidget::clearTextureCache ( QMap<QString, bool> *textureCacheUsed ) {
    if ( textureCacheUsed ) {
        // Check for unused textures
        QMutableMapIterator<QString, int> i ( TextureCache );
        while ( i.hasNext() ) {
            i.next();
            if ( !textureCacheUsed->contains ( i.key() ) ) {
                INFO ( tr("Removing texture from cache: ") +i.key() );
                GLuint id = i.value();
                deleteTexture ( id );
                TextureCache.remove ( i.key() );
            }
        }
    } else {
        QMapIterator<QString, int> i ( TextureCache );
        while ( i.hasNext() ) {
            i.next();
            INFO ( tr("Removing unused texture from cache: ") +i.key() );
            GLuint id = i.value();
            deleteTexture ( id );
        }
    }
    TextureCache.clear();
}


void DisplayWidget::setupBufferShader() {
    if ( bufferShaderProgram ) {
        bufferShaderProgram->release();
    }
    delete ( bufferShaderProgram );
    bufferShaderProgram = 0;

    if ( !fragmentSource.bufferShaderSource ) return;

    bufferShaderProgram = new QOpenGLShaderProgram ( this );

    // Vertex shader
    bool s = false;
    s = bufferShaderProgram->addShaderFromSourceCode ( QOpenGLShader::Vertex,fragmentSource.bufferShaderSource->vertexSource.join ( "\n" ) );
    if ( fragmentSource.bufferShaderSource->vertexSource.count() == 0 ) {
        WARNING ( tr("No buffer shader vertex shader found!") );
        s = false;
    }
    
    if ( !s ) {
        WARNING ( tr("Could not create buffer vertex shader: ") + bufferShaderProgram->log() );
        delete ( bufferShaderProgram );
        bufferShaderProgram = 0;
        return;
    }
    if ( !bufferShaderProgram->log().isEmpty() ) INFO ( tr("Buffer vertex shader compiled with warnings: ") + bufferShaderProgram->log() );

    // Fragment shader
    s = bufferShaderProgram->addShaderFromSourceCode ( QOpenGLShader::Fragment, fragmentSource.bufferShaderSource->getText() );

    if ( s ) s = bufferShaderProgram->create();

    if ( !s ) {
        WARNING ( tr("Could not create buffer fragment shader: ") + bufferShaderProgram->log() );
        delete ( bufferShaderProgram );
        bufferShaderProgram = 0;
        return;
    }
    if ( !bufferShaderProgram->log().isEmpty() ) INFO ( tr("Buffer fragment shader compiled with warnings: ") + bufferShaderProgram->log() );

    s = bufferShaderProgram->link();

    if ( !s ) {
        QStringList tmp = bufferShaderProgram->log().split ( '\n' );
        QStringList logStrings; // the first 5 lines
        for ( int i=0; i<5; i++ ) logStrings << tmp.at ( i );
        logStrings << "";
        WARNING ( tr("Could not link buffershader: ") );
        CRITICAL ( logStrings.join ( '\n' ) );
        delete ( bufferShaderProgram );
        bufferShaderProgram = 0;
        return;
    }
    if ( !bufferShaderProgram->log().isEmpty() ) INFO ( tr("Fragment shader compiled with warnings: ") + bufferShaderProgram->log() );

    s = bufferShaderProgram->bind();
    if ( !s ) {
        WARNING ( tr("Could not bind shaders: ") + bufferShaderProgram->log() );
        delete ( shaderProgram );
        bufferShaderProgram = 0;
        return;
    }
}

void DisplayWidget::setViewFactor ( int val ) {
    viewFactor = val;
    requireRedraw ( true );
}

void DisplayWidget::resetCamera ( bool fullReset ) {
    if ( !cameraControl ) return;
    cameraControl->reset ( fullReset );
}

void DisplayWidget::setPreviewFactor ( int val ) {
    previewFactor = val;
    makeBuffers();
    requireRedraw ( true );
}

void DisplayWidget::makeBuffers() {
    int w = pixelWidth() / ( previewFactor+1 );
    int h = pixelHeight() / ( previewFactor+1 );

    if ( bufferSizeX!=0 ) {
        w = bufferSizeX;
        h = bufferSizeY;
    }

    QString b; // As of now, we never do direct renders. Always two buffers.
    switch ( bufferType ) {
    case 0:
        b = "NONE";
        bufferType=GL_RGBA32F;
        break;
    case GL_RGBA8:
        b = "RGBA8";
        break;
    case GL_RGBA16:
        b = "RGBA16";
        break;
    case GL_RGBA16F:
        b = "RGBA16F";
        break;
    case GL_RGBA32F:
        b = "RGBA32F";
        break;
    default:
        b = "UNKNOWN";
        bufferType=GL_RGBA32F;
        break;
    }
    
    // we must create both the backbuffer and previewBuffer
    bufferString = QString ( "%1x%2 %3." ).arg ( w ).arg ( h ).arg ( b );
    if ( oldBufferString == bufferString ) return;
    oldBufferString = bufferString;

    if ( previewBuffer!=0 ) {
        delete ( previewBuffer );
        previewBuffer = 0;
    }
    if ( backBuffer!=0 ) {
        delete ( backBuffer );
        backBuffer = 0;
    }

    QOpenGLFramebufferObjectFormat fbof;
    fbof.setAttachment ( QOpenGLFramebufferObject::Depth );
    fbof.setInternalTextureFormat ( bufferType );
    fbof.setTextureTarget ( GL_TEXTURE_2D );

    makeCurrent();
    // we must create both the backbuffer and previewBuffer
    backBuffer = new QOpenGLFramebufferObject ( w, h, fbof );
    clearBackBuffer();
    previewBuffer = new QOpenGLFramebufferObject ( w, h, fbof );
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        WARNING( tr("FBO Incomplete Error!") );
    }
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
}

void DisplayWidget::clearBackBuffer() {
    doClearBackBuffer = true;
}

void DisplayWidget::setViewPort ( int w, int h ) {
    if ( drawingState == Tiled ) {
        glViewport ( 0, 0,bufferSizeX, bufferSizeY );
    } else if ( fitWindow ) {
        glViewport ( 0, 0, w, h );
    } else {
        glViewport ( 0, 0,bufferSizeX<w ? bufferSizeX : w, bufferSizeY<h ? bufferSizeY : h );
    }
}

/// BEGIN 3DTexture
// void DisplayWidget::init3DTexture() {
//   
//   if(voxelFileName.isEmpty()) return;
//   qDebug() << "Loading :" << voxelFileName << "...";
//   //
//   // parse the multipart input
//   //
//   string filename ( voxelFileName.toStdString() );
//   
//   // add check for existance of the file
//   try
//   {
//     MultiPartInputFile temp (filename.c_str());
//   }
//   catch (IEX_NAMESPACE::BaseExc &e)
//   {
//     std::cerr << std::endl << "ERROR: " << e.what() << std::endl;
//     return;
//   }
//   
//   MultiPartInputFile *inputimage = new MultiPartInputFile (filename.c_str());
//   int numOutputs = inputimage->parts();
//   
//   int d;
//   glGetIntegerv ( GL_MAX_3D_TEXTURE_SIZE, &d );
//   d /= 4;
//   
//   //
//   // separate outputs
//   //
//   int p = 0;
//   Box2i dw = inputimage->header(p).dataWindow();
//   int w  = dw.max.x - dw.min.x + 1;
//   int h = dw.max.y - dw.min.y + 1;
//   
//   std::cout << "numOutputs: " << numOutputs << " W:" << w << " H:" << h << std::endl;
//   
//   if ( w>d || h>d ) {
//     WARNING ( QString ( "EXR voxel loader found X:%1 Y:%2 too large! max %3x%3" ).arg ( w ).arg ( h ).arg ( d ) );
//     return;
//   }
//   else
//     if ( w != numOutputs || h != numOutputs ) {
//       WARNING ( QString ( "EXR voxel loader found X:%1 Y:%2 Z:%3" ).arg ( w ).arg ( h ).arg ( numOutputs ) );
//       return;
//     }
//     
//     float *voxels = new float[w*h*d*4];
//   
//   while (p < numOutputs)
//   {
//     Header header = inputimage->header(p);
//     
//     std::string type = header.type();
//     
//     std::cout << "image:" << p << "\r";
//     
//     if ( inputimage->partComplete(p) ) {
//       
//       Array2D<Rgba>pixels ( w, h );
//       TiledInputPart tip( *inputimage, p );
//       
//       FrameBuffer frameBuffer;
//       
//       frameBuffer.insert ("R",                                     // name
//                           Slice (HALF,                        // type
//                                  (char *) &pixels[0][0].r,     // base
//                                  sizeof (pixels[0][0]) * 1,       // xStride
//                                  sizeof (pixels[0][0]) * w)); // yStride
//       
//       frameBuffer.insert ("G",                                     // name
//                           Slice (HALF,                       // type
//                                  (char *) &pixels[0][0].g,     // base
//                                  sizeof (pixels[0][0]) * 1,       // xStride
//                                  sizeof (pixels[0][0]) * w)); // yStride
//       
//       frameBuffer.insert ("B",                                     // name
//                           Slice (HALF,                        // type
//                                  (char *) &pixels[0][0].b,     // base
//                                  sizeof (pixels[0][0]) * 1,       // xStride
//                                  sizeof (pixels[0][0]) * w)); // yStride
//       
// //       frameBuffer.insert ("A",                                     // name
// //                           Slice (HALF,                       // type
// //                                  (char *) &pixels[0][0].a,     // base
// //                                  sizeof (pixels[0][0]) * 1,       // xStride
// //                                  sizeof (pixels[0][0]) * w)); // yStride
//       
//       tip.setFrameBuffer ( frameBuffer );
//       tip.readTile( 0, 0 );
//       
//       // convert to GL pixels
//       for ( int j = 0; j<h; j++ ) {
//         for ( int i = 0; i<w; i++ ) {
//           // convert 3D array to 1D
//           int indx = ( (w*h*p) + (j*w+i) ) *4;
//           voxels[indx]=pixels[j][i].r;
//           voxels[indx+1]=pixels[j][i].g;
//           voxels[indx+2]=pixels[j][i].b;
//           // in this case we use r+g+b / 3 as alpha value to conserve some ram space
//           voxels[indx+3] = (pixels[j][i].r+pixels[j][i].g+pixels[j][i].b) * 0.33333; // setting alpha
//         }
//       }
//       
//     } else
//       WARNING ( QString ( "Exrloader found EXR image: %1 part %2 is not complete" ).arg ( voxelFileName ).arg ( p ) );
//     
//     p++;
//   }
// 
//   std::cout << "\nSending data to GPU...\n";
//   
//   if( 0 != m3DTexId )
//   {
//     glDeleteTextures( 1, (GLuint*)&m3DTexId );
//   }
//   glGenTextures(1,(GLuint*)&m3DTexId );
//   glBindTexture( GL_TEXTURE_3D, m3DTexId );
//   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
//   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//   glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   glTexImage3D ( GL_TEXTURE_3D, 0, GL_RGBA , w, h, numOutputs, 0, GL_RGBA, GL_FLOAT, voxels );
//   glBindTexture( GL_TEXTURE_3D, 0 );
//   
//   std::cout << "\nDone.\n";
//   
//   if(!objFileName.isEmpty()) saveObjFile( voxels );
//   
//   delete inputimage;
//   delete [] voxels;
//   voxels = 0;
// }
// 
// void DisplayWidget::draw3DTexture() {
//   
//   if( m3DTexId==0 && !voxelFileName.isEmpty() ) init3DTexture();
//   if( m3DTexId==0) return;
//   
//   glEnable( GL_ALPHA_TEST );
//   glAlphaFunc( GL_GREATER, 0.05f );
//   glDisable(GL_CULL_FACE);
//   glEnable(GL_BLEND);
//   glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//   
//   glPushAttrib ( GL_ALL_ATTRIB_BITS );
// //   glMatrixMode ( GL_PROJECTION );
// //   glLoadIdentity();
// //   glMatrixMode ( GL_MODELVIEW );
// //   glLoadIdentity();
//   
// 
//   QStringList cs = mainWindow->getCameraSettings().split ( "\n" );
//   float fov = cs.filter ( "FOV" ).at ( 0 ).split ( "=" ).at ( 1 ).toFloat();
//   QStringList cv = cs.filter ( "Eye " ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   QVector3D eye = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
//   cv = cs.filter ( "Target" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   QVector3D target = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
//   cv = cs.filter ( "Up" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   QVector3D up = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
//   
//   float ascpectRatio = float( ( float ) width() / ( float ) height() );
//   float zNear = 0.00001;
//   float zFar = 1000.0;
//   float vertAngle = 180.0 * ( 2.0 * atan2 ( 1.0, ( 1.0/fov ) ) / M_PI );
//   
//   QMatrix4x4 matrix;
//   matrix.setToIdentity();
//   matrix.perspective ( vertAngle, ascpectRatio, zNear, zFar );
//   matrix.lookAt ( eye,target,up );
//   
//   texMatrix = matrix;
//   
//   glMatrixMode( GL_TEXTURE );
//   glLoadIdentity();
// 
// //   if(texMatrix != QMatrix() ) {
// //     texMatrix.scale( (float)width()/(float)height(), -1.0f, 1.0f);
// //     glLoadMatrixf(texMatrix.constData());
// //   }
// //   else{
// //   // Translate and make 0.5f as the center 
// //   // (texture co ordinate is from 0 to 1.
// //   // so center of rotation has to be 0.5f)
//   glTranslatef( 0.5f, 0.5f, 0.5f );
//   // A scaling applied to normalize the axis 
//   // (Usually the number of slices will be less so if this is not - 
//   // normalized then the z axis will look bulky)
//   // Flipping of the y axis is done by giving a negative value in y axis.
//   // This can be achieved either by changing the y co ordinates in -
//   // texture mapping or by negative scaling of y axis
//   glScalef( (float)width()/(float)height(), 
//             -1.0f, 
//             1.0f);
// 
// // //   glRotatef(rotangle, vX,vY,vZ);
// //   
//   glTranslatef( -0.5f,-0.5f, -0.5f );
// //   }
//   
// 
//   glEnable(GL_TEXTURE_3D);
//   glBindTexture( GL_TEXTURE_3D, m3DTexId );
//   
//     for ( float fIndx = -1.0f; fIndx <= 1.0f; fIndx+=0.0195f )
//     {
//         glBegin(GL_QUADS);
// 
//         glTexCoord3f(0.0f, 0.0f, ((float)fIndx+1.0f)/2.0f); glVertex3f(-1.0f,-1.0f,fIndx);
//         glTexCoord3f(1.0f, 0.0f, ((float)fIndx+1.0f)/2.0f); glVertex3f( 1.0f,-1.0f,fIndx);
//         glTexCoord3f(1.0f, 1.0f, ((float)fIndx+1.0f)/2.0f); glVertex3f( 1.0f, 1.0f,fIndx);
//         glTexCoord3f(0.0f, 1.0f, ((float)fIndx+1.0f)/2.0f); glVertex3f(-1.0f, 1.0f,fIndx);
//         
// //         glTexCoord3f(0.0f, ((float)fIndx+1.0f)/2.0f, 0.0f); glVertex3f(-1.0f,fIndx,-1.0f);
// //         glTexCoord3f(1.0f, ((float)fIndx+1.0f)/2.0f, 0.0f); glVertex3f( 1.0f,fIndx,-1.0f);
// //         glTexCoord3f(1.0f, ((float)fIndx+1.0f)/2.0f, 1.0f); glVertex3f( 1.0f,fIndx, 1.0f);
// //         glTexCoord3f(0.0f, ((float)fIndx+1.0f)/2.0f, 1.0f); glVertex3f(-1.0f,fIndx, 1.0f);
// //         
// //         glTexCoord3f( ((float)fIndx+1.0f)/2.0f,0.0f, 0.0f); glVertex3f(fIndx,-1.0f,-1.0f);
// //         glTexCoord3f( ((float)fIndx+1.0f)/2.0f,1.0f, 0.0f); glVertex3f(fIndx, 1.0f,-1.0f);
// //         glTexCoord3f( ((float)fIndx+1.0f)/2.0f,1.0f, 1.0f); glVertex3f(fIndx, 1.0f, 1.0f);
// //         glTexCoord3f( ((float)fIndx+1.0f)/2.0f,0.0f, 1.0f); glVertex3f(fIndx,-1.0f, 1.0f);
//         glEnd();
//     }
// 
//   glBindTexture( GL_TEXTURE_3D, 0 );
//   glPopAttrib();
//   glEnable(GL_CULL_FACE);
//   
// }
// 
// void DisplayWidget::saveObjFile(float *vxls ){ 
//   
//   std::cout << "Saving .obj point cloud..." << std::endl;
//   
//   QFile fileStream( objFileName );
//   if (!fileStream.open(QFile::WriteOnly | QFile::Text)) {
//     QMessageBox::warning(this, tr("Fragmentarium"),
//                          tr("Cannot write file %1:\n%2.")
//                          .arg(objFileName)
//                          .arg(fileStream.errorString()));
//     return;
//   }
//   
//   QTextStream out(&fileStream);
//   int pcnt = 0;
//   
//   out << "# Fragmentarium v1.0.25 point cloud\n";
//   for(int z=0;z<512;z++) {
//     for(int y=0;y<512;y++) {
//       for(int x=0;x<512;x++) {
//         int indx = ((512*512*z) + (y*512+x))*4;
//         float scale = (1.0/512.0);
//         if( vxls[indx+3] != 0.0 ) {
//           out << "v " << (0.5-(scale*x))*16.0 << " " << (0.5-(scale*y))*16.0 << " " << (0.5-(scale*z))*14.0 << "\n";
//           pcnt++;
//         }
//         std::cout << "Point:" << pcnt << '\r';
//       } } } std::cout << "Done " << pcnt << " points out of " << 512*512*512 << std::endl;
// }
/// END 3DTexture

void DisplayWidget::setShaderUniforms(QOpenGLShaderProgram* shaderProg) {
    
    // this should speed things up a litte because we are only setting uniforms on the first subframe
    if(subframeCounter > 1) return;
    // contains ALL uniforms in buffershader and shader program
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();

    GLuint programID = shaderProg->programId();
   
    int count;
    glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);

    if(programID == shaderProgram->programId()) {
        qDebug() << "shaderProgram";
    }

    if(programID == bufferShaderProgram->programId()) {
        qDebug() << "bufferShaderProgram";
    }

    for (int i = 0; i < count; i++) {

        GLsizei bufSize = 256;
        GLsizei length;
        GLint size;
        GLenum type;
        GLchar name[bufSize];

        double x,y,z,w;

        glGetActiveUniform( programID, (GLuint)i, bufSize, &length, &size, &type, name);        
        GLuint index = glGetUniformLocation(programID, name);
        QString uniformName = (char *)name;
        QString uniformValue;
        
        // find a value to go with the name index in the program may not be the same as index in our list
        for( int n=0; n < vw.count(); n++) {
            if(uniformName == vw[n]->getName()) {
                uniformValue = vw[n]->getValueAsText();
                break;
            }
        }
        
        QString tp;
        bool found = false;
        if (!uniformValue.isEmpty()) {

            switch(type) {

                case GL_BYTE:           tp = "BYTE "; found = false; break;
                case GL_UNSIGNED_BYTE:  tp = "UNSIGNED_BYTE"; found = false; break;
                case GL_SHORT:          tp = "SHORT"; found = false; break;
                case GL_UNSIGNED_SHORT: tp = "UNSIGNED_SHORT"; found = false; break;
                case GL_INT:            tp = "INT  "; found = false; break;
                case GL_UNSIGNED_INT:   tp = "UNSIGNED_INT"; found = false; break;
                case GL_FLOAT:          tp = "FLOAT"; found = false; break;
                case GL_FIXED:          tp = "FIXED"; found = false; break;
                case GL_FLOAT_VEC2:     tp = "FLOAT_VEC2"; found = false; break;
                case GL_FLOAT_VEC3:     tp = "FLOAT_VEC3"; found = false; break;
                case GL_FLOAT_VEC4:     tp = "FLOAT_VEC4"; found = false; break;
                case GL_INT_VEC2:       tp = "INT_VEC2"; found = false; break;
                case GL_INT_VEC3:       tp = "INT_VEC3"; found = false; break;
                case GL_INT_VEC4:       tp = "INT_VEC4"; found = false; break;
                case GL_BOOL:           tp = "BOOL "; found = false; break;
                case GL_BOOL_VEC2:      tp = "BOOL_VEC2"; found = false; break;
                case GL_BOOL_VEC3:      tp = "BOOL_VEC3"; found = false; break;
                case GL_BOOL_VEC4:      tp = "BOOL_VEC4"; found = false; break;
                case GL_FLOAT_MAT2:     tp = "FLOAT_MAT2"; found = false; break;
                case GL_FLOAT_MAT3:     tp = "FLOAT_MAT3"; found = false; break;
                case GL_FLOAT_MAT4:     tp = "FLOAT_MAT4"; found = false; break;
                case GL_SAMPLER_2D:     tp = "SAMPLER_2D"; found = false; break;
                case GL_SAMPLER_CUBE:   tp = "SAMPLER_CUBE"; found = false; break;
                           
                case GL_DOUBLE:         tp = "DOUBLE"; found = true;
                glUniform1d(index, uniformValue.toDouble());
                break;
                case GL_DOUBLE_VEC2:    tp = "DOUBLE_VEC2"; found = true;
                x = uniformValue.split(",").at(0).toDouble();
                y = uniformValue.split(",").at(1).toDouble();
                glUniform2d(index, x, y);
                break;
                case GL_DOUBLE_VEC3:    tp = "DOUBLE_VEC3"; found = true;
                x = uniformValue.split(",").at(0).toDouble();
                y = uniformValue.split(",").at(1).toDouble();
                z = uniformValue.split(",").at(2).toDouble();
                glUniform3d(index, x, y, z);
                break;
                case GL_DOUBLE_VEC4:    tp = "DOUBLE_VEC4"; found = true;
                x = uniformValue.split(",").at(0).toDouble();
                y = uniformValue.split(",").at(1).toDouble();
                z = uniformValue.split(",").at(2).toDouble();
                w = uniformValue.split(",").at(3).toDouble();
                glUniform4d(index, x, y, z, w);
                break;
                case GL_DOUBLE_MAT2:    tp = "DOUBLE_MAT2"; found = true; break;
                case GL_DOUBLE_MAT3:    tp = "DOUBLE_MAT3"; found = true; break;
                case GL_DOUBLE_MAT4:    tp = "DOUBLE_MAT4"; found = true; break;
                case GL_DOUBLE_MAT2x3:  tp = "DOUBLE_MAT2x3"; found = true; break;
                case GL_DOUBLE_MAT2x4:  tp = "DOUBLE_MAT2x4"; found = true; break;
                case GL_DOUBLE_MAT3x2:  tp = "DOUBLE_MAT3x2"; found = true; break;
                case GL_DOUBLE_MAT3x4:  tp = "DOUBLE_MAT3x4"; found = true; break;
                case GL_DOUBLE_MAT4x2:  tp = "DOUBLE_MAT4x2"; found = true; break;
                case GL_DOUBLE_MAT4x3:  tp = "DOUBLE_MAT4x3"; found = true; break;
                default:
                break;
            }

            // type name and value to console
            qDebug() << tp << "\t" << uniformName << uniformValue;
            // this sets User (32 bit) uniforms not handled above
            for( int n=0; n < vw.count(); n++) {
                if(uniformName == vw[n]->getName() && !found) {
                    vw[n]->setIsDouble(false); // ensure sliders set to float decimals
                    vw[n]->setUserUniform(shaderProg);
                    break;
                }
            }
        } else {
            tp.sprintf("%x",type);
            qDebug() << tp << uniformName;
        }
        
        if(found) vw[i]->setIsDouble(true); // this takes care of buffershader (Post) sliders :D
    }
    qDebug() << " ";

    
}

void DisplayWidget::drawFragmentProgram ( int w,int h, bool toBuffer ) {
    //static int c = 0;
    //INFO(QString("Draw fragment program: %1").arg(c++));

    /// BEGIN 3DTexture
    //     if( m3DTexId==0 )
    /// END 3DTexture
    shaderProgram->bind();

    // -- Viewport
    if ( toBuffer ) {
        glViewport ( 0, 0,w,h );
    } else {
        setViewPort ( w,h );
    }
    
    /// BEGIN 3DTexture
    //     draw3DTexture();
    //     if( m3DTexId!=0 ) return;
    /// END 3DTexture

    // -- Projection
    // The projection mode as used here
    // allow us to render only a region of the viewport.
    // This allows us to perform tile based rendering.
    glMatrixMode ( GL_PROJECTION );

    if ( getState() == DisplayWidget::Tiled ) {
        double x = ( tilesCount / tiles ) - ( tiles-1 ) /2.0;
        double y = ( tilesCount % tiles ) - ( tiles-1 ) /2.0;

        glLoadIdentity();

        glTranslated ( x * ( 2.0/tiles ) , y * ( 2.0/tiles ), 1.0 );
        glScaled ( ( 1.0+padding ) /tiles, ( 1.0+padding ) /tiles,1.0 );
    }

    cameraControl->transform ( pixelWidth(), pixelHeight() ); // -- Modelview + loadIdentity

    int l = shaderProgram->uniformLocation ( "pixelSize" );
    if ( l != -1 ) {
        shaderProgram->setUniformValue ( l, ( float ) ( 1.0/w ), ( float ) ( 1.0/h ) );
    }
    // Only in DepthBufferShader.frag & NODE-Raytracer.frag
    l = shaderProgram->uniformLocation ( "globalPixelSize" );
    if ( l != -1 ) {
        int d = 1; // TODO: Set to Tile factor. if ( d<1 ) d = 1;
        if ( viewFactor > 0 ) {
            d = viewFactor+1;
        }
        shaderProgram->setUniformValue ( l, ( ( float ) d/w ), ( ( float ) d/h ) );
    }

    l = shaderProgram->uniformLocation ( "time" );
    if ( l != -1 ) {
        double t = mainWindow->getTime() / ( double ) renderFPS;
        shaderProgram->setUniformValue ( l, ( float ) t );
    } else {
        mainWindow->getTime();
    }

    if ( bufferType!=0 ) {
        l = shaderProgram->uniformLocation ( "backbuffer" );
        if ( l != -1 ) {
            glActiveTexture ( GL_TEXTURE0 ); // non-standard (>OpenGL 1.3) gl extension
            GLuint i = backBuffer->texture();
            glBindTexture ( GL_TEXTURE_2D,i );
            if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
            }
            shaderProgram->setUniformValue ( l, 0 );
            //INFO(QString("Binding backbuffer (ID: %1) to active texture %2").arg(i).arg(0));
            //INFO(QString("Setting uniform backbuffer to active texture %2").arg(0));
        }

        l = shaderProgram->uniformLocation ( "subframe" );
        if ( l != -1 ) {
            shaderProgram->setUniformValue ( l, subframeCounter );
            //INFO(QString("Setting subframe: %1").arg(subframeCounter));

        }
    }

    // Setup User Uniforms
    // new method
    setShaderUniforms(shaderProgram);   
    // old method
    // mainWindow->setUserUniforms(shaderProgram);   
   
    // save current state
    glPushAttrib ( GL_ALL_ATTRIB_BITS );

    glColor4f ( 1.0,1.0,1.0,1.0 );

    glDepthFunc ( GL_ALWAYS );    // always passes test so we write color
    glEnable ( GL_DEPTH_TEST );   // enable depth testing
    glDepthMask ( GL_TRUE );      // enable depth buffer writing

    glBegin ( GL_TRIANGLES );     // shader draws on this surface
    glTexCoord2f ( 0.0f, 0.0f );
    glVertex3f ( -1.0f, -1.0f,  0.0f );
    glTexCoord2f ( 2.0f, 0.0f );
    glVertex3f ( 3.0f, -1.0f,  0.0f );
    glTexCoord2f ( 0.0f, 2.0f );
    glVertex3f ( -1.0f,  3.0f,  0.0f );
    glEnd();
    // finished with the shader
    shaderProgram->release();
    
    // draw splines using depth buffer for occlusion... or not
    if ( mainWindow->wantPaths() && cameraControl->getID() == "3D") {
        if ( eyeSpline!=NULL && drawingState != Animation && !isContinuous() && drawingState != Tiled ) {
            if ( mainWindow->wantSplineOcc() ) {
              // this lets splines be visible when DEPTH_TO_ALPHA mode is active
              if(depthToAlpha )
                    glDepthFunc ( GL_ALWAYS );
                else {
                    glDepthFunc ( GL_LESS );    // closer to eye passes test
                    glDepthMask ( GL_FALSE );   // no Writing to depth buffer for splines
                }
                glEnable ( GL_DEPTH_TEST ); // only testing
            } else {
                glDisable ( GL_DEPTH_TEST );  // disable depth testing
            }
            setPerspective();
            drawSplines();
            drawLookatVector();
        }
    }
    /// copy the depth value @ mouse XY
    if(!zapLocked) {
      float zatmxy;
      glReadPixels ( mouseXY.x(), height() - mouseXY.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zatmxy );
      ZAtMXY = zatmxy;
    }
    // restore state
    glPopAttrib();
   
}

void DisplayWidget::drawToFrameBufferObject ( QOpenGLFramebufferObject* buffer, bool drawLast ) {
    //static int c = 0;
    //INFO(QString("drawToFrameBufferObject: %1").arg(c++));

    if ( previewBuffer == 0 || !previewBuffer->isValid() ) {
      WARNING ( tr("Non valid FBO - previewBuffer") );
        return;
    }

    if ( backBuffer == 0 || !backBuffer->isValid() ) {
      WARNING ( tr("Non valid FBO - backBuffer") );
        return;
    }

    QSize s = backBuffer->size();

    if ( s.height() ==0 || s.width() ==0 ) {
      WARNING ( tr("BACK BUFFER FAILED!") );
        return;
    }

    s = previewBuffer->size();

    if ( s.height() ==0 || s.width() ==0 ) {
      WARNING ( tr("PREVIEW BUFFER FAILED!") );
        return;
    }

    if ( !drawLast ) {
        for ( int i = 0; i <= iterationsBetweenRedraws; i++ ) {
            if ( backBuffer ) {
                // swap backbuffer
                QOpenGLFramebufferObject* temp = backBuffer;
                backBuffer= previewBuffer;
                previewBuffer = temp;
                subframeCounter++;
            }

            if ( !previewBuffer->bind() ) {
              WARNING ( tr("Failed to bind FBO") );
                return;
            }

            drawFragmentProgram ( s.width(),s.height(), true );

            if ( !previewBuffer->release() ) {
              WARNING ( tr("Failed to release FBO") );
                return;
            }
        }
    }

    mainWindow->setSubFrameDisplay ( subframeCounter );

    if ( buffer && !buffer->bind() ) {
      WARNING ( tr("Failed to bind target buffer") );
        return;
    }

    // Draw a textured quad using the preview texture.
    if ( bufferShaderProgram ) {
        bufferShaderProgram->bind();
        int l = bufferShaderProgram->uniformLocation ( "pixelSize" );
        if ( l != -1 ) {
            shaderProgram->setUniformValue ( l, ( float ) ( 1.0/s.width() ), ( float ) ( 1.0/s.height() ) );
        }
        // for DepthBufferShader.frag & NODE-Raytracer.frag
        l = bufferShaderProgram->uniformLocation ( "globalPixelSize" );
        if ( l != -1 ) {
            int d = 1; // TODO: Set to Tile factor. if ( d<1 ) d = 1;
            if ( viewFactor > 0 ) {
                d = viewFactor+ 1;
            }
            shaderProgram->setUniformValue ( l, ( d/ ( float ) s.width() ), ( d/ ( float ) s.height() ) );
        }

        l = bufferShaderProgram->uniformLocation ( "frontbuffer" );
        if ( l != -1 ) {
            bufferShaderProgram->setUniformValue ( l, 0 );
        } else {
          WARNING ( tr("No front buffer sampler found in buffer shader. This doesn't make sense.") );
        }
        // new method
        setShaderUniforms ( bufferShaderProgram );
        // old method
        // mainWindow->setUserUniforms(bufferShaderProgram);   
    }

    glPushAttrib ( GL_ALL_ATTRIB_BITS );
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity();
    glMatrixMode ( GL_MODELVIEW );
    glLoadIdentity();
//                 if (bufferShaderProgram) {
    setViewPort ( pixelWidth(),pixelHeight() );
//                 } else {
//                     glViewport(0, 0, width(),height());
//                 }
    glActiveTexture ( GL_TEXTURE0 ); // non-standard (>OpenGL 1.3) gl extension
    glBindTexture ( GL_TEXTURE_2D, previewBuffer->texture() );

    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glEnable ( GL_TEXTURE_2D );

    glDisable ( GL_DEPTH_TEST ); // No testing: coming from RTB (render to buffer)
    glDepthMask ( GL_FALSE );   // No writing: output is color data from post effects

    glBegin ( GL_TRIANGLES );
    glTexCoord2f ( 0.0f, 0.0f );
    glVertex3f ( -1.0f, -1.0f,  0.0f );
    glTexCoord2f ( 2.0f, 0.0f );
    glVertex3f ( 3.0f, -1.0f,  0.0f );
    glTexCoord2f ( 0.0f, 2.0f );
    glVertex3f ( -1.0f,  3.0f,  0.0f );
    glEnd();
    glPopAttrib();

    if ( bufferShaderProgram ) bufferShaderProgram->release();
    if ( buffer && !buffer->release() ) {
        WARNING ( tr("Failed to release target buffer") );
    }
}

/**
 * Call this before and after performing a tile render.
 * @brief DisplayWidget::clearTileBuffer
 */
void DisplayWidget::clearTileBuffer()  {
    if ( hiresBuffer!=0 ) {
        hiresBuffer->release();
        delete ( hiresBuffer );
        hiresBuffer = 0;
    }
    mainWindow->getBufferSize ( pixelWidth(), pixelHeight(),bufferSizeX, bufferSizeY, fitWindow );
    makeBuffers();
}

void DisplayWidget::clearGL() {
    /// proper clear on tile based GPU
    /// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-TileBasedArchitectures.pdf
    glDisable ( GL_SCISSOR_TEST );
    glColorMask ( GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE );
    glDepthMask ( GL_TRUE );
    glStencilMask ( 0xFFFFFFFF );
    glClear ( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT );
}

#ifdef USE_OPEN_EXR
void DisplayWidget::getRGBAFtile ( Array2D<Rgba>&array, int w, int h ) {

  GLfloat (*myImgdata)[h][w][4] = (GLfloat (*)[h][w][4])malloc( (h*w*4*sizeof(GLfloat)) );
  GLfloat (*myDepths)[h][w][1] = (GLfloat (*)[h][w][1])malloc( (h*w*sizeof(GLfloat)) );
  
    if ( !hiresBuffer->bind() ) {
      WARNING ( tr("Failed to bind hiresBuffer FBO") );
    }
    
    // read colour values from hiresBuffer
    glReadPixels ( 0, 0, w, h, GL_RGBA, GL_FLOAT, myImgdata );
    
    if ( !hiresBuffer->release() ) {
      WARNING ( tr("Failed to release hiresBuffer FBO") );
    }
    
    if ( depthToAlpha ) {
      if ( !previewBuffer->bind() ) {
        WARNING ( tr("Failed to bind previewBuffer FBO") );
      }
        // read depth values from previewBuffer
        glReadPixels ( 0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, myDepths );

        if ( !previewBuffer->release() ) {
          WARNING ( tr("Failed to release previewBuffer FBO") );
        }
    }

    // put them together as RGBZ or RGBA
    for ( int i = 0; i < h; i++ ) {
      for ( int j = 0; j < w; j++ ) {
        array[ ( h-1 )-i][j] = Rgba ( (*myImgdata)[i][j][0], (*myImgdata)[i][j][1], (*myImgdata)[i][j][2],
                                      depthToAlpha ? (*myDepths)[i][j][0] : (*myImgdata)[i][j][3] );
        }
    }
    
    free(myImgdata);myImgdata=0;
    free(myDepths);myDepths=0;
    
}
#endif

void DisplayWidget::renderTile ( double pad, double time, int subframes, int w, int h, int tile, int tileMax, QProgressDialog* progress, int *steps, QImage *im ) {
    tiles = tileMax;
    tilesCount = tile;
    padding = pad;
    mainWindow->setLastStoredTime ( time*renderFPS );

    if ( hiresBuffer==0 || hiresBuffer->width() != w || hiresBuffer->height() != h) {
        QOpenGLFramebufferObjectFormat fbof;
        fbof.setAttachment ( QOpenGLFramebufferObject::Depth );
        fbof.setInternalTextureFormat ( bufferType );
        fbof.setTextureTarget ( GL_TEXTURE_2D );

        hiresBuffer = new QOpenGLFramebufferObject ( w, h, fbof );
    }
    if ( !hiresBuffer->isValid() ) {
      WARNING ( tr("Failed to create hiresBuffer FBO") );
    }
    if ( !previewBuffer->bind() ) {
      WARNING ( tr("Failed to bind previewBuffer FBO") );
    }
    glClearColor ( 0.0f,0.0f,0.0f,0.0f );
    clearGL();

    if ( !previewBuffer->release() ) {
      WARNING ( tr("Failed to release previewBuffer FBO") );
    }
    if ( !hiresBuffer->bind() ) {
      WARNING ( tr("Failed to bind hiresBuffer FBO") );
    }

    QString tmp = QString ( "%1" ).arg ( tileMax*tileMax );
    int tileField = tmp.length();
    tmp = QString ( "%1" ).arg ( subframes+1 );
    int subField = tmp.length();
    QString frametile = QString("%1.%2").arg ( tileMax*tileMax ).arg ( subframes );
    QString framesize = QString("%1x%2").arg ( tileMax*w ).arg ( tileMax*h );

    for ( int i = 0; i< subframes; i++ ) {

        if ( !progress->wasCanceled() ) {

        progress->setValue ( *steps );
        progress->setLabelText ( tr( "Frame:%1/%2 Time:%3\nTile:%4.%5/%6 Size:%7\n avg sec/tile:%8 ETA:%9" )
                                 .arg ( ( int ) ( time*renderFPS ) ).arg ( framesToRender )
                                 .arg ( time, 8, 'g', 3, QChar ( ' ' )  )
                                 .arg ( tile,tileField,10, QChar ( '0' ) )
                                 .arg ( i,subField,10,QChar ( '0' ) )
                                 .arg ( frametile )
                                 .arg ( framesize ) 
                                 .arg ( (tileAVG/(tile+1))/1000.0, 8, 'g', 3, QChar ( ' ' ) )
                                 .arg ( renderETA ) );

        ( *steps ) ++;

        drawToFrameBufferObject ( hiresBuffer, false );

        }
    }


    (*im) = hiresBuffer->toImage();

    subframeCounter=0;

    if ( !hiresBuffer->release() ) {
      WARNING ( tr("Failed to release hiresBuffer FBO") );
    }

}

void DisplayWidget::setState ( DrawingState state ) {
    drawingState = state;
}

void DisplayWidget::paintGL() {
    if ( drawingState == Tiled ) {
        return;
    }
    if ( pixelHeight() == 0 || pixelWidth() == 0 ) return;

    if ( pendingRedraws > 0 ) pendingRedraws--;

    if ( disabled || !shaderProgram ) {
        glClearColor ( backgroundColor.redF(),backgroundColor.greenF(),backgroundColor.blueF(),backgroundColor.alphaF() );
        clearGL();
        return;
    }

    if ( ( doClearBackBuffer || drawingState == DisplayWidget::Animation ) && backBuffer ) {
        if ( !previewBuffer->bind() ) {
          WARNING ( tr("Failed to bind previewBuffer FBO") );
            return;
        }
        glClearColor ( 0.0f,0.0f,0.0f,0.0f );
        clearGL();
        if ( !previewBuffer->release() ) {
          WARNING ( tr("Failed to release previewBuffer FBO") );
        }
        subframeCounter = 0;
        doClearBackBuffer = false;
    } else {
        if ( doClearBackBuffer ) {
            subframeCounter = 0;
            doClearBackBuffer = false;
            glClearColor ( 0.0f,0.0f,0.0f,0.0f );
            clearGL();
        }
    }

    if ( subframeCounter==0 && drawingState == DisplayWidget::Animation ) {
        if ( mainWindow->getVariableEditor()->hasEasing() ) {
            updateEasingCurves ( mainWindow->getFrame() ); // current frame
        }
        if ( eyeSpline != NULL ) {
            int index = mainWindow->getFrame();
            QVector3D e = eyeSpline->getSplinePoint ( index );
            QVector3D t = targetSpline->getSplinePoint ( index );
            // camera path tracking makes for a bumpy ride
            //  t = eyeSpline->getSplinePoint( index+1 );
            QVector3D u = upSpline->getSplinePoint ( index );
            if ( !e.isNull() && !t.isNull() && !u.isNull() ) {
                mainWindow->setCameraSettings ( e,t,u.normalized() ); // normalizing Up here allows spline path animating
            }
        }
    }

    if ( drawingState == DisplayWidget::Progressive ) {
        if ( subframeCounter>=maxSubFrames && maxSubFrames>0 ) {
            drawToFrameBufferObject ( 0, true );
            return;
        }
    }

    if ( previewBuffer ) {
        drawToFrameBufferObject ( 0, false );
    } else {
        drawFragmentProgram ( pixelWidth(),pixelHeight(), true );
        if ( drawingState == DisplayWidget::Progressive ) {
            subframeCounter++;
            mainWindow->setSubFrameDisplay ( subframeCounter );
        }
    }
}

void DisplayWidget::updateBuffers() {
    resizeGL ( 0,0 );
}

void DisplayWidget::resizeGL ( int /* width */, int /* height */ ) {
    // When resizing the perspective must be recalculated
    updatePerspective();
    clearPreviewBuffer();
}

void DisplayWidget::updatePerspective() {
    if ( pixelHeight() == 0 || pixelWidth() == 0 ) return;
    mainWindow->getBufferSize ( pixelWidth(), pixelHeight(),bufferSizeX, bufferSizeY, fitWindow );
    QString infoText = tr( "[%1x%2] Aspect=%3" ).arg ( pixelWidth() ).arg ( pixelHeight() ).arg ( ( double ) pixelWidth() /pixelHeight() );
    mainWindow-> statusBar()->showMessage ( infoText, 5000 );
}

void DisplayWidget::timerSignal() {
    static bool firstTime = true;
    if ( firstTime ) {
        firstTime = false;
        updatePerspective();
        requireRedraw ( true );
    }
    
    static QWidget* lastFocusedWidget = QApplication::focusWidget();
    if ( QApplication::focusWidget() !=lastFocusedWidget && cameraControl ) {
        cameraControl->releaseControl();
        lastFocusedWidget = QApplication::focusWidget();
    }
    if ( cameraControl && cameraControl->wantsRedraw() ) {
      requireRedraw ( clearOnChange );
        cameraControl->updateState();
    }

    if ( pendingRedraws ) {
      if(buttonDown) pendingRedraws=0;
        updateGL();
    } else if ( continuous ) {
        if ( drawingState == Progressive &&
                ( subframeCounter>=maxSubFrames && maxSubFrames>0 ) ) {
            // we're done rendering
        } else {
            if(buttonDown) return;
            QTime t = QTime::currentTime();
            updateGL();
            QTime cur = QTime::currentTime();
            long ms = t.msecsTo ( cur );
            fpsCounter++;
            double fps = -1;

            // If the render takes more than 0.5 seconds, we will directly measure fps from one frame.
            if ( ms>500 ) {
                fps = 1000.0f/ ( ( double ) ms );
                mainWindow->setFPS ( fps );
            } else {
                if ( drawingState == Animation ) {
                    // Else measure over two seconds.
                    long ms2 = fpsTimer.msecsTo ( cur );
                    if ( ms2>2000 || ms2<0 ) {
                        fps = fpsCounter/ ( ms2/1000.0 );
                        fpsTimer = cur;
                        fpsCounter = 0;
                        mainWindow->setFPS ( fps );
                    }
                } else {
                    mainWindow->setFPS ( 0 );
                }
            }
        }
    }
}

/// Default QGLFormat settings
//    Double buffer: Enabled.
//    Depth buffer: Enabled.
//    RGBA: Enabled (i.e., color index disabled).
//    Alpha channel: Disabled.
//    Accumulator buffer: Disabled.    should use this one: anti-aliasing, progressive render, ambient shading etc.
//    Stencil buffer: Enabled.         disable this but could be used for tile render
//    Stereo: Disabled.
//    Direct rendering: Enabled.
//    Overlay: Disabled.
//    Plane: 0 (i.e., normal plane).
//    Multisample buffers: Disabled.

void DisplayWidget::showEvent(QShowEvent * event) {

    vendor = QString ( ( char * ) glGetString ( GL_VENDOR ) );
    renderer = QString ( ( char * ) glGetString ( GL_RENDERER ) );
    /// test for nVidia card and set the nV flag
    foundnV = vendor.contains ( "NVIDIA", Qt::CaseInsensitive );

    requireRedraw ( true );

    // Show info first time...
    INFO ( vendor + " " + renderer );
    INFO ( tr("This video card supports: ") + GetOpenGLFlags().join ( ", " ) );

    QStringList extensions;

    QList<QByteArray> a = QImageWriter::supportedImageFormats();
#ifdef USE_OPEN_EXR
    a.append ( "exr" );
#endif
    foreach ( QByteArray s, a ) {
        extensions.append ( QString ( s ) );
    }
    INFO ( tr("Available output formats: ") + extensions.join ( ", " ) );

    if ( QSettings().value ( "autorun", true ).toBool() == false ) {
        WARNING ( tr("Auto run is disabled! You must select \"Build\" and apply a preset.") );
        WARNING ( tr("If the preset alters locked variables \"Build\" will be required again.") );
        mainWindow->highlightBuildButton ( true );
    }
}

void DisplayWidget::setZappaStatus(Qt::KeyboardModifiers km) {
    if( km == AddModifier) {
        wantZappaAdd = true;
    } else if( km == MinusModifier ) {
        wantZappaMinus = true;
    }  else if( km == DeleteModifier ) {
        wantZappaDelete=true;
    } else if( km == ClearModifier ) {
        wantZappaClear=true;
    }
};

void DisplayWidget::resetZappaStatus() {
    wantZappaMinus = false;
    wantZappaAdd = false;
    wantZappaDelete = false;
    wantZappaClear = false;
};

void DisplayWidget::wheelEvent ( QWheelEvent* ev ) {

    if ( mainWindow->getCameraSettings().isEmpty() ) return;

    // M Benesi "Spray gun" wheelEvent
    if( zapLocked ) {
        QStringList cs = mainWindow->getCameraSettings().split ( "\n" );
        QStringList cv = cs.filter ( "Eye " ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
        QVector3D eye = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
        ZAtMXY += (cameraControl->StepSize() * (ev->delta()/120.0) );
        QVector3D target = cameraControl->screenTo3D( ev->x(), ev->y(), ZAtMXY );
        QVector3D direction = (target-eye);
        QVector3D dir = direction.normalized();
        QVector3D offset = dir*(cameraControl->StepSize()*0.001);
        target = target+offset;
        mainWindow->setFeebackCoords( QVector3D(target.x(), target.y(), target.z()) );
        requireRedraw ( clearOnChange );
        QGLWidget::wheelEvent ( ev );
        return;
    }
    
    cameraControl->wheelEvent ( ev );
    requireRedraw ( clearOnChange );
    ev->accept();
}

void DisplayWidget::mouseMoveEvent ( QMouseEvent *ev ) {
    if ( mainWindow->getCameraSettings().isEmpty() ) return;

    // M Benesi "Spray gun" mouseMoveEvent
    mouseXY=ev->pos();
    if( zapLocked ) {
        if( buttonDown) {
            QVector3D m = cameraControl->screenTo3D( ev->x(), ev->y(), ZAtMXY );
            mainWindow->setFeebackCoords( QVector3D(m.x(), m.y(), m.z()) );
            requireRedraw ( clearOnChange );
            QGLWidget::mouseMoveEvent ( ev );
        }
        return;
    }

    if( buttonDown) {
        bool redraw = cameraControl->mouseEvent ( ev, width(), height() );
        if ( redraw ) {
            requireRedraw ( clearOnChange );
            ev->accept();
        }
    }
}

void DisplayWidget::mouseReleaseEvent ( QMouseEvent* ev )  {
    if ( mainWindow->getCameraSettings().isEmpty() ) return;

    // M Benesi "Spray gun" mouseReleaseEvent
    buttonDown=false;
    resetZappaStatus();
    if( zapLocked ) return;
    // if the user just clicked and didn't drag update the statusbar
    if ( ev->pos() == mouseXY ) {
        QVector3D mXYZ = cameraControl->screenTo3D( mouseXY.x(), mouseXY.y(), ZAtMXY );
        // update statusbar
        mainWindow->statusBar()->showMessage ( QString ( "X:%1 Y:%2 Z:%3" ).arg ( mXYZ.x() ).arg ( mXYZ.y() ).arg ( mXYZ.z() ) );
        if(ev->button() == Qt::MiddleButton) {
          // SpotLightDir = polar coords vec2 DE-Raytracer.frag
          // LightPos = vec3 DE-Kn2.frag
          if(ev->modifiers() == Qt::ControlModifier)
            // placement of light in DE-Kn2.frag
            mainWindow->setParameter( QString("LightPos = %1,%2,%3").arg(mXYZ.x()).arg(mXYZ.y()).arg(mXYZ.z()) );
          else
            // placement of target
            mainWindow->setParameter( QString("Target = %1,%2,%3").arg(mXYZ.x()).arg(mXYZ.y()).arg(mXYZ.z()) );
        }
    }
    // normal process when mouse is released after moving
    bool redraw = cameraControl->mouseEvent ( ev, width(), height() );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    }
}

void DisplayWidget::mousePressEvent ( QMouseEvent* ev )  {
    buttonDown=true;
    if ( mainWindow->getCameraSettings().isEmpty() ) return;

    // M Benesi "Spray gun" mousePressEvent
    if( zapLocked ) // this is where ZAtMXY needs to be set to current zappa dist from camera
      return;
    if( ev->button() == Qt::LeftButton) {
        setZappaStatus( ev->modifiers() );
    }

    mouseXY=ev->pos();

    bool redraw = cameraControl->mouseEvent ( ev, width(), height() );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    }
    
//     mainWindow->statusBar()->showMessage ( QString ( "X:%1 Y:%2 Z:%3" ).arg ( mXYZ.x() ).arg ( mXYZ.y() ).arg ( mXYZ.z() ) );
}

void DisplayWidget::keyPressEvent ( QKeyEvent* ev ) {
    if ( mainWindow->getCameraSettings().isEmpty() ) return;

    if( zapLocked ) {
        QGLWidget::keyPressEvent ( ev );
        return;
    }

    bool redraw = cameraControl->keyPressEvent ( ev );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    } else {
        QGLWidget::keyPressEvent ( ev );
    }
}

void DisplayWidget::keyReleaseEvent ( QKeyEvent* ev ) {
    if ( mainWindow->getCameraSettings().isEmpty() ) return;

    if( zapLocked ) {
        int fbi = mainWindow->getFeedbackIndex();
        int fbmi = mainWindow->getFeedbackMaxIndex();
        int fbc = mainWindow->getFeedbackCount();

        if(ev->key() == Qt::Key_Left) fbi--;
        else if(ev->key() == Qt::Key_Right) fbi++;

        if(fbi+1 > fbc) fbi = fbc;
        else if(fbi+1 > fbmi) fbi = 1;
        else if(fbi-1 < 1) fbi = 1;

        if(ev->key() == Qt::Key_Left || ev->key() == Qt::Key_Right) mainWindow->setFeedbackId( fbi );

        QGLWidget::keyReleaseEvent ( ev );
        return;
    }

    bool redraw = cameraControl->keyPressEvent ( ev );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    } else {
        QGLWidget::keyReleaseEvent ( ev );
    }
}

void DisplayWidget::clearPreviewBuffer() {
    setPreviewFactor ( previewFactor );
    makeBuffers();
    requireRedraw ( true );
}

void DisplayWidget::updateEasingCurves ( int currentframe ) {
    static int cf = 0; // prevent getting called more than once per frame ?
    if ( cf == currentframe ) return;
    else cf = currentframe;

    int count = curveSettings.count();

    for ( int i = 0; i < count; i++ ) {

        QString wName = curveSettings[i].split ( ":" ).first();
        QString wNum = wName.right ( 1 );
        wName = wName.left ( wName.length()-1 );
        ComboSlider *cs = ( ComboSlider* ) ( mainWindow->getVariableEditor()->findChild<ComboSlider*> ( QString ( "%1%2" ).arg ( wName ).arg ( wNum ) ) );

        if ( cs->m_anim != NULL ) {
            cs->blockSignals ( true );
            int animframe = currentframe-cs->m_framestart;
            int loopduration = cs->m_framefin-cs->m_framestart;
            int singleframe = ( 1.0/renderFPS ) *1000; // in msecs
            int maxttime = ( int ) ( mainWindow->getTimeMax() *renderFPS );
            int endframe = cs->m_framefin;
            if ( cs->m_loops > 0 ) endframe*=cs->m_loops;
            
            // test end frame against maxTime warn and stop if greater
            if ( endframe > maxttime ) {
              VariableWidget *vw = ( VariableWidget* ) ( mainWindow->getVariableEditor()->findChild<VariableWidget*> ( wName ) );
              QString gr;
              if ( vw ) gr = vw->getGroup();
              else gr = tr("Fix me");
              
              QString mess = tr("Easingcurve end frame %1 is greater than maximum %2 \
              <br><br>Select <b>%3 -> %4</b> and press \"F7\" \
              <br><br>Or set the <b>animation length</b> greater than <b>%5</b>" )
              .arg ( endframe ).arg ( maxttime ).arg ( gr ).arg ( wName ).arg ( ( int ) ( ( cs->m_framestart + endframe ) /renderFPS ) );
              QMessageBox::warning(this, tr("Easing Curve Error"),
                                    mess);
              
              mainWindow->setTimeSliderValue ( currentframe );
              mainWindow->stop();
              
              return;
            }
            if ( currentframe < cs->m_framestart ) {
              cs->m_anim->setCurrentTime ( 1 ); // preserve the startvalue for prior frames
            } else if ( currentframe < cs->m_framestart + ( loopduration * cs->m_loops ) ) { // active!
              
              if ( cs->m_pong ) { // pingpong = 1 loop per so 4 loops = ping pong ping pong
                
                int test = ( animframe/loopduration ); // for even or odd loop
                
                if ( ( int ) ( test*0.5 ) *2 == test ) {
                  cs->m_anim->setCurrentTime ( ( animframe*singleframe ) +1 );
                } else {
                  cs->m_anim->setCurrentTime ( ( ( ( loopduration*cs->m_loops )-animframe ) *singleframe )-1 );
                }
              } else cs->m_anim->setCurrentTime ( ( animframe*singleframe )-1 );
            } else { // preserve the end value for remaining frames
              if ( cs->m_pong && ( int ) ( cs->m_loops*0.5 ) *2 == cs->m_loops ) { // even or odd loop
                cs->m_anim->setCurrentTime ( 1 );
              } else {
                cs->m_anim->setCurrentTime ( cs->m_anim->duration()-1 );
              }
            }
            cs->blockSignals ( false );
        }
    }
}

void DisplayWidget::drawLookatVector() {
    QVector3D ec = eyeSpline->getSplinePoint ( mainWindow->getTime() +1 );
    QVector3D tc = targetSpline->getSplinePoint ( mainWindow->getTime() +1 );
    glColor4f ( 1.0,1.0,0.0,1.0 );
    glBegin ( GL_LINE_STRIP );
    glVertex3f ( ec.x(),ec.y(),ec.z() );
    glVertex3f ( tc.x(),tc.y(),tc.z() );
    glEnd();
}

void DisplayWidget::setPerspective() {

    QStringList cs = mainWindow->getCameraSettings().split ( "\n" );
    double fov = cs.filter ( "FOV" ).at ( 0 ).split ( "=" ).at ( 1 ).toFloat();
    QStringList cv = cs.filter ( "Eye " ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    QVector3D eye = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
    cv = cs.filter ( "Target" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    QVector3D target = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
    cv = cs.filter ( "Up" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    QVector3D up = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
    
    double ascpectRatio = double( ( double ) width() / ( double ) height() );
    double zNear = 0.00001;
    double zFar = 1000.0;
    double vertAngle = 180.0 * ( 2.0 * atan2 ( 1.0, ( 1.0/fov ) ) / M_PI );
    
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.perspective ( vertAngle, ascpectRatio, zNear, zFar );
    matrix.lookAt ( eye,target,up );

    /// BEGIN 3DTexture
    //     texMatrix = matrix;
    /// END 3DTexture
    
    glLoadMatrixf ( matrix.constData() );
}

QStringList DisplayWidget::getCurveSettings() {
    return curveSettings;
}

void DisplayWidget::setCurveSettings ( QStringList cset ) {
    mainWindow->getVariableEditor()->setEasingEnabled ( !cset.isEmpty() );
    curveSettings = cset;
}

void DisplayWidget::drawSplines() {
    // control point to highlight = currently selected preset keyframe
    int p = mainWindow->getCurrentCtrlPoint();

    targetSpline->drawSplinePoints();
    targetSpline->drawControlPoints ( p );

    eyeSpline->drawSplinePoints();
    // TODO add Up vectors to spline control points and enable editing of points with mouse
    eyeSpline->drawControlPoints ( p );
}

QVector3D *DisplayWidget::getControlPoints ( int i ) {
    if ( i==1 ) return ( QVector3D * ) eyeControlPoints.constData();
    else if ( i==2 ) return ( QVector3D * ) targetControlPoints.constData();
    else if ( i==3 ) return ( QVector3D * ) upControlPoints.constData();
    return NULL;
}

void DisplayWidget::addControlPoint ( QVector3D eP, QVector3D tP, QVector3D uP ) {
    eyeControlPoints.append ( eP );
    targetControlPoints.append ( tP );
    upControlPoints.append ( uP );
}

void DisplayWidget::clearControlPoints() {
    eyeControlPoints.clear();
    targetControlPoints.clear();
    upControlPoints.clear();
}
}
}

