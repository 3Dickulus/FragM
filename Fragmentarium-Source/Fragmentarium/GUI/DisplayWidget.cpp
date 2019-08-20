#include <cstdio>

#include <qopengl.h>
#include <qopenglext.h>

#include <QFileInfo>
#include <QMatrix4x4>
#include <QMenu>
#include <QStatusBar>
#include <QWheelEvent>


#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "DisplayWidget.h"

#include "../../ThirdPartyCode/hdrloader.h"
#include "MainWindow.h"
#include "VariableWidget.h"
#include "TextEdit.h"

#define DBOUT qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__

namespace Fragmentarium
{
namespace GUI
{

DisplayWidget::DisplayWidget ( MainWindow* mainWin, QWidget* parent )
    : QOpenGLWidget(parent), mainWindow(mainWin)
{

    QSurfaceFormat fmt;
    fmt.setSwapInterval(0);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setVersion(4,1);
#ifdef Q_OS_MAC
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
#endif
    fmt.setOption(QSurfaceFormat::DeprecatedFunctions,true);

    setFormat(fmt);

    verbose = false;
    clearOnChange = true;
    drawingState = Progressive;
    hiresBuffer = nullptr;
    iterationsBetweenRedraws = 0;
    previewBuffer = nullptr;
    doClearBackBuffer = true;
    backBuffer = nullptr;
    subframeCounter = 0;
    bufferShaderProgram = nullptr;
    shaderProgram = nullptr;
    bufferType = 0;
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
    contextMenu = nullptr;
    setFocusPolicy ( Qt::WheelFocus );
    timer = nullptr;
    maxSubFrames = 0;
    eyeSpline = targetSpline = upSpline = nullptr;
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

void DisplayWidget::initializeGL()
{

    initializeOpenGLFunctions();

    vendor = QString ( ( char * ) glGetString ( GL_VENDOR ) );
    renderer = QString ( ( char * ) glGetString ( GL_RENDERER ) );
    glvers = QString ( ( char * ) glGetString ( GL_VERSION ) );
    /// test for nVidia card and set the nV flag
    foundnV = vendor.contains ( "NVIDIA", Qt::CaseInsensitive );

}

void DisplayWidget::updateRefreshRate()
{
    QSettings settings;
    int i = settings.value ( "refreshRate", 40 ).toInt(); // 25 fps
    if (timer == nullptr) {
        timer = new QTimer();
        connect ( timer, SIGNAL ( timeout() ), this, SLOT ( timerSignal() ) );
    }
    if (i == 0) {
        i = 1;
    } // must be at least 1ms or GPU blocks GUI refresh.
    timer->start ( i );
    renderFPS = settings.value ( "fps", 25 ).toInt();
    INFO(tr("Setting display update timer to %1 ms (max %2 FPS).")
         .arg(i)
         .arg(1000.0 / i, 0, 'f', 2));
}

// let the system handle widget paint events,without shaders
void DisplayWidget::paintEvent(QPaintEvent *ev)
{

    if ( drawingState == Tiled ) {
        return;
    }
    if (bufferShaderProgram != nullptr) {
        bufferShaderProgram->release();
    }
    if (shaderProgram != nullptr) {
        shaderProgram->release();
    }
    QOpenGLWidget::paintEvent(ev);
}

DisplayWidget::~DisplayWidget() = default;

void DisplayWidget::contextMenuEvent(QContextMenuEvent *ev)
{
    if (ev->modifiers() == Qt::ShiftModifier) {
        if( mainWindow->isFullScreen()) {
            contextMenu->show();
            //            QPoint
            //            myPos((width()-contextMenu->width())/2,(height()-contextMenu->height())/2);
            contextMenu->move( mapToGlobal( ev->pos() ));
        }
    }
}

void DisplayWidget::setFragmentShader(FragmentSource fs)
{

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
        WARNING(tr("Unknown buffertype requested: ") + b + tr(".\n Type must be: NONE, RGBA8, RGBA16, RGBA16F, RGBA32F\nInitialized as GL_RGBA8"));
        bufferType = GL_RGBA8;
    }

    makeBuffers();
    INFO ( tr("Created front and back buffers as ") + b );
    int s;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s); // GL_MAX_3D_TEXTURE_SIZE GL_MAX_CUBE_MAP_TEXTURE_SIZE
    INFO ( tr( "Maximum texture size: %1x%1" ).arg ( s ) );

    requireRedraw ( true );

    initFragmentShader();

    if (shaderProgram == nullptr) { // something went wrong so do not try to setup textures or buffer shader
        return;
    }

    if (fs.textures.count() != 0) {
        initFragmentTextures();
    }

    initBufferShader();

