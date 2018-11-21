#include <stdio.h>

#include <qopengl.h>
#include <qopenglext.h>

#include <QWheelEvent>
#include <QStatusBar>
#include <QMenu>
#include <QVector2D>
#include <QFileInfo>
#include <QMatrix4x4>
#include <QVector3D>

#include "DisplayWidget.h"

#include "MainWindow.h"
#include "VariableWidget.h"
#include "../../ThirdPartyCode/hdrloader.h"

#ifdef USE_OPEN_EXR
using namespace Imf;
using namespace Imath;
#endif

using namespace SyntopiaCore::Logging;

namespace Fragmentarium {
namespace GUI {

using namespace Parser;

// use int64_t to allow renders longer than 24.8 days without overflowing
int64_t computeETA( int64_t elapsed, int64_t total, int64_t current ) {
    return elapsed * (total - current) / (double) current;
}

QString formatTime( int64_t estRenderMS ) {
    QTime t(0,0,0,0);
    const int day = 24 * 60 * 60 * 1000;
    int days = estRenderMS / day;
    estRenderMS %= day;
    t=t.addMSecs((int) estRenderMS);
    return QString(" %1:%2").arg(days).arg(t.toString("hh:mm:ss.zzz"));
}

DisplayWidget::DisplayWidget ( MainWindow* mainWin, QWidget* parent )
    : QOpenGLWidget ( parent ), mainWindow ( mainWin ) {

    verbose = false;
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
    /// BEGIN 3DTexture
    //     m3DTexId = 0;
    /// END 3DTexture
    buttonDown = false;
    updateRefreshRate();
    }

void DisplayWidget::initializeGL() {
         initializeOpenGLFunctions();
    vendor = QString ( ( char * ) glGetString ( GL_VENDOR ) );
    renderer = QString ( ( char * ) glGetString ( GL_RENDERER ) );
    /// test for nVidia card and set the nV flag
    foundnV = vendor.contains ( "NVIDIA", Qt::CaseInsensitive );

}

void DisplayWidget::updateRefreshRate() {
    QSettings settings;
    int i = settings.value ( "refreshRate", 20 ).toInt();
    if ( !timer ) {
        timer = new QTimer();
        connect ( timer, SIGNAL ( timeout() ), this, SLOT ( timerSignal() ) );
    }
    if(i==0)i=1; // must be at least 1ms or GPU blocks GUI refresh.
    timer->start ( i );
    renderFPS = settings.value ( "fps", 25 ).toInt();
    INFO ( tr( "Setting display update timer to %1 ms (max %2 FPS)." ).arg ( i ).arg ( 1000.0/i,0,'f',2 ) );

}

// let the system handle widget paint events,with out shaders
void DisplayWidget::paintEvent(QPaintEvent * ev) {

    if ( drawingState == Tiled ) {
        return;
    }
    if ( bufferShaderProgram ) {
        bufferShaderProgram->release();
    }
    if ( shaderProgram ) {
        shaderProgram->release();
    }
    QOpenGLWidget::paintEvent(ev);
}

DisplayWidget::~DisplayWidget() {
}

void DisplayWidget::contextMenuEvent(QContextMenuEvent* ev ) {
    if(ev->modifiers() == Qt::ShiftModifier)
        if( mainWindow->isFullScreen()) {
            contextMenu->show();
//            QPoint myPos((width()-contextMenu->width())/2,(height()-contextMenu->height())/2);
            contextMenu->move( mapToGlobal( ev->pos() ));
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
    initFragmentShader();    
}

void DisplayWidget::requireRedraw ( bool clear ) {
    if ( disableRedraw ) return;

    if ( clear ) {
        clearBackBuffer();
        pendingRedraws = 1;
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

if(context()->format().majorVersion() > 2)
            glGenerateMipmap ( GL_TEXTURE_2D ); //Generate mipmaps here!!!
else
            glTexParameteri ( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE ); //Generate mipmaps here!!!

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

// this function parses the assembler text and returns a string list containing the lines
QStringList DisplayWidget::shaderAsm ( bool w ) {
    
    if(!foundnV) return QStringList("nVidia gfx card required for this feature!");
    
    GLuint progId = w ? shaderProgram->programId() : bufferShaderProgram->programId();
    GLint formats = 0;
    glGetIntegerv ( GL_NUM_PROGRAM_BINARY_FORMATS, &formats );
    GLint binaryFormats[formats];
    glGetIntegerv ( GL_PROGRAM_BINARY_FORMATS, binaryFormats );

    GLint len=0;
    glGetProgramiv ( progId, GL_PROGRAM_BINARY_LENGTH, &len );
    uchar binary[ size_t(len+1) ];

    glGetProgramBinary ( progId, len, NULL, ( GLenum* ) binaryFormats, &binary[0] );

    QString asmTxt = "";
    QStringList asmList;

    // contains ALL uniforms in buffershader and shader program
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();
    bool foundFirstUniform=false;
    bool foundLastUniform=false;

    for ( int x=0; x<len-1; x++ ) {
        if ( isprint ( binary[x] ) ) {
            asmTxt += binary[x];
        }

        if ( !asmList.isEmpty() && asmList.last() == "END" && !asmTxt.startsWith ( "!" ) ) {
            asmTxt="";
        }

        if ( !asmTxt.isEmpty() && binary[x] == 10) { // end of assembler statement
                if(foundFirstUniform==true) asmList << asmTxt;
            asmTxt="";
            }

        // this locates all of the uniform names not optimized out by the compiler
        if ( !asmTxt.isEmpty() && binary[x] == 0  ) { // end of string literal
            int uLoc = w ? shaderProgram->uniformLocation ( asmTxt ) : bufferShaderProgram->uniformLocation ( asmTxt );
            if(uLoc != -1) {
                if(uLoc == 0)foundFirstUniform=true;
                if(foundFirstUniform==true) asmList << asmTxt;
            } else { if( foundFirstUniform==true ) foundLastUniform=true; }

            asmTxt="";
        }
        
        if(!asmList.isEmpty() && foundLastUniform && asmList.last().length() == 1) asmList.removeLast();
        
    }

    // find a value to go with the name, index in the program may not be the same as index in our list
    for( int n=0; n < vw.count(); n++) {
        asmTxt = vw[n]->getName();
        if( asmList.indexOf( asmTxt ) != -1 ){
            int uLoc = w ? shaderProgram->uniformLocation ( asmTxt ) : bufferShaderProgram->uniformLocation ( asmTxt );
            if(uLoc != -1) {
                QString newLine = QString("%1").arg(vw[n]->getLockedSubstitution());
                asmList[ asmList.indexOf(vw[n]->getName()) ] = newLine;
            }
        }
    }

// uncomment this if you want to save the binaries for hexeditor? disassembler?
//     if(verbose) {
//         QString fileName = w ? "RenderShader.bin" : "BufferShader.bin";
//         QFile file(fileName);
//         if (!file.open(QFile::WriteOnly)) {
//             QMessageBox::warning(this, tr("Fragmentarium"),
//                                 tr("Cannot write Shader binary %1:\n%2.")
//                                 .arg(fileName)
//                                 .arg(file.errorString()));
//         return asmList;
//         }
// 
//         file.write( ( char * )( &binary ), len );
//         file.close();
//         INFO(tr("Binary shader saved to file:")+fileName);
//     }

    return asmList;
}

void DisplayWidget::initFragmentShader() {

    QImage im;

    if ( shaderProgram ) {
        shaderProgram->release();
        delete ( shaderProgram );
        shaderProgram = 0;
    }
    
    shaderProgram = new QOpenGLShaderProgram ( this );

    // Vertex shader
    bool s = shaderProgram->addShaderFromSourceCode ( QOpenGLShader::Vertex,fragmentSource.vertexSource.join ( "\n" ) );
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
    else if ( !shaderProgram->log().isEmpty() ) INFO ( tr("Fragment shader compiled with warnings: ") + shaderProgram->log() );

    s = shaderProgram->link();

    if ( !s ) {
        WARNING( tr("Could not link vertex + fragment shader: ") );
        CRITICAL( shaderProgram->log() );
        delete ( shaderProgram );
        shaderProgram = 0;
        return;
    }
    else if ( !shaderProgram->log().isEmpty() ) INFO ( tr("Fragment shader compiled with warnings: ") + shaderProgram->log() );

    s = shaderProgram->bind();
    if ( !s ) {
        WARNING ( tr("Could not bind shaders: ") + shaderProgram->log() );
        delete ( shaderProgram );
        shaderProgram = 0;
        return;
    }

    // Setup textures.
    int u = 0;
    if ( bufferType != 0 ) {
        makeCurrent();
        // Bind first texture to backbuffer
        glActiveTexture ( GL_TEXTURE0+u ); // non-standard (>OpenGL 1.3) gl extension
        int l = shaderProgram->uniformLocation ( "backbuffer" );
        if ( l != -1 ) {
              GLuint i = backBuffer->texture();
              glBindTexture ( GL_TEXTURE_2D,i );
              if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                  setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
              }
              shaderProgram->setUniformValue ( l, ( GLuint ) u );
              if(verbose && subframeCounter == 1) qDebug() << QString("Binding back buffer (ID: %1) to active texture %2").arg(backBuffer->texture()).arg(u);
              if(verbose && subframeCounter == 1) qDebug() << QString("Setting uniform backbuffer to active texture %2").arg(u);
        }
        u++;
    } else {
        WARNING ( tr("Trying to use a backbuffer, but no bufferType set.") );
        WARNING ( tr("Use the buffer define, e.g.: '#buffer RGBA8' ") );
    }

    GLuint textureID=u;

    QMapIterator<QString, QString> it( fragmentSource.textures );
    while( it.hasNext() ) { it.next();

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

                if(verbose) qDebug() << QString ( "Allocated texture ID: %1" ).arg ( textureID );
                
                glBindTexture ( (type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID );

                if ( texturePath.endsWith ( ".hdr", Qt::CaseInsensitive ) ) { // is HDR format image ?
                    HDRLoaderResult result;
                    if ( HDRLoader::load ( texturePath.toLatin1().data(), result ) ) {
                        if(type == GL_SAMPLER_CUBE) {
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*0*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*1*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*2*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*3*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*4*3] );
                            glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width*5*3] );
                        } else
                            glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, result.width, result.height, 0, GL_RGB, GL_FLOAT, result.cols );
                        loaded = true;
                    }
                }
#ifdef USE_OPEN_EXR
#ifdef Q_OS_WIN
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
                    