    resetUniformProvenance();
}

void DisplayWidget::requireRedraw(bool clear, bool bufferShaderOnly)
{
    if (disableRedraw) {
        return;
    }

    if ( clear ) {
        clearBackBuffer();
        pendingRedraws = 1;
    } else if ( bufferShaderOnly ) {
        // nop
    } else {
        subframeCounter = 0;
    }
}

void DisplayWidget::uniformsHasChanged(Provenance provenance)
{
  if(fragmentSource.depthToAlpha) {
        BoolWidget *btest = dynamic_cast<BoolWidget *>(mainWindow->getVariableEditor()->getWidgetFromName("DepthToAlpha"));
        if (btest != nullptr) {
      // widget detected
      depthToAlpha = btest->isChecked();
    }
  }
  bufferUniformsHaveChanged |= !!(provenance & FromBufferShader);
  bool bufferShaderOnly = provenance == FromBufferShader;
  requireRedraw ( bufferShaderOnly ? false : clearOnChange, bufferShaderOnly );
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

void DisplayWidget::setGlTexParameter(QMap<QString, QString> map)
{

    /// as per https://www.opengl.org/wiki/Common_Mistakes#Automatic_mipmap_generation
    /// this has to get done before the rest of the GL texture parameters are set
    if ( map.values().contains ( "GL_LINEAR_MIPMAP_LINEAR" ) ||
            map.values().contains ( "GL_LINEAR_MIPMAP_NEAREST" ) ||
            map.values().contains ( "GL_NEAREST_MIPMAP_LINEAR" ) ||
            map.values().contains("GL_NEAREST_MIPMAP_NEAREST")) {
        if ( map.keys().contains ( "GL_TEXTURE_MAX_LEVEL" ) ) {
            bool ok;
            GLint levels;
            GLint wantedLevels = map["GL_TEXTURE_MAX_LEVEL"].toInt ( &ok,10 );
            if (!ok) {
                wantedLevels = 128;
            } // just an arbitrary small number, GL default = 1000
            glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, wantedLevels );

            if (format().majorVersion() > 2 || format().profile() == QSurfaceFormat::CompatibilityProfile) {
                glGenerateMipmap(GL_TEXTURE_2D); // Generate mipmaps here!!!
            }
            else {
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // Generate mipmaps here!!!
            }

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
        if (!ok) {
            fWanted = 1.0;
        }
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (fWanted > 1.0) ? (fWanted < fLargest) ? fWanted : fLargest : 1.0);
        glGetTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
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
QStringList DisplayWidget::shaderAsm(bool w)
{

    QStringList asmList;
    if (!foundnV) {
        if( format().majorVersion() < 4 ) {
                asmList = QStringList("nVidia GL > 4.0 required for this feature!");
        }
    }

    GLuint progId = w ? shaderProgram->programId() : bufferShaderProgram->programId();
    GLint formats = 0;
    glGetIntegerv ( GL_NUM_PROGRAM_BINARY_FORMATS, &formats );
    GLint binaryFormats[formats];
    glGetIntegerv ( GL_PROGRAM_BINARY_FORMATS, binaryFormats );

    GLint len=0;
    glGetProgramiv ( progId, GL_PROGRAM_BINARY_LENGTH, &len );
    uchar binary[ size_t(len+1) ];

    glGetProgramBinary(progId, len, nullptr, (GLenum *)binaryFormats, &binary[0]);

    QString asmTxt = "";

    // contains ALL uniforms in buffershader and shader program
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();
    bool foundFirstUniform=false;
    bool foundLastUniform=false;

    for ( int x=0; x<len-1; x++ ) {
        if (isprint(binary[x]) != 0) {
            asmTxt += binary[x];
        }

        if (!asmList.isEmpty() && asmList.last() == "END" && !asmTxt.startsWith("!")) {
            asmTxt="";
        }

        if ( !asmTxt.isEmpty() && binary[x] == 10) { // end of assembler statement
            if (foundFirstUniform) {
                asmList << asmTxt;
            }
            asmTxt="";
        }

        // this locates all of the uniform names not optimized out by the compiler
        if ( !asmTxt.isEmpty() && binary[x] == 0  ) { // end of string literal
            int uLoc = w ? shaderProgram->uniformLocation(asmTxt) : bufferShaderProgram->uniformLocation(asmTxt);
            if(uLoc != -1) {
                if (uLoc == 0) {
                    foundFirstUniform = true;
                }
                if (foundFirstUniform) {
                    asmList << asmTxt;
                }
            } else {
                if (foundFirstUniform) {
                    foundLastUniform = true;
                }
            }

            asmTxt="";
        }

        if (!asmList.isEmpty() && foundLastUniform && asmList.last().length() == 1) {
            asmList.removeLast();
        }
    }

    // find a value to go with the name, index in the program may not be the same as index in our list
    for( int n=0; n < vw.count(); n++) {
        asmTxt = vw[n]->getName();
        if( asmList.indexOf( asmTxt ) != -1 ){
            int uLoc = w ? shaderProgram->uniformLocation(asmTxt) : bufferShaderProgram->uniformLocation(asmTxt);
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

void DisplayWidget::createErrorLineLog ( QString message, QString log, bool infoOrWarn )
{
    QStringList logList = log.split ( "\n" );
    QRegExp num ( "([0-9]+)" );
    int errorCount=0;
    int errorLine=0;
    int fileIndex=0;

    infoOrWarn ? WARNING ( message ) : INFO ( message );

    foreach ( const QString &str, logList ) {
        QString newStr=str;

        if(foundnV) {
            // test nVidia log
            QRegExp testnvidia ( "^([0-9]+)[(]([0-9]+)[)]" );
            if ( testnvidia.indexIn ( str ) != -1 ) {
                if ( num.indexIn ( testnvidia.cap ( 1 ) ) != -1 ) {
                    fileIndex=num.cap ( 1 ).toInt();
                    newStr.replace ( 0,num.cap ( 1 ).length(), fragmentSource.sourceFileNames[fileIndex] + " " );
                }
                if ( num.indexIn ( testnvidia.cap ( 2 ) ) != -1 )
                    errorLine=num.cap ( 1 ).toInt()-fileIndex;
                errorCount++;
            }
        }
        else {
            // test AMD RX 580 ? Mesa ?
            QRegExp testamd ( "^([0-9]+)[:]([0-9]+)[(]" );
            if ( testamd.indexIn ( str ) != -1 ) {
                if ( num.indexIn ( testamd.cap ( 1 ) ) != -1 ) {
                    fileIndex=num.cap ( 1 ).toInt();
                    newStr.replace ( 0,num.cap ( 1 ).length(), fragmentSource.sourceFileNames[fileIndex] + " " );
                }
                if ( num.indexIn ( testamd.cap ( 2 ) ) != -1 ) {
                    errorLine=num.cap ( 1 ).toInt()-fileIndex;
                }
                errorCount++;
            }
        }
        // emit a single log widget line for each line in the log
        if ( infoOrWarn ) {
            WARNING ( newStr );
        } else {
            INFO ( newStr );
        }

        QSettings settings; // only jump to first error as later errors may be caused by this one so fix it first
        if ( (settings.value ( "jumpToLineOnWarn", true ).toBool() || settings.value ( "jumpToLineOnError", true ).toBool())) {
            if(errorCount == 1 && fileIndex == 0) {
                // jump to error line in text editor
                TextEdit *te = mainWindow->getTextEdit();
                QTextCursor cursor(te->textCursor());
                cursor.setPosition(0);
                cursor.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,errorLine+10);
                te->setTextCursor( cursor );
                cursor.movePosition(QTextCursor::Up,QTextCursor::MoveAnchor,11);
                te->setTextCursor( cursor );
            }
        }
    }
}

void DisplayWidget::initFragmentShader()
{
    if (shaderProgram != nullptr) {
        shaderProgram->release();
        shaderProgram->removeAllShaders();
        delete ( shaderProgram );
        shaderProgram = nullptr;
    }

    QSettings settings;

    shaderProgram = new QOpenGLShaderProgram ( context() );

    // Vertex shader
    bool s = shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, fragmentSource.vertexSource.join("\n"));
    if ( fragmentSource.vertexSource.count() == 0 ) {
        WARNING ( tr("No vertex shader found!") );
        s = false;
    }

    if ( !s ) {
        createErrorLineLog( tr("Could not create vertex shader: "), shaderProgram->log(), true /*Warn*/ );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }
    if (!shaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Vertex shader compiled with warnings: "), shaderProgram->log(), false /*Info*/ );
    }

    // Fragment shader
    s = shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource.getText());

    if (s) { // Requests the shader program's id to be created immediately.
        s = shaderProgram->create();
    }

    if ( !s ) {
        createErrorLineLog( tr("Could not create fragment shader: "), shaderProgram->log(), true /*Warn*/ );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }

    if (!shaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Fragment shader compiled with warnings: "), shaderProgram->log(), false /*Info*/ );
    }

    s = shaderProgram->link();

    if ( !s ) {
        createErrorLineLog( tr("Could not link vertex + fragment shader: "), shaderProgram->log(), true /*Warn*/ );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }

    if (!shaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Fragment shader compiled with warnings: "), shaderProgram->log(), false /*Info*/ );
    }

    s = shaderProgram->bind();
    if ( !s ) {
        createErrorLineLog( tr("Could not bind shaders: "), shaderProgram->log(), true /*Warn*/ );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }

    // Setup backbuffer texture for this shader
    if ( bufferType != 0 ) {
        makeCurrent();
        // Bind first texture to backbuffer
        glActiveTexture ( GL_TEXTURE0 ); // non-standard (>OpenGL 1.3) gl extension

        int l = shaderProgram->uniformLocation ( "backbuffer" );
        if ( l != -1 ) {
              GLuint i = backBuffer->texture();
              glBindTexture ( GL_TEXTURE_2D,i );
              if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                  setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
              }
              shaderProgram->setUniformValue ( l, ( GLuint ) 0 );
        }
    } else {
        WARNING ( tr("Trying to use a backbuffer, but no bufferType set.") );
        WARNING ( tr("Use the buffer define, e.g.: '#buffer RGBA8' ") );
    }

}

/*
#define providesBackground
#group Skybox
uniform samplerCube skybox; file[cubemap.png]
vec3  backgroundColor(vec3 dir) {
    return textureCube(skybox, dir).rgb;
}
*/

bool DisplayWidget::loadHDRTexture ( QString texturePath, GLenum type, GLuint textureID )
{
    HDRLoaderResult result;
    bool loaded = HDRLoader::load ( texturePath.toStdString().c_str(), result );

    if ( loaded ) {
        glBindTexture ( ( type == GL_SAMPLER_CUBE ) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID );

        if ( type == GL_SAMPLER_CUBE ) {
            glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width * 0 * 3] );
            glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width * 1 * 3] );
            glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width * 2 * 3] );
            glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width * 3 * 3] );
            glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width * 4 * 3] );
            glTexImage2D ( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, result.width, result.width, 0, GL_RGB, GL_FLOAT, &result.cols[result.width * 5 * 3] );
        } else {
            glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, result.width, result.height, 0, GL_RGB, GL_FLOAT, &result.cols[0] );
        }

    } else {
        WARNING ( "HDR texture failed to load!" );
    }

    return loaded;
}