                    if(type == GL_SAMPLER_CUBE) {
                        QImage t = im.convertToFormat(QImage::Format_RGBA8888);
                        
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*0) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*1) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*2) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*3) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*4) );
                        glTexImage2D( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width()*5) );
                    } else {
                      QImage t = im.mirrored().convertToFormat(QImage::Format_RGBA8888);
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

                if(verbose) qDebug() << QString("Setting uniform %1 to active texture %2").arg(textureName).arg(u);
                
                shaderProgram->setUniformValue ( l, ( GLuint ) u );
                u++;
            } else  WARNING ( tr("Not a valid texture: ") + QFileInfo ( texturePath ).absoluteFilePath() );
        } else WARNING ( tr("Unused sampler uniform: ") + textureName );
        
    }
    initBufferShader();
    
        // Check for unused textures
        QMapIterator<QString, int> i ( TextureCache );
        while ( i.hasNext() ) {
            i.next();
            if ( !textureCacheUsed.contains ( i.key() ) ) {
                INFO ( tr("Removing unused texture from cache: ") +i.key() );

                if( glIsTexture( i.value() ) ) {
                    glDeleteTextures(1, (GLuint*)&i.value() );
                    TextureCache.remove ( i.key() );
                }
            }
        }
}

void DisplayWidget::clearTextureCache () {

        QMapIterator<QString, int> i ( TextureCache );
        while ( i.hasNext() ) {
            i.next();
            INFO ( tr("Removing texture from cache: ") +i.key() );

            if( glIsTexture( i.value() ) ) {
                glDeleteTextures(1, (GLuint*)&i.value() );
                TextureCache.remove ( i.key() );
                textureCacheUsed.remove ( i.key() );
            }
            
        }

        TextureCache.clear();
}


void DisplayWidget::initBufferShader() {
  
    if ( bufferShaderProgram ) {
        bufferShaderProgram->release();
        delete ( bufferShaderProgram );
        bufferShaderProgram = 0;
    }

    if ( !fragmentSource.bufferShaderSource ) return;

    bufferShaderProgram = new QOpenGLShaderProgram ( this );

    // Vertex shader
    bool s = bufferShaderProgram->addShaderFromSourceCode ( QOpenGLShader::Vertex,fragmentSource.bufferShaderSource->vertexSource.join ( "\n" ) );
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

    if(context()->format().majorVersion() > 3 && context()->format().minorVersion() > 0) {
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
            qDebug( ) << tr("FBO Incomplete Error!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, defaultFramebufferObject());
    }
    else glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
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
//   float fov = cs.filter ( "FOV" ).at ( 0 ).split ( "=" ).at ( 1 ).toDouble();
//   QStringList cv = cs.filter ( "Eye " ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   QVector3D eye = QVector3D ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
//   cv = cs.filter ( "Target" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   QVector3D target = QVector3D ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
//   cv = cs.filter ( "Up" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   QVector3D up = QVector3D ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
//   
//   float aspectRatio = float( ( float ) width() / ( float ) height() );
//   float zNear = 0.00001;
//   float zFar = 1000.0;
//   float vertAngle = 180.0 * ( 2.0 * atan2 ( 1.0, ( 1.0/fov ) ) / M_PI );
//   
//   QMatrix4x4 matrix;
//   matrix.setToIdentity();
//   matrix.perspective ( vertAngle, aspectRatio, zNear, zFar );
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

    // this should speed things up a litte because we are only setting uniforms on the first subframe of the first tile
    if(subframeCounter > 1) return;
    if(tilesCount > 1) return;
    
    // contains ALL uniforms in buffershader and shader program
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();

    GLuint programID = shaderProg->programId();
   
    int count;
    glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);

    if(subframeCounter == 1 && verbose) {
        if(programID == shaderProgram->programId()) {
            qDebug() << "\nshaderProgram";
        }
        
        if(hasBufferShader() && programID == bufferShaderProgram->programId()) {
            qDebug() << "\nbufferShaderProgram";
        }
    }

    for (int i = 0; i < count; i++) {

        GLsizei bufSize = 256;
        GLsizei length;
        GLint size;
        GLenum type;
        GLchar name[bufSize];

        glGetActiveUniform( programID, (GLuint)i, bufSize, &length, &size, &type, name);        
        QString uniformName = (char *)name;
        QString uniformValue = "";
        if(uniformName.startsWith("gl_")) uniformValue = "OpenGL variable";
        if(uniformName == "pixelSize") uniformValue = "Special variable";
        if(uniformName == "subframe") uniformValue = "Special variable";
        if(uniformName == "frontbuffer") uniformValue = "Special variable";
        if(uniformName == "backbuffer") uniformValue = "Special variable";
        if(uniformName == "time") uniformValue = "Special variable";

        // find a value to go with the name index in the program may not be the same as index in our list
        for( int n=0; n < vw.count(); n++) {
            if(uniformName == vw[n]->getName()) {
                // get slider values
                uniformValue = vw[n]->getValueAsText();
                // test and get filename
                if(uniformValue.isEmpty()) uniformValue = vw[n]->toString();
                break;
            }
        }

        if(uniformValue.isEmpty()) uniformValue = "Unused variable widget";

        QString tp;
        bool foundDouble = false;

          switch(type) {

                case GL_BYTE:           tp = "BYTE "; foundDouble = false; break;
                case GL_UNSIGNED_BYTE:  tp = "UNSIGNED_BYTE"; foundDouble = false; break;
                case GL_SHORT:          tp = "SHORT"; foundDouble = false; break;
                case GL_UNSIGNED_SHORT: tp = "UNSIGNED_SHORT"; foundDouble = false; break;
                case GL_INT:            tp = "INT  "; foundDouble = false; break;
                case GL_UNSIGNED_INT:   tp = "UNSIGNED_INT"; foundDouble = false; break;
                case GL_FLOAT:          tp = "FLOAT"; foundDouble = false; break;
                case GL_FIXED:          tp = "FIXED"; foundDouble = false; break;
                case GL_FLOAT_VEC2:     tp = "FLOAT_VEC2"; foundDouble = false; break;
                case GL_FLOAT_VEC3:     tp = "FLOAT_VEC3"; foundDouble = false; break;
                case GL_FLOAT_VEC4:     tp = "FLOAT_VEC4"; foundDouble = false; break;
                case GL_INT_VEC2:       tp = "INT_VEC2"; foundDouble = false; break;
                case GL_INT_VEC3:       tp = "INT_VEC3"; foundDouble = false; break;
                case GL_INT_VEC4:       tp = "INT_VEC4"; foundDouble = false; break;
                case GL_BOOL:           tp = "BOOL "; foundDouble = false; break;
                case GL_BOOL_VEC2:      tp = "BOOL_VEC2"; foundDouble = false; break;
                case GL_BOOL_VEC3:      tp = "BOOL_VEC3"; foundDouble = false; break;
                case GL_BOOL_VEC4:      tp = "BOOL_VEC4"; foundDouble = false; break;
                case GL_FLOAT_MAT2:     tp = "FLOAT_MAT2"; foundDouble = false; break;
                case GL_FLOAT_MAT3:     tp = "FLOAT_MAT3"; foundDouble = false; break;
                case GL_FLOAT_MAT4:     tp = "FLOAT_MAT4"; foundDouble = false; break;
                case GL_SAMPLER_2D:     tp = "SAMPLER_2D"; foundDouble = false; break;
                case GL_SAMPLER_CUBE:   tp = "SAMPLER_CUBE"; foundDouble = false; break;
            }

        if(context()->format().majorVersion() > 3 && context()->format().minorVersion() > 0) {
          double x,y,z,w;
          GLuint index = glGetUniformLocation(programID, name);
          switch(type) {
                case GL_DOUBLE:         tp = "DOUBLE"; foundDouble = true;
                glUniform1d(index, uniformValue.toDouble());
                break;
                case GL_DOUBLE_VEC2:    tp = "DOUBLE_VEC2"; foundDouble = true;
                x = uniformValue.split(",").at(0).toDouble();
                y = uniformValue.split(",").at(1).toDouble();
                glUniform2d(index, x, y);
                break;
                case GL_DOUBLE_VEC3:    tp = "DOUBLE_VEC3"; foundDouble = true;
                x = uniformValue.split(",").at(0).toDouble();
                y = uniformValue.split(",").at(1).toDouble();
                z = uniformValue.split(",").at(2).toDouble();
                glUniform3d(index, x, y, z);
                break;
                case GL_DOUBLE_VEC4:    tp = "DOUBLE_VEC4"; foundDouble = true;
                x = uniformValue.split(",").at(0).toDouble();
                y = uniformValue.split(",").at(1).toDouble();
                z = uniformValue.split(",").at(2).toDouble();
                w = uniformValue.split(",").at(3).toDouble();
                glUniform4d(index, x, y, z, w);
                break;
                case GL_DOUBLE_MAT2:    tp = "DOUBLE_MAT2"; foundDouble = true; break;
                case GL_DOUBLE_MAT3:    tp = "DOUBLE_MAT3"; foundDouble = true; break;
                case GL_DOUBLE_MAT4:    tp = "DOUBLE_MAT4"; foundDouble = true; break;
                case GL_DOUBLE_MAT2x3:  tp = "DOUBLE_MAT2x3"; foundDouble = true; break;
                case GL_DOUBLE_MAT2x4:  tp = "DOUBLE_MAT2x4"; foundDouble = true; break;
                case GL_DOUBLE_MAT3x2:  tp = "DOUBLE_MAT3x2"; foundDouble = true; break;
                case GL_DOUBLE_MAT3x4:  tp = "DOUBLE_MAT3x4"; foundDouble = true; break;
                case GL_DOUBLE_MAT4x2:  tp = "DOUBLE_MAT4x2"; foundDouble = true; break;
                case GL_DOUBLE_MAT4x3:  tp = "DOUBLE_MAT4x3"; foundDouble = true; break;
                default:
                break;
            }
        }

            // type name and value to console
            if(subframeCounter == 1 && verbose) qDebug() << tp << "\t" << uniformName << uniformValue;
            // this sets User (32 bit) uniforms not handled above
            if(!foundDouble) {
                for( int n=0; n < vw.count(); n++) {
                    if(uniformName == vw[n]->getName()) {
                        vw[n]->setIsDouble(false); // ensure sliders set to float decimals
                        vw[n]->setUserUniform(shaderProg);
                        break;
                    }
                }
            }
        
        if(foundDouble) vw[i]->setIsDouble(true); // this takes care of buffershader (Post) sliders :D
    }
    if(subframeCounter == 1 && verbose) qDebug() << " ";
    
}