#ifdef USE_OPEN_EXR
//
// Read an RGBA image using class RgbaInputFile:
//
//    - open the file
//    - allocate memory for the pixels
//    - describe the memory layout of the pixels
//    - read the pixels from the file
//
bool DisplayWidget::loadEXRTexture(QString texturePath, GLenum type, GLuint textureID)
{
    RgbaInputFile file ( texturePath.toLatin1().data() );
    Box2i dw = file.dataWindow();

    if ( file.isComplete() ) {

        int w  = dw.max.x - dw.min.x + 1;
        int h = dw.max.y - dw.min.y + 1;
        int s;
        glGetIntegerv ( GL_MAX_TEXTURE_SIZE, &s );
        s /= 4;
        if ( w>s || h>(s*6) ) {
            WARNING(tr("Exrloader found EXR image: %1 x %2 is too large! max %3x%3").arg(w).arg(h).arg(s));
            return false;
        }

        glBindTexture((type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID);

        Array2D<Rgba>pixels ( w, 1 );

        auto *cols = new float[w * h * 4];
        while ( dw.min.y <= dw.max.y ) {
            file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * w, 1, w);
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
        if(type == GL_SAMPLER_CUBE) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, w, w, 0, GL_RGBA, GL_FLOAT, cols + ((w * w * 4) * 0));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, w, w, 0, GL_RGBA, GL_FLOAT, cols + ((w * w * 4) * 1));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, w, w, 0, GL_RGBA, GL_FLOAT, cols + ((w * w * 4) * 2));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, w, w, 0, GL_RGBA, GL_FLOAT, cols + ((w * w * 4) * 3));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, w, w, 0, GL_RGBA, GL_FLOAT, cols + ((w * w * 4) * 4));
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, w, w, 0, GL_RGBA, GL_FLOAT, cols + ((w * w * 4) * 5));
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, cols);
        }
        delete [] cols;
        cols = nullptr;
    } else {
        WARNING(tr("Exrloader found EXR image: %1 is not complete").arg(texturePath));
        return false;
    }
    return true;
}
#endif

// Qt format image, Qt 5+ loads EXR format on linux
bool DisplayWidget::loadQtTexture(QString texturePath, GLenum type, GLuint textureID)
{
    QImage im;
    bool loaded = im.load(texturePath);

    if(loaded) {
        glBindTexture((type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID);
    }

    if(type == GL_SAMPLER_CUBE && loaded) {
        QImage t = im.convertToFormat(QImage::Format_RGBA8888);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width() * 0));
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width() * 1));
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width() * 2));
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width() * 3));
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width() * 4));
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, t.width(), t.width(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.scanLine(t.width() * 5));
    } else if(loaded) {
        QImage t = im.mirrored().convertToFormat(QImage::Format_RGBA8888);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits());
    } else {
        WARNING("Texture failed to load!");
        return false;
    }
    return loaded;
}

bool DisplayWidget::setTextureParms(QString textureUniformName, GLenum type)
{
    if(type == GL_SAMPLER_CUBE) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return true;

    } else if (fragmentSource.textureParams.contains(textureUniformName)) { // set texparms
        setGlTexParameter ( fragmentSource.textureParams[textureUniformName] );
        return true;
    } else { // User did not set texparms Disable mip-mapping per default.
        QMap<QString, QString> map;
        map.insert ( "GL_TEXTURE_MAG_FILTER","GL_LINEAR" );
        map.insert ( "GL_TEXTURE_MIN_FILTER","GL_LINEAR" );
        setGlTexParameter ( map );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        return true;
    }
    // if we get here something is wrong
    return false;
}