void DisplayWidget::drawFragmentProgram ( int w,int h, bool toBuffer ) {

    if(verbose && subframeCounter == 1) {
        static int c = 0;
        qDebug() << QString("Draw fragment program: %1").arg(c++);
    }
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
        glActiveTexture ( GL_TEXTURE0 ); // non-standard (>OpenGL 1.3) gl extension
        GLuint i = backBuffer->texture();
        glBindTexture ( GL_TEXTURE_2D,i );
        l = shaderProgram->uniformLocation ( "backbuffer" );
        if ( l != -1 ) {
            if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
            }
            shaderProgram->setUniformValue ( l, 0 );
            if(verbose && subframeCounter == 1) qDebug() << QString("Binding backbuffer (ID: %1) to active texture %2").arg(i).arg(0);
            if(verbose && subframeCounter == 1) qDebug() << QString("Setting uniform backbuffer to active texture %2").arg(0);
        }

        l = shaderProgram->uniformLocation ( "subframe" );
        if ( l != -1 ) {
            shaderProgram->setUniformValue ( l, subframeCounter );
            // if(verbose) qDebug() << QString("Setting subframe: %1").arg(subframeCounter);

        }
    }

    // Setup User Uniforms
    setShaderUniforms(shaderProgram);   
   
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
    
    glFinish(); // wait for GPU to return control

    if (cameraControl->getID() == "3D") {
        // draw splines using depth buffer for occlusion... or not
        if ( mainWindow->wantPaths()  && !isContinuous()) {
            if ( eyeSpline!=NULL && drawingState != Animation && drawingState != Tiled ) {
                if ( mainWindow->wantSplineOcc() ) {
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
        if(subframeCounter == 1) {
            float zatmxy;
            glReadPixels ( mouseXY.x(), height() - mouseXY.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zatmxy );
            ZAtMXY = zatmxy;
        }
    }
    // restore state
    glPopAttrib();
}

void DisplayWidget::drawToFrameBufferObject ( QOpenGLFramebufferObject* buffer, bool drawLast ) {
    if(verbose && subframeCounter == 1) {
        static int c = 0;
        qDebug() << QString("drawToFrameBufferObject: %1").arg(c++);
    }
    
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
                makeCurrent();
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


    if(drawingState != Tiled)
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

        setShaderUniforms ( bufferShaderProgram );
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

    glFinish(); // wait for GPU to return control
    
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
        bool relcheck = hiresBuffer->release();
        if(relcheck) {
          delete ( hiresBuffer );
          hiresBuffer = 0;
        } else WARNING( tr("Failed to release hires buffer"));
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
    glClear ( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
}

#ifdef USE_OPEN_EXR
void DisplayWidget::getRGBAFtile ( Array2D<Rgba>&array, int w, int h ) {

  GLfloat *myImgdata = (GLfloat *)malloc( (h*w*4*sizeof(GLfloat)) );
  GLfloat *myDepths = (GLfloat *)malloc( (h*w*sizeof(GLfloat)) );
  
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
        array[ ( h-1 )-i][j] = Rgba ( myImgdata[((i * w) + j) * 4 + 0], myImgdata[((i * w) + j) * 4 + 1], myImgdata[((i * w) + j) * 4 + 2],
                                      depthToAlpha ? myDepths[((i * w) + j) * 1 + 0] : myImgdata[((i * w) + j) * 4 + 3] );
        }
    }
    
    free(myImgdata);myImgdata=0;
    free(myDepths);myDepths=0;
    
}
#endif

void DisplayWidget::renderTile ( double pad, double time, int subframes, int w, int h, int tile, int tileMax, QProgressDialog* progress, int64_t *steps, QImage *im, QTime &refresh, QTime &totalTime ) {
    tiles = tileMax;
    tilesCount = tile;
    padding = pad;
    mainWindow->setLastStoredTime ( time*renderFPS );

    makeCurrent();
    
    if ( !previewBuffer->bind() ) {
      WARNING ( tr("Failed to bind previewBuffer FBO") );
    }
    glClearColor ( 0.0f,0.0f,0.0f,0.0f );
    clearGL();

    if ( !previewBuffer->release() ) {
      WARNING ( tr("Failed to release previewBuffer FBO") );
    }
    
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
    if ( !hiresBuffer->bind() ) {
      WARNING ( tr("Failed to bind hiresBuffer FBO") );
    }

    QString tmp = QString ( "%1" ).arg ( tileMax*tileMax );
    int tileField = tmp.length();
    tmp = QString ( "%1" ).arg ( subframes+1 );
    int subField = tmp.length();
    QString frametile = QString("%1.%2").arg ( tileMax*tileMax ).arg ( subframes );
    QString framesize = QString("%1x%2").arg ( tileMax*w ).arg ( tileMax*h );
    
    progress->setWindowTitle(tr( "Frame:%1/%2 Time:%3" )
                                    .arg ( ( int ) ( time*renderFPS ) ).arg ( framesToRender )
                                    .arg ( time, 8, 'g', 3, QChar ( ' ' )  )
        
    );

    for ( int i = 0; i< subframes; i++ ) {

        if ( !progress->wasCanceled() ) {

            // 30Hz display update frequency should be enough for humans
            if ( refresh.elapsed() > 1000/30 ) {
                refresh.restart();
                int64_t eta = computeETA( totalTime.elapsed(), progress->maximum(), *steps );
                renderETA = formatTime( eta );
                progress->setValue ( *steps );
                progress->setLabelText ( tr( "Tile:%1.%2\nof %3\nSize:%4\navg/sub:%5\navg/tile:%6\navg/frame:%7\nETA:%8" )
                                        .arg ( tile,tileField,10, QChar ( '0' ) )
                                        .arg ( i,subField,10,QChar ( '0' ) )
                                        .arg ( frametile )
                                        .arg ( framesize )
                                        .arg ( subTime / (double) subCount / 1000.0 )
                                        .arg ( tileTime / (double) tileCount / 1000.0 )
                                        .arg ( frameTime / (double) frameCount / 1000.0 )
                                        .arg ( renderETA ) );
                mainWindow->processGuiEvents();
            }
            
            ( *steps ) ++;

            drawToFrameBufferObject ( hiresBuffer, false );

            subCount += 1;
            subTime = totalTime.elapsed();

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
    
    mainWindow->getVariableEditor()->dockChanged( (mainWindow->getVariableEditor()->width() > mainWindow->getVariableEditor()->height()*2) );

}

void DisplayWidget::updatePerspective() {
    if ( pixelHeight() == 0 || pixelWidth() == 0 ) return;
    mainWindow->getBufferSize ( pixelWidth(), pixelHeight(),bufferSizeX, bufferSizeY, fitWindow );
    QString infoText = tr( "[%1x%2] Aspect=%3" ).arg ( pixelWidth() ).arg ( pixelHeight() ).arg ( ( double ) pixelWidth() /pixelHeight() );
    mainWindow-> statusBar()->showMessage ( infoText, 5000 );
}

void DisplayWidget::timerSignal() {

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
        update();
    } else if ( continuous ) {
        if ( drawingState == Progressive &&
                ( subframeCounter>=maxSubFrames && maxSubFrames>0 ) ) {
            // we're done rendering
        } else {
            if(buttonDown) return;
            QTime t = QTime::currentTime();
            
            // render
            update();
            QTime cur = QTime::currentTime();
            long ms = t.msecsTo ( cur );
            fpsCounter++;
            double fps = -1;

            // If the render takes more than 0.5 seconds, we will directly measure fps from one frame.
            if ( ms>500 ) {
                fps = 1000.0 / ( ( double ) ms );
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

    mainWindow->processGuiEvents();

}

void DisplayWidget::showEvent(QShowEvent * ) {

    // Show info first... when NO-auto-run
    INFO ( vendor + " " + renderer );

    requireRedraw ( true );

    // report the version and profile that was actually created in the engine
    int prof = context()->format().profile();
    QString s1 = QString("Using GL version %1.%2").arg(context()->format().majorVersion()).arg(context()->format().minorVersion());
    INFO(s1);
    QString s2 = QString("Using GL profile %1").arg(prof==0 ? "None" : prof == 1 ? "Core" : prof == 2 ? "Compatibility" : "oops");
    INFO(s2);
    // report to console
    if( verbose ) {
        qDebug() << s1;
        qDebug() << s2;
    }
    
    QStringList imgFileExtensions;

    QList<QByteArray> a = QImageWriter::supportedImageFormats();
#ifdef USE_OPEN_EXR
    a.append ( "exr" );
#endif
    foreach ( QByteArray s, a ) {
        imgFileExtensions.append ( QString ( s ) );
    }
    INFO ( tr("Available output formats: ") + imgFileExtensions.join ( ", " ) );

}

void DisplayWidget::wheelEvent ( QWheelEvent* ev ) {
    if ( mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled ) return;

    cameraControl->wheelEvent ( ev );
    requireRedraw ( clearOnChange );
    ev->accept();
}

void DisplayWidget::mouseMoveEvent ( QMouseEvent *ev ) {
    if ( mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled ) return;

    mouseXY = ev->pos();
    
    if( buttonDown) {
        bool redraw = cameraControl->mouseEvent ( ev, width(), height() );
        if ( redraw ) {
            requireRedraw ( clearOnChange );
            ev->accept();
        }
    }
}

void DisplayWidget::mouseReleaseEvent ( QMouseEvent* ev )  {
    if ( mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled) return;

    buttonDown=false;

    // if the user just clicked and didn't drag update the statusbar
    if ( ev->pos() == mouseXY ) {
        QVector3D mXYZ = cameraControl->screenTo3D( mouseXY.x(), mouseXY.y(), ZAtMXY );
        // update statusbar
        mainWindow->statusBar()->showMessage ( QString ( "X:%1 Y:%2 Z:%3" ).arg ( mXYZ.x() ).arg ( mXYZ.y() ).arg ( mXYZ.z() ) );
        if(ev->button() == Qt::MiddleButton) {
          // SpotLightDir = polar coords vec2 DE-Raytracer.frag
          // LightPos = vec3 DE-Kn2.frag
          if(ev->modifiers() == Qt::ControlModifier) {
            // placement of light in DE-Kn2.frag
            mainWindow->setParameter( QString("LightPos = %1,%2,%3").arg(mXYZ.x()).arg(mXYZ.y()).arg(mXYZ.z()) );
          } else {
            // placement of target
            mainWindow->setParameter( QString("Target = %1,%2,%3").arg(mXYZ.x()).arg(mXYZ.y()).arg(mXYZ.z()) );
          }
            // do we have autofocus widget
            if(getFragmentSource()->autoFocus) {
                // test the widget state
                if( mainWindow->getParameter("AutoFocus") == "true") {
                    // only do this for 3D scenes
                    if(cameraID() == "3D" ) {
                        // get the eye pos
                        QStringList in = mainWindow->getParameter("Eye").split(",");
                        // convert parameter to 3d vector
                        QVector3D e = QVector3D(in.at(0).toDouble(),in.at(1).toDouble(),in.at(2).toDouble());
                        // calculate distance between camera and target                    
                        double d = mXYZ.distanceToPoint(e);
                        // set the focal plane to this distance
                        mainWindow->setParameter( "FocalPlane", d );
                        mainWindow->statusBar()->showMessage ( QString ( "X:%1 Y:%2 Z:%3 Dist:%4" ).arg ( mXYZ.x() ).arg ( mXYZ.y() ).arg ( mXYZ.z() ).arg(d) );
                    }
                }
            }
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
    if ( mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled ) return;
    if ( ev->button() == Qt::MouseButton::RightButton && ev->modifiers() == Qt::ShiftModifier) return;

    mouseXY=ev->pos();

    bool redraw = cameraControl->mouseEvent ( ev, width(), height() );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    }
    
//     mainWindow->statusBar()->showMessage ( QString ( "X:%1 Y:%2 Z:%3" ).arg ( mXYZ.x() ).arg ( mXYZ.y() ).arg ( mXYZ.z() ) );
}

void DisplayWidget::keyPressEvent ( QKeyEvent* ev ) {
    if ( mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled ) return;

    bool redraw = cameraControl->keyPressEvent ( ev );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    } else {
        QOpenGLWidget::keyPressEvent ( ev );
    }
}

void DisplayWidget::keyReleaseEvent ( QKeyEvent* ev ) {
    if ( mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled ) return;

    bool redraw = cameraControl->keyPressEvent ( ev );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    } else {
        QOpenGLWidget::keyReleaseEvent ( ev );
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
    double fov = cs.filter ( "FOV" ).at ( 0 ).split ( "=" ).at ( 1 ).toDouble();
    QStringList cv = cs.filter ( "Eye " ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    QVector3D eye = QVector3D ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
    cv = cs.filter ( "Target" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    QVector3D target = QVector3D ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
    cv = cs.filter ( "Up" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    QVector3D up = QVector3D ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
    
    double aspectRatio = double( ( double ) width() / ( double ) height() );
    double zNear = 0.00001;
    double zFar = 1000.0;
    double vertAngle = 180.0 * ( 2.0 * atan2 ( 1.0, ( 1.0/fov ) ) / M_PI );
    
    QMatrix4x4 matrix;
    matrix.setToIdentity();
    matrix.perspective ( vertAngle, aspectRatio, zNear, zFar );
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
    if(verbose) qDebug() << cset;
}

void DisplayWidget::drawSplines() {
    // this lets splines be visible when DEPTH_TO_ALPHA mode is active
    if(depthToAlpha )
        glDepthFunc ( GL_ALWAYS );
    else {
        glDepthFunc ( GL_LESS );    // closer to eye passes test
        glDepthMask ( GL_FALSE );   // no Writing to depth buffer for splines
    }
    
    // control point to highlight = currently selected preset keyframe
    int p = mainWindow->getCurrentCtrlPoint();

    targetSpline->drawSplinePoints();
    targetSpline->drawControlPoints ( p );

    eyeSpline->drawSplinePoints();
    eyeSpline->drawControlPoints ( p );
    
    // TODO add vectors to spline control points and enable editing of points with mouse
}

void DisplayWidget::createSplines(int numberOfControlPoints, int numberOfFrames ) {
    if( cameraID() == "3D" ) {
        QVector3D *eyeCp = ( QVector3D * ) eyeControlPoints.constData();
        QVector3D *tarCp = ( QVector3D * ) targetControlPoints.constData();
        QVector3D *upCp =  ( QVector3D * ) upControlPoints.constData();

        if(eyeCp != NULL && tarCp != NULL && upCp != NULL) {
            eyeSpline = new QtSpline(this,numberOfControlPoints,numberOfFrames, eyeCp);
            targetSpline = new QtSpline(this,numberOfControlPoints,numberOfFrames, tarCp);
            upSpline = new QtSpline(this,numberOfControlPoints,numberOfFrames, upCp);
            
            eyeSpline->setSplineColor( QColor("red") );
            eyeSpline->setControlColor( QColor("green"));
            
            targetSpline->setSplineColor( QColor("blue"));
            targetSpline->setControlColor( QColor("green"));
        }
    }
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