void DisplayWidget::initFragmentTextures()
{
    if (shaderProgram == nullptr || fragmentSource.textures.count() < 1) {
        return; // something went wrong so do not try to setup textures
    }

    int u = 1; // the backbuffer is always 0 while textures from uniforms start at 1
    QMap<QString, bool> textureCacheUsed;

    QMapIterator<QString, QString> it( fragmentSource.textures );
    while( it.hasNext() ) {
        it.next();

        GLuint textureID;

        QString textureUniformName = it.key();
        QString texturePath = it.value();
        bool loaded = false;
        if(!texturePath.isEmpty()) {

            int l = shaderProgram->uniformLocation ( textureUniformName );

            if ( l != -1 ) { // found named texture in shader program

                // 2D or Cube ?
                GLsizei bufSize = 256;
                GLsizei length;
                GLint size;
                GLenum type;
                GLchar name[bufSize];

                glGetActiveUniform(shaderProgram->programId(), l, bufSize, &length, &size, &type, name);

                // set current texture
                glActiveTexture(GL_TEXTURE0 + u); // non-standard (>OpenGL 1.3) gl extension

                // check cache first
                if ( !TextureCache.contains ( texturePath ) ) {
                    // if not in cache then create one and try to load and add to cache
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // byte alignment 4 bytes = 32 bits
                    // allocate a texture id
                    glGenTextures ( 1, &textureID );

                    if (verbose) {
                        qDebug() << QString("Allocating texture ID: %1 %2").arg(textureID).arg(texturePath);
                    }

                    if (texturePath.endsWith(".hdr", Qt::CaseInsensitive)) { // is HDR format image ?
                        loaded = loadHDRTexture(texturePath, type, textureID);
                    }
#ifdef USE_OPEN_EXR
                    else if (texturePath.endsWith(".exr", Qt::CaseInsensitive)) { // is EXR format image ?
                        loaded = loadEXRTexture(texturePath, type, textureID);
                    }
#endif
                    else {
                        loaded = loadQtTexture(texturePath, type, textureID);
                    }
                    if ( loaded ) {
                        // add to cache
                        TextureCache[texturePath] = textureID;
                        textureCacheUsed[texturePath] = true;
                    }
                } else { // use cache
                    textureID = TextureCache[texturePath];
                    textureCacheUsed[texturePath] = true;
                    loaded = true;
                }


                if ( loaded ) {
                    glBindTexture((type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID);
                    if( setTextureParms(textureUniformName, type) ) {
                        shaderProgram->setUniformValue ( l, ( GLuint ) u );
                    }
                    u++;
                } else {
                    WARNING(tr("Not a valid texture: ") + QFileInfo(texturePath).absoluteFilePath());
                }
            } else {
                WARNING(tr("Unused sampler uniform: ") + textureUniformName);
            }
        }
    }

    // Check for unused textures
    clearTextureCache(&textureCacheUsed);

}

void DisplayWidget::clearTextureCache(QMap<QString, bool> *textureCacheUsed)
{

    // Check for unused textures
    if (textureCacheUsed != nullptr) {
        QMutableMapIterator<QString, int> i(TextureCache);
        while ( i.hasNext() ) {
            i.next();
            if (!textureCacheUsed->contains(i.key())) {
                INFO("Removing texture from cache: " +i.key());
                GLuint id = i.value();
                glDeleteTextures(1, &id);
                i.remove();
            }

        }
    } else { // clear cache and textures
        QMutableMapIterator<QString, int> i(TextureCache);
        while (i.hasNext()) {
            i.next();
            INFO("Removing texture from cache: " +i.key());
            GLuint id = i.value();
            glDeleteTextures(1, &id);
        }
        TextureCache.clear();
    }
}

void DisplayWidget::initBufferShader()
{

    if (bufferShaderProgram != nullptr) {
        bufferShaderProgram->release();
        delete ( bufferShaderProgram );
        bufferShaderProgram = nullptr;
    }

    if (fragmentSource.bufferShaderSource == nullptr) {
        return;
    }

    bufferShaderProgram = new QOpenGLShaderProgram ( this );

    // Vertex shader
    bool s = bufferShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, fragmentSource.bufferShaderSource->vertexSource.join("\n"));
    if ( fragmentSource.bufferShaderSource->vertexSource.count() == 0 ) {
        WARNING ( tr("No buffershader vertex shader found!") );
        s = false;
    }

    if ( !s ) {
        WARNING(tr("Could not create buffer vertex shader: ") + bufferShaderProgram->log());
        delete ( bufferShaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
    if (!bufferShaderProgram->log().isEmpty()) {
        INFO(tr("Buffer vertex shader compiled with warnings: ") + bufferShaderProgram->log());
    }

    // Fragment shader
    s = bufferShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource.bufferShaderSource->getText());

    if (s) {
        s = bufferShaderProgram->create();
    }

    if ( !s ) {
        WARNING(tr("Could not create buffer fragment shader: ") + bufferShaderProgram->log());
        delete ( bufferShaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
    if (!bufferShaderProgram->log().isEmpty()) {
        INFO(tr("Buffer fragment shader compiled with warnings: ") + bufferShaderProgram->log());
    }

    s = bufferShaderProgram->link();

    if ( !s ) {
        QStringList tmp = bufferShaderProgram->log().split ( '\n' );
        QStringList logStrings; // the first 5 lines
        for (int i = 0; i < 5; i++) {
            logStrings << tmp.at(i);
        }
        logStrings << "";
        WARNING ( tr("Could not link buffershader: ") );
        CRITICAL ( logStrings.join ( '\n' ) );
        delete ( bufferShaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
    if (!bufferShaderProgram->log().isEmpty()) {
        INFO(tr("Fragment shader compiled with warnings: ") + bufferShaderProgram->log());
    }

    s = bufferShaderProgram->bind();
    if ( !s ) {
        WARNING ( tr("Could not bind shaders: ") + bufferShaderProgram->log() );
        delete ( shaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
}

void DisplayWidget::resetCamera(bool fullReset)
{
    if (cameraControl == nullptr) {
        return;
    }
    cameraControl->reset ( fullReset );
}

void DisplayWidget::makeBuffers()
{
    int w = pixelWidth();
    int h = pixelHeight();

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
    if (oldBufferString == bufferString) {
        return;
    }
    oldBufferString = bufferString;

    if (previewBuffer != nullptr) {
        delete ( previewBuffer );
        previewBuffer = nullptr;
    }
    if (backBuffer != nullptr) {
        delete ( backBuffer );
        backBuffer = nullptr;
    }

    makeCurrent();

    QOpenGLFramebufferObjectFormat fbof;
    fbof.setAttachment ( QOpenGLFramebufferObject::Depth );
    fbof.setInternalTextureFormat ( bufferType );
    fbof.setTextureTarget ( GL_TEXTURE_2D );

    // we must create both the backbuffer and previewBuffer
    previewBuffer = new QOpenGLFramebufferObject ( w, h, fbof );
    backBuffer = new QOpenGLFramebufferObject ( w, h, fbof );
    clearBackBuffer();

    if (format().majorVersion() > 2 || format().profile() == QSurfaceFormat::CompatibilityProfile) {
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
            qDebug( ) << tr("FBO Incomplete Error!");
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER_EXT, defaultFramebufferObject());
        }
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
    }
}

void DisplayWidget::clearBackBuffer()
{
    doClearBackBuffer = true;
}

void DisplayWidget::setViewPort(int w, int h)
{
    if ( drawingState == Tiled ) {
        glViewport ( 0, 0,bufferSizeX, bufferSizeY );
    } else if ( fitWindow ) {
        glViewport ( 0, 0, w, h );
    } else {
        glViewport(0, 0, bufferSizeX < w ? bufferSizeX : w, bufferSizeY < h ? bufferSizeY : h);
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
//   glm::dvec3 eye = glm::dvec3 ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
//   cv = cs.filter ( "Target" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   glm::dvec3 target = glm::dvec3 ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
//   cv = cs.filter ( "Up" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
//   glm::dvec3 up = glm::dvec3 ( cv.at ( 0 ).toDouble(),cv.at ( 1 ).toDouble(),cv.at ( 2 ).toDouble() );
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

bool DisplayWidget::checkShaderProg(GLuint programID)
{
    if(programID == shaderProgram->programId()) {
        if(verbose) {
            qDebug() << "\nshaderProgram";
        }
        return true;
    } else if (hasBufferShader() && programID == bufferShaderProgram->programId()) {
        if(verbose) {
            qDebug() << "\nbufferShaderProgram";
        }
        return true;
    } else {
        if(verbose) {
            qDebug() << "\nNO ShaderProgram!!!";
        }
    }
    return false;
}

void DisplayWidget::checkForSpecialCase(QString uniformName, QString &uniformValue)
{
        if (uniformName.startsWith("gl_")) { uniformValue = "OpenGL variable"; }
        if (uniformName == "pixelSize") { uniformValue = "Special variable"; }
        if (uniformName == "globalPixelSize") { uniformValue = "Special variable"; }
        if (uniformName == "subframe") { uniformValue = "Special variable"; }
        if (uniformName == "frontbuffer") { uniformValue = "Special variable"; }
        if (uniformName == "backbuffer") { uniformValue = "Special variable"; }
        if (uniformName == "time") { uniformValue = "Special variable"; }
}

void DisplayWidget::setFloatType(GLenum type, QString &tp)
{
        switch(type) {
            case GL_BYTE:           tp = "BYTE  "; break;
            case GL_UNSIGNED_BYTE:  tp = "UNSIGNED_BYTE"; break;
            case GL_SHORT:          tp = "SHORT "; break;
            case GL_UNSIGNED_SHORT: tp = "UNSIGNED_SHORT"; break;
            case GL_INT:            tp = "INT   "; break;
            case GL_UNSIGNED_INT:   tp = "UNSIGNED_INT"; break;
            case GL_FLOAT:          tp = "FLOAT "; break;
            case GL_FIXED:          tp = "FIXED "; break;
            case GL_FLOAT_VEC2:     tp = "FLOAT_VEC2"; break;
            case GL_FLOAT_VEC3:     tp = "FLOAT_VEC3"; break;
            case GL_FLOAT_VEC4:     tp = "FLOAT_VEC4"; break;
            case GL_INT_VEC2:       tp = "INT_VEC2"; break;
            case GL_INT_VEC3:       tp = "INT_VEC3"; break;
            case GL_INT_VEC4:       tp = "INT_VEC4"; break;
            case GL_BOOL:           tp = "BOOL  "; break;
            case GL_BOOL_VEC2:      tp = "BOOL_VEC2"; break;
            case GL_BOOL_VEC3:      tp = "BOOL_VEC3"; break;
            case GL_BOOL_VEC4:      tp = "BOOL_VEC4"; break;
            case GL_FLOAT_MAT2:     tp = "FLOAT_MAT2"; break;
            case GL_FLOAT_MAT3:     tp = "FLOAT_MAT3"; break;
            case GL_FLOAT_MAT4:     tp = "FLOAT_MAT4"; break;
            case GL_SAMPLER_2D:     tp = "SAMPLER_2D"; break;
            case GL_SAMPLER_CUBE:   tp = "SAMPLER_CUBE"; break;
            default: tp = "Not found!"; break;
        }
}

void DisplayWidget::setDoubleType(GLuint programID, GLenum type, QString uniformName, QString uniformValue, bool &foundDouble, QString &tp)
{
    double x,y,z,w;

    GLint index = glGetUniformLocation(programID, uniformName.toStdString().c_str());

    bool found = false;
    if(index != -1) {
        switch(type) {
            case GL_DOUBLE:
                x = uniformValue.toDouble(&found);
                foundDouble |= found;
                if( foundDouble ) {
                    glUniform1d(index, x);
                    tp = "DOUBLE";
                }
            break;
            case GL_DOUBLE_VEC2:
                x = uniformValue.split(",").at(0).toDouble(&found);
                foundDouble |= found;
                y = uniformValue.split(",").at(1).toDouble(&found);
                foundDouble |= found;
                if(foundDouble) {
                    glUniform2d(index, x, y);
                    tp = "DOUBLE_VEC2";
                }
            break;
            case GL_DOUBLE_VEC3:
                x = uniformValue.split(",").at(0).toDouble(&found);
                foundDouble |= found;
                y = uniformValue.split(",").at(1).toDouble(&found);
                foundDouble |= found;
                z = uniformValue.split(",").at(2).toDouble(&found);
                foundDouble |= found;
                if(foundDouble) {
                    glUniform3d(index, x, y, z);
                    tp = "DOUBLE_VEC3";
                }
            break;
            case GL_DOUBLE_VEC4:
                x = uniformValue.split(",").at(0).toDouble(&found);
                foundDouble |= found;
                y = uniformValue.split(",").at(1).toDouble(&found);
                foundDouble |= found;
                z = uniformValue.split(",").at(2).toDouble(&found);
                foundDouble |= found;
                w = uniformValue.split(",").at(3).toDouble(&found);
                foundDouble |= found;
                if(foundDouble) {
                    glUniform4d(index, x, y, z, w);
                    tp = "DOUBLE_VEC4";
                }
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
}

void DisplayWidget::resetUniformProvenance()
{
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();
    foreach (VariableWidget *w, vw) {
        w->setProvenance(FromUnknown);
    }
    QOpenGLShaderProgram *programs[2] = { shaderProgram, bufferShaderProgram };
    Provenance provenances[2] = { FromMainShader, FromBufferShader };
    for (int k = 0; k < 2; ++k) {
        QOpenGLShaderProgram *shaderProg = programs[k];
        Provenance provenance = provenances[k];
        if (shaderProg == nullptr) continue;
        GLuint programID = shaderProg->programId();
        if (programID == 0) continue;
        GLint count = 0;
        // this only returns uniforms that have not been optimized out
        glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);
        for (int i = 0; i < count; i++) {
            GLsizei bufSize = 256;
            GLsizei length;
            GLint size;
            GLenum type;
            GLchar name[bufSize];
            glGetActiveUniform(programID, i, bufSize, &length, &size, &type, name);
            QString uniformName = (char *)name;
            // FIXME this is quadratic: O(number of active uniforms * number of widgets declared)
            // FIXME could go to n log n by sorting each by name and zip ascending?
            foreach (VariableWidget *w, vw) {
                if (uniformName == w->getName()) {
                    w->addProvenance(provenance);
                    break; // can't have more than one uniform with the same name
                }
            }
        }
    }
}

void DisplayWidget::setShaderUniforms(QOpenGLShaderProgram *shaderProg)
{

    GLuint programID = shaderProg->programId();

    if(!checkShaderProg(programID)) return;

    // contains ALL uniforms in buffershader and shader program
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();

    int count;
    // this only returns uniforms that have not been optimized out
    glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);

    for (int i = 0; i < count; i++) {

        GLsizei bufSize = 256;
        GLsizei length;
        GLint size;
        GLenum type;
        GLchar name[bufSize];

        glGetActiveUniform(programID, i, bufSize, &length, &size, &type, name);
        QString uniformName = (char *)name;
        QString uniformValue;
        checkForSpecialCase(uniformName, uniformValue);

        // find a value to go with the name index in the program, may not be the same as index in our list
        for( int n=0; n < vw.count(); n++) {
            if(uniformName == vw[n]->getName()) {
                // get slider values
                uniformValue = vw[n]->getValueAsText();
                // test and get filename
                if (uniformValue.isEmpty()) {
                    uniformValue = vw[n]->toString();
                }
                break;
            }
        }

        if (uniformValue.isEmpty()) {
            uniformValue = "Unused variable widget";
        }

        QString tp = "";
        setFloatType(type, tp);
        bool foundDouble = false;

        if (format().majorVersion() > 3 && format().minorVersion() >= 0) {
            // do not try to set special, gl_ or unused uniform even if it is double type
            if (!uniformValue.contains("variable")) {
                setDoubleType(programID, type, uniformName, uniformValue, foundDouble, tp);
            }
        }

        // this sets User (32 bit) uniforms not handled above
        if(!foundDouble) {
            for( int n=0; n < vw.count(); n++) {
                if(uniformName == vw[n]->getName()) {
                    auto *sw = dynamic_cast<SamplerWidget *>(vw[n]);
                    if (sw != nullptr) {
                        QMapIterator<QString, QString> it( fragmentSource.textures );
                        while( it.hasNext() ) {
                            it.next();
                            if(it.value().contains(sw->getValue())) {
                                sw->texID = TextureCache[it.value()]+GL_TEXTURE0;
//                                 DBOUT << "Sampler real texID " << TextureCache[it.value()]+GL_TEXTURE0;
                            }
                        }
                    }
                    vw[n]->setIsDouble(foundDouble); // ensure float sliders set to float decimals
                    vw[n]->setUserUniform(shaderProg);
                    break;
                }
            }
        } else {
            vw[i]->setIsDouble(foundDouble);
        } // this takes care of buffershader (Post) sliders :D

        // type name and value to console
        if (subframeCounter == 1 && verbose) {
            qDebug() << tp << "\t" << uniformName << uniformValue;
        }
    }
    if (subframeCounter == 1 && verbose) {
        qDebug() << count << " active uniforms initialized\n";
    }
}

void DisplayWidget::setupShaderVars(int w, int h) {

    cameraControl->transform(pixelWidth(), pixelHeight()); // -- Modelview + loadIdentity
    int l = shaderProgram->uniformLocation ( "pixelSize" );

    if ( l != -1 ) {
        shaderProgram->setUniformValue ( l, ( float ) ( 1.0/w ), ( float ) ( 1.0/h ) );
    }
    // Only in DepthBufferShader.frag & NODE-Raytracer.frag
    l = shaderProgram->uniformLocation ( "globalPixelSize" );

    if ( l != -1 ) {
        shaderProgram->setUniformValue ( l, ( ( float ) 1.0/w ), ( ( float ) 1.0/h ) );
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
            if (verbose && subframeCounter == 1) {
                qDebug() << QString("Binding backbuffer (ID: %1) to active texture %2").arg(i).arg(0);
            }
            if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
            }
            shaderProgram->setUniformValue ( l, 0 );
        }

        l = shaderProgram->uniformLocation ( "subframe" );

        if ( l != -1 ) {
            shaderProgram->setUniformValue ( l, subframeCounter );
            // if(verbose) qDebug() << QString("Setting subframe: %1").arg(subframeCounter);
        }
    }

}

void DisplayWidget::draw3DHints()
{
    if ( mainWindow->wantPaths()  && !isContinuous()) {
        if (eyeSpline != nullptr && drawingState != Animation && drawingState != Tiled) {
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
}

void DisplayWidget::drawFragmentProgram(int w, int h, bool toBuffer)
{

    if (!this->isValid()) {
        return;
    }

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
    //     if( m3DTexId!=0 ) {
    //     draw3DTexture();
    //     return; }
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

        // only available in GL > 2.0 < 3.2 and compatibility profile
        glTranslated ( x * ( 2.0/tiles ) , y * ( 2.0/tiles ), 1.0 );
        glScaled ( ( 1.0+padding ) /tiles, ( 1.0+padding ) /tiles,1.0 );
    }

    setupShaderVars(w, h);

    // Setup User Uniforms

    // this should speed things up a little because we are only setting uniforms on
    // the first subframe of the first tile
    if (subframeCounter <= 1 && tilesCount <= 1) {
        setShaderUniforms(shaderProgram);
    }

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

    glFinish(); // wait for GPU to return control

    // finished with the shader
    shaderProgram->release();

    if (cameraControl->getID() == "3D") {
        // draw splines using depth buffer for occlusion... or not
            draw3DHints();
        /// copy the depth value @ mouse XY
        if(subframeCounter == 1) {
            float zatmxy;
            glReadPixels(mouseXY.x(), height() - mouseXY.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &zatmxy);
            ZAtMXY = zatmxy;
        }
    }
    // restore state
    glPopAttrib();
}

bool DisplayWidget::FBOcheck()
{

    if (previewBuffer == nullptr || !previewBuffer->isValid()) {
      WARNING ( tr("Non valid FBO - previewBuffer") );
        return false;
    }

    if (backBuffer == nullptr || !backBuffer->isValid()) {
      WARNING ( tr("Non valid FBO - backBuffer") );
        return false;
    }

    QSize s = backBuffer->size();

    if ( s.height() ==0 || s.width() ==0 ) {
      WARNING ( tr("BACK BUFFER FAILED!") );
        return false;
    }

    s = previewBuffer->size();

    if ( s.height() ==0 || s.width() ==0 ) {
      WARNING ( tr("PREVIEW BUFFER FAILED!") );
        return false;
    }

    if(verbose && subframeCounter == 1) {
        static int c = 0;
        qDebug() << QString("drawToFrameBufferObject: %1").arg(c++);
    }

    return true;
}

void DisplayWidget::setupBufferShaderVars(int w, int h)
{
        int l = bufferShaderProgram->uniformLocation ( "pixelSize" );

        if ( l != -1 ) {
            shaderProgram->setUniformValue(l, (float)(1.0 / w), (float)(1.0 / h));
        }
        // for DepthBufferShader.frag & NODE-Raytracer.frag
        l = bufferShaderProgram->uniformLocation ( "globalPixelSize" );

        if ( l != -1 ) {
            shaderProgram->setUniformValue(l, (1.0 / w), (1.0 / h));
        }

        l = bufferShaderProgram->uniformLocation ( "frontbuffer" );

        if ( l != -1 ) {
            bufferShaderProgram->setUniformValue ( l, 0 );
        } else {
            WARNING(tr("No front buffer sampler found in buffer shader. This doesn't make sense."));
        }
        // Setup User Uniforms
        if (bufferUniformsHaveChanged) {
            setShaderUniforms ( bufferShaderProgram );
            bufferUniformsHaveChanged = false;
        }
}

void DisplayWidget::drawToFrameBufferObject(QOpenGLFramebufferObject *buffer, bool drawLast, bool doMain)
{

    if (!this->isValid() || !FBOcheck()) {
        return;
    }

    QSize s = backBuffer->size();

    if ( !drawLast && doMain ) {
        for ( int i = 0; i <= iterationsBetweenRedraws; i++ ) {
            if (backBuffer != nullptr) {
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

    if (drawingState != Tiled) {
        mainWindow->setSubFrameDisplay ( subframeCounter );
    }

    if (buffer != nullptr && !buffer->bind()) {
        WARNING ( tr("Failed to bind target buffer") );
        return;
    }

    // Draw a textured quad using the preview texture.
    if (bufferShaderProgram != nullptr) {
        bufferShaderProgram->bind();
        setupBufferShaderVars( s.width(),s.height());
    }

    glPushAttrib ( GL_ALL_ATTRIB_BITS );
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity();
    glMatrixMode ( GL_MODELVIEW );
    glLoadIdentity();

    setViewPort ( pixelWidth(),pixelHeight() );

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

    if (bufferShaderProgram != nullptr) {
        bufferShaderProgram->release();
    }

    if (buffer != nullptr && !buffer->release()) {
        WARNING ( tr("Failed to release target buffer") );
    }

}

/**
 * Call this before and after performing a tile render.
 * @brief DisplayWidget::clearTileBuffer
 */
void DisplayWidget::clearTileBuffer()
{
    if (hiresBuffer != nullptr) {
        bool relcheck = hiresBuffer->release();
        if(relcheck) {
          delete ( hiresBuffer );
            hiresBuffer = nullptr;
        } else {
            WARNING(tr("Failed to release hiresBuffer FBO"));
    }
    }
    mainWindow->getBufferSize(pixelWidth(), pixelHeight(), bufferSizeX, bufferSizeY, fitWindow);
    makeBuffers();
}

void DisplayWidget::clearGL()
{
    /// proper clear on tile based GPU
    /// http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-TileBasedArchitectures.pdf
    glDisable ( GL_SCISSOR_TEST );
    glColorMask ( GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE );
    glDepthMask ( GL_TRUE );
    glStencilMask ( 0xFFFFFFFF );
    glClearDepth(1.0f);
    glClear ( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
}

#ifdef USE_OPEN_EXR
bool DisplayWidget::getRGBAFtile(Array2D<Rgba> &array, int w, int h)
{

    auto *myImgdata = (GLfloat *)malloc((h * w * 4 * sizeof(GLfloat)));
    auto *myDepths = (GLfloat *)malloc((h * w * sizeof(GLfloat)));

    bool retOK = true;

    if ( !hiresBuffer->bind() ) {
      WARNING ( tr("Failed to bind hiresBuffer FBO") );
      retOK = false;
    }

    // read colour values from hiresBuffer
    if (retOK) {
        glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, myImgdata);
    }

    if ( !hiresBuffer->release() ) {
      WARNING ( tr("Failed to release hiresBuffer FBO") );
        retOK = false;
    }

    if ( depthToAlpha && retOK ) {
      if ( !previewBuffer->bind() ) {
        WARNING ( tr("Failed to bind previewBuffer FBO") );
        retOK = false;
      }
        // read depth values from previewBuffer
        if (retOK) {
            glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, myDepths);
        }

        if ( !previewBuffer->release() ) {
          WARNING ( tr("Failed to release previewBuffer FBO") );
          retOK = false;
        }
    }

    // put them together as RGBZ or RGBA
    if(retOK) {
        for ( int i = 0; i < h; i++ ) {
            for ( int j = 0; j < w; j++ ) {
                array[(h - 1) - i][j] = Rgba(
                                            myImgdata[((i * w) + j) * 4 + 0], myImgdata[((i * w) + j) * 4 + 1],
                                            myImgdata[((i * w) + j) * 4 + 2],
                                            depthToAlpha ? myDepths[((i * w) + j) * 1 + 0] : myImgdata[((i * w) + j) * 4 + 3]);
            }
        }
    }

    free(myImgdata);
    myImgdata = nullptr;
    free(myDepths);
    myDepths = nullptr;

    return retOK;

}
#endif

bool DisplayWidget::initPreviewBuffer() {
    bool ret = true;
    if ( !previewBuffer->bind() ) {
      WARNING ( tr("Failed to bind previewBuffer FBO") );
      ret = false;
    }

    glClearColor ( 0.0f,0.0f,0.0f,0.0f );
    clearGL();

    if ( !previewBuffer->release() ) {
      WARNING ( tr("Failed to release previewBuffer FBO") );
      ret = false;
    }

    return ret;
}

void DisplayWidget::renderTile(double pad, double time, int subframes, int w,
                               int h, int tile, int tileMax,
                               QProgressDialog *progress, int *steps,
                               QImage *im, const QTime &totalTime)
{
    tiles = tileMax;
    tilesCount = tile;
    padding = pad;
    mainWindow->setLastStoredTime ( time*renderFPS );

    makeCurrent();

    initPreviewBuffer();

    if (hiresBuffer == nullptr || hiresBuffer->width() != w || hiresBuffer->height() != h) {
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

    QString frametile = QString("%1.%2").arg ( tileMax*tileMax ).arg ( subframes );
    QString framesize = QString("%1x%2").arg ( tileMax*w ).arg ( tileMax*h );

    progress->setWindowTitle(tr( "Frame:%1/%2 Time:%3" )
                             .arg((int)(time * renderFPS))
                             .arg(framesToRender)
                             .arg(time, 8, 'g', 3, QChar(' ')));

    for ( int i = 0; i< subframes; i++ ) {

        if ( !progress->wasCanceled() ) {

            // compute ETA in ms
            int64_t total = progress->maximum();
            int64_t current = *steps;
            int64_t elapsed = totalTime.elapsed();
            int64_t eta = elapsed * (total - current) / (current + !current); // TODO verify ~ or !

            // format ETA to string
            const int hour = 60 * 60 * 1000;
            const int day = 24 * hour;
            int days = eta / day;
            eta %= day;
            bool hours = eta >= hour;
            QTime t(0,0,0,0);
            t=t.addMSecs((int) eta);
            if (days > 0) {
                renderETA = QString("%1:%2").arg(days).arg(t.toString("hh:mm:ss"));
            } else if (hours) {
                renderETA = t.toString("hh:mm:ss");
            } else {
                renderETA = t.toString("mm:ss");
            }

            progress->setValue ( *steps );
            progress->setLabelText ( tr( "<table width=\"100%\"> \
            <tr><td>Total</td><td align=\"center\">%1</td><td>Final Size: %2</td></tr> \
            <tr><td>Current</td><td align=\"center\">Tile: %3</td><td>Sub: %4</td></tr> \
            <tr><td>Avg sec/tile</td><td align=\"center\">%5</td><td>ETA: %6</td></tr> \
            </table>" )
                                    .arg ( frametile )
                                    .arg ( framesize )
                                    .arg ( tile )
                                    .arg ( i )
                                    .arg ( (tileAVG/(tile+1))/1000.0 )
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

void DisplayWidget::setState(DrawingState state)
{
    drawingState = state;
}

void DisplayWidget::paintGL()
{
    if ( drawingState == Tiled ) {
        return;
    }
    if (pixelHeight() == 0 || pixelWidth() == 0) {
        return;
    }

    bool doMain = pendingRedraws > 0;
    if (pendingRedraws > 0) {
        pendingRedraws--;
    }

    if (disabled || shaderProgram == nullptr) {
        glClearColor(backgroundColor.redF(), backgroundColor.greenF(), backgroundColor.blueF(), backgroundColor.alphaF());
        clearGL();
        return;
    }

    if ((doClearBackBuffer || drawingState == DisplayWidget::Animation) && backBuffer != nullptr) {
        if ( !initPreviewBuffer() ) {
            return;
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
        if (eyeSpline != nullptr) {
            int index = mainWindow->getFrame();
            glm::dvec3 e = eyeSpline->getSplinePoint ( index );
            glm::dvec3 t = targetSpline->getSplinePoint ( index );
            // camera path tracking makes for a bumpy ride
            //  t = eyeSpline->getSplinePoint( index+1 );
            glm::dvec3 u = upSpline->getSplinePoint ( index );
            glm::dvec3 zero = glm::dvec3(0.0,0.0,0.0);
            if ( e!=zero && t!=zero && u!=zero ) {
                mainWindow->setCameraSettings(e, t, normalize(u)); // normalizing Up here allows spline path animating
            }
        }
    }

    drawToFrameBufferObject( nullptr, (subframeCounter >= maxSubFrames && maxSubFrames > 0), doMain );

}

void DisplayWidget::updateBuffers()
{
    resizeGL ( 0,0 );
}

void DisplayWidget::resizeGL(int /* width */, int /* height */)
{
    // When resizing the perspective must be recalculated
    updatePerspective();
    makeBuffers();
    requireRedraw ( true );
}

void DisplayWidget::updatePerspective()
{
    if (pixelHeight() == 0 || pixelWidth() == 0) {
        return;
    }
    mainWindow->getBufferSize(pixelWidth(), pixelHeight(), bufferSizeX, bufferSizeY, fitWindow);
    QString infoText = tr("[%1x%2] Aspect=%3")
                       .arg(pixelWidth())
                       .arg(pixelHeight())
                       .arg((double)pixelWidth() / pixelHeight());
    mainWindow-> statusBar()->showMessage ( infoText, 5000 );
}

void DisplayWidget::timerSignal()
{

    static QWidget* lastFocusedWidget = QApplication::focusWidget();
    if (QApplication::focusWidget() != lastFocusedWidget && cameraControl != nullptr) {
        cameraControl->releaseControl();
        lastFocusedWidget = QApplication::focusWidget();
    }
    if (cameraControl != nullptr && cameraControl->wantsRedraw()) {
        requireRedraw ( clearOnChange );
        cameraControl->updateState();
    }

    if (isPending()) {
        if (buttonDown) {
            pendingRedraws = 0;
        }
        update();
    } else if ( continuous ) {
        if ( drawingState == Progressive &&
                ( subframeCounter>=maxSubFrames && maxSubFrames>0 ) ) {
            // we're done rendering
        } else {
            if (buttonDown) {
                return;
            }

            QTime t = QTime::currentTime();

            // render
            pendingRedraws = 1;
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
}

void DisplayWidget::showEvent(QShowEvent *ev)
{
    Q_UNUSED(ev)

    requireRedraw ( true );
}

void DisplayWidget::wheelEvent(QWheelEvent *ev)
{
    if (mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled) {
        return;
    }

    cameraControl->wheelEvent ( ev );
    requireRedraw ( clearOnChange );
    ev->accept();
}

void DisplayWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled) {
        return;
    }

    mouseXY = ev->pos();

    if( buttonDown) {
        bool redraw = cameraControl->mouseEvent ( ev, width(), height() );
        if ( redraw ) {
            requireRedraw ( clearOnChange );
            ev->accept();
        }
    }
}

void DisplayWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if (mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled) {
        return;
    }

    buttonDown=false;

    // if the user just clicked and didn't drag update the statusbar
    if ( ev->pos() == mouseXY ) {
        glm::dvec3 mXYZ = cameraControl->screenTo3D(mouseXY.x(), mouseXY.y(), ZAtMXY);
        // update statusbar
        mainWindow->statusBar()->showMessage(QString("X:%1 Y:%2 Z:%3").arg(mXYZ.x).arg(mXYZ.y).arg(mXYZ.z));
        if(ev->button() == Qt::MiddleButton) {
          // SpotLightDir = polar coords vec2 DE-Raytracer.frag
          // LightPos = vec3 DE-Kn2.frag
          if(ev->modifiers() == Qt::ControlModifier) {
            // placement of light in DE-Kn2.frag
                mainWindow->setParameter(QString("LightPos = %1,%2,%3")
                                         .arg(mXYZ.x)
                                         .arg(mXYZ.y)
                                         .arg(mXYZ.z));
          } else {
            // placement of target
                mainWindow->setParameter(QString("Target = %1,%2,%3")
                                         .arg(mXYZ.x)
                                         .arg(mXYZ.y)
                                         .arg(mXYZ.z));
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
                        glm::dvec3 e = glm::dvec3(in.at(0).toDouble(), in.at(1).toDouble(), in.at(2).toDouble());
                        // calculate distance between camera and target
                        double d = distance(mXYZ, e);
                        // set the focal plane to this distance
                        mainWindow->setParameter( "FocalPlane", d );
                        mainWindow->statusBar()->showMessage(
                            QString("X:%1 Y:%2 Z:%3 Dist:%4")
                            .arg(mXYZ.x)
                            .arg(mXYZ.y)
                            .arg(mXYZ.z)
                            .arg(d));
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

void DisplayWidget::mousePressEvent(QMouseEvent *ev)
{
    buttonDown=true;
    if (mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled) {
        return;
    }
    if (ev->button() == Qt::MouseButton::RightButton &&
            ev->modifiers() == Qt::ShiftModifier) {
        return;
    }

    mouseXY=ev->pos();

    bool redraw = cameraControl->mouseEvent ( ev, width(), height() );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    }

    //     mainWindow->statusBar()->showMessage ( QString ( "X:%1 Y:%2 Z:%3" ).arg( mXYZ.x() ).arg ( mXYZ.y() ).arg ( mXYZ.z() ) );
}

void DisplayWidget::keyPressEvent(QKeyEvent *ev)
{
    if (mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled) {
        return;
    }

    bool redraw = cameraControl->keyPressEvent ( ev );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    } else {
        QOpenGLWidget::keyPressEvent ( ev );
    }
}

void DisplayWidget::keyReleaseEvent(QKeyEvent *ev)
{
    if (mainWindow->getCameraSettings().isEmpty() || drawingState == Tiled) {
        return;
    }

    bool redraw = cameraControl->keyPressEvent ( ev );
    if ( redraw ) {
        requireRedraw ( clearOnChange );
        ev->accept();
    } else {
        QOpenGLWidget::keyReleaseEvent ( ev );
    }
}

void DisplayWidget::updateEasingCurves(int currentframe)
{
    static int cf = 0; // prevent getting called more than once per frame ?
    if (cf == currentframe) {
        return;
    }
    cf = currentframe;

    int count = curveSettings.count();

    for ( int i = 0; i < count; i++ ) {

        QString wName = curveSettings[i].split ( ":" ).first();
        QString wNum = wName.right ( 1 );
        wName = wName.left ( wName.length()-1 );
        ComboSlider *cs = (ComboSlider *)(mainWindow->getVariableEditor()->findChild<ComboSlider *>(QString("%1%2").arg(wName).arg(wNum)));

        if (cs->m_anim != nullptr) {
            cs->blockSignals ( true );
            int animframe = currentframe-cs->m_framestart;
            int loopduration = cs->m_framefin-cs->m_framestart;
            int singleframe = ( 1.0/renderFPS ) *1000; // in msecs
            auto maxttime = (int)(mainWindow->getTimeMax() * renderFPS);
            int endframe = cs->m_framefin;
            if (cs->m_loops > 0) {
                endframe *= cs->m_loops;
            }

            // test end frame against maxTime warn and stop if greater
            if ( endframe > maxttime ) {
                VariableWidget *vw = (VariableWidget *)(mainWindow->getVariableEditor()->findChild<VariableWidget *>(wName));
              QString gr;
                if (vw != nullptr) {
                    gr = vw->getGroup();
                } else {
                    gr = tr("Fix me");
                }

                QString mess = tr("Easingcurve end frame %1 is greater than maximum %2 \
              <br><br>Select <b>%3 -> %4</b> and press \"F7\" \
              <br><br>Or set the <b>animation length</b> greater than <b>%5</b>" )
                    .arg(endframe)
                    .arg(maxttime)
                    .arg(gr)
                    .arg(wName)
                    .arg((int)((cs->m_framestart + endframe) / renderFPS));
                QMessageBox::warning(this, tr("Easing Curve Error"), mess);

              mainWindow->setTimeSliderValue ( currentframe );
              mainWindow->stop();

              return;
            }
            if ( currentframe < cs->m_framestart ) {
                cs->m_anim->setCurrentTime(1); // preserve the startvalue for prior frames
            } else if (currentframe < cs->m_framestart + (loopduration * cs->m_loops)) { // active!

                if (cs->m_pong == 1) { // pingpong = 1 loop per so 4 loops = ping pong ping pong

                    int test = ( animframe/loopduration ); // for even or odd loop

                    if ( ( int ) ( test*0.5 ) *2 == test ) {
                        cs->m_anim->setCurrentTime ( ( animframe*singleframe ) +1 );
                    } else {
                        cs->m_anim->setCurrentTime((((loopduration * cs->m_loops) - animframe) * singleframe) - 1);
                    }
                } else {
                    cs->m_anim->setCurrentTime((animframe * singleframe) - 1);
                }
            } else { // preserve the end value for remaining frames
                if (cs->m_pong == 1 && (int)(cs->m_loops * 0.5) * 2 == cs->m_loops) { // even or odd loop
                  cs->m_anim->setCurrentTime ( 1 );
                } else {
                  cs->m_anim->setCurrentTime ( cs->m_anim->duration()-1 );
                }
            }
            cs->blockSignals ( false );
        }
    }
}

void DisplayWidget::drawLookatVector()
{
    glm::dvec3 ec = eyeSpline->getSplinePoint ( mainWindow->getTime() +1 );
    glm::dvec3 tc = targetSpline->getSplinePoint ( mainWindow->getTime() +1 );
    glColor4f ( 1.0,1.0,0.0,1.0 );
    glBegin ( GL_LINE_STRIP );
    glVertex3f ( ec.x,ec.y,ec.z );
    glVertex3f ( tc.x,tc.y,tc.z );
    glEnd();
}

void DisplayWidget::setPerspective()
{

    QStringList cs = mainWindow->getCameraSettings().split ( "\n" );
    double fov = cs.filter ( "FOV" ).at ( 0 ).split ( "=" ).at ( 1 ).toDouble();
    QStringList cv = cs.filter ( "Eye " ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    glm::dvec3 eye = glm::dvec3(cv.at(0).toDouble(), cv.at(1).toDouble(), cv.at(2).toDouble());
    cv = cs.filter ( "Target" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    glm::dvec3 target = glm::dvec3(cv.at(0).toDouble(), cv.at(1).toDouble(), cv.at(2).toDouble());
    cv = cs.filter ( "Up" ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
    glm::dvec3 up = glm::dvec3(cv.at(0).toDouble(), cv.at(1).toDouble(), cv.at(2).toDouble());

    auto aspectRatio = double((double)width() / (double)height());
    double zNear = 0.00001;
    double zFar = 1000.0;
    double vertAngle = 180.0 * ( 2.0 * atan2 ( 1.0, ( 1.0/fov ) ) / M_PI );

    glm::dmat4 matrix;
    matrix = glm::perspective ( vertAngle, aspectRatio, zNear, zFar );
    matrix = matrix * glm::lookAt ( eye,target,up );

    /// BEGIN 3DTexture
    //     texMatrix = matrix;
    /// END 3DTexture

    glLoadMatrixd ( &matrix[0][0] );
}

QStringList DisplayWidget::getCurveSettings()
{
    return curveSettings;
}

void DisplayWidget::setCurveSettings(const QStringList cset)
{
    mainWindow->getVariableEditor()->setEasingEnabled ( !cset.isEmpty() );
    curveSettings = cset;
    if (verbose) {
        qDebug() << cset;
    }
}

void DisplayWidget::drawSplines()
{
    // this lets splines be visible when DEPTH_TO_ALPHA mode is active
    if (depthToAlpha) {
        glDepthFunc ( GL_ALWAYS );
    } else {
        glDepthFunc ( GL_LESS );    // closer to eye passes test
        glDepthMask ( GL_FALSE );   // no Writing to depth buffer for splines
    }

    // control point to highlight = currently selected preset keyframe
    int p = mainWindow->getCurrentCtrlPoint();

    targetSpline->drawSplinePoints();
    targetSpline->drawControlPoints ( p );

    eyeSpline->drawSplinePoints();
    eyeSpline->drawControlPoints ( p );

    // TODO add vectors to spline control points and enable editing of points with
    // mouse
}

void DisplayWidget::createSplines(int numberOfControlPoints, int numberOfFrames)
{
    if( cameraID() == "3D" ) {
        auto *eyeCp = (glm::dvec3 *)eyeControlPoints.constData();
        auto *tarCp = (glm::dvec3 *)targetControlPoints.constData();
        auto *upCp = (glm::dvec3 *)upControlPoints.constData();

        if (eyeCp != nullptr && tarCp != nullptr && upCp != nullptr) {
            eyeSpline = new QtSpline(this, numberOfControlPoints, numberOfFrames, eyeCp);
            targetSpline = new QtSpline(this, numberOfControlPoints, numberOfFrames, tarCp);
            upSpline = new QtSpline(this, numberOfControlPoints, numberOfFrames, upCp);

            eyeSpline->setSplineColor( QColor("red") );
            eyeSpline->setControlColor( QColor("green"));

            targetSpline->setSplineColor( QColor("blue"));
            targetSpline->setControlColor( QColor("green"));
        }
    }
}

void DisplayWidget::addControlPoint(glm::dvec3 eP, glm::dvec3 tP, glm::dvec3 uP)
{
    eyeControlPoints.append ( eP );
    targetControlPoints.append ( tP );
    upControlPoints.append ( uP );
}

void DisplayWidget::clearControlPoints()
{
    eyeControlPoints.clear();
    targetControlPoints.clear();
    upControlPoints.clear();
}
} // namespace GUI
} // namespace Fragmentarium
