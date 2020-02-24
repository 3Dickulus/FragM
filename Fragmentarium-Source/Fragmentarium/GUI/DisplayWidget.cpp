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

#ifdef Q_OS_LINUX
#ifdef USE_OPENGL_4
void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "GL Debug message (" << id << "): " <<  message << " ";

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "API "; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Window System "; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Shader Compiler "; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Third Party "; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Application "; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Other"; break;
        default:           std::cout << "Source:" << source << " unknown"; break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Error "; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Deprecated Behaviour "; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Undefined Behaviour "; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Portability "; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Performance "; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Marker "; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Push Group "; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Pop Group "; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Other "; break;
        default:           std::cout << "Type:" << type << " unknown"; break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high "; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium "; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low "; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification "; break;
        default:           std::cout << "Severity:" << severity << " unknown"; break;
    }

    std::cout << std::endl;
}
#endif
#endif

GLenum DisplayWidget::glCheckError_(const char *file, int line, const char *func)
{
    GLenum errorCode = GL_NO_ERROR;
    
    if( glDebugEnabled ) {
        while ((errorCode = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "-> " << func << "() @ line " << line << std::endl;
        }
    }
    return errorCode;
}

// used immediately after glFunctions() to get line numbers with gl debug output
#define glCheckError() glCheckError_(__FILE__, __LINE__, __FUNCTION__) 

DisplayWidget::DisplayWidget ( MainWindow* mainWin, QWidget* parent )
    : QOpenGLWidget(parent), mainWindow(mainWin)
{

    QSurfaceFormat fmt;
    fmt.setSwapInterval(0);
    fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
#ifdef USE_OPENGL_4
    fmt.setVersion(4,5);
    fmt.setDepthBufferSize(32);
#else
    fmt.setVersion(3,2);
#endif

#ifdef Q_OS_MAC
    fmt.setVersion(4,1);
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
    buttonDown = false;
    updateRefreshRate();
    glDebugEnabled = false;
}

void DisplayWidget::initializeGL()
{

    initializeOpenGLFunctions();

    vendor = QString ( ( char * ) glGetString ( GL_VENDOR ) );
    renderer = QString ( ( char * ) glGetString ( GL_RENDERER ) );
    glvers = QString ( ( char * ) glGetString ( GL_VERSION ) );
    /// test for nVidia card and set the nV flag
    foundnV = vendor.contains ( "NVIDIA", Qt::CaseInsensitive );

#ifdef Q_OS_LINUX
#ifdef USE_OPENGL_4
    // Enable debug output
    QSettings settings;
    if( settings.value("enableGLDebug").toBool() ) {
        glDebugEnabled = true;
        glEnable ( GL_DEBUG_OUTPUT );
        glEnable ( GL_DEBUG_OUTPUT_SYNCHRONOUS );
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        glDebugMessageCallback( MessageCallback, nullptr );
    }
#endif
#endif

}

void DisplayWidget::updateRefreshRate()
{

    QSettings settings;
    int i = settings.value ( "refreshRate", 40 ).toInt(); // 25 fps
    if (timer == nullptr) {
        timer = new QTimer();
        timer->setTimerType(Qt::PreciseTimer);
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
            contextMenu->move( mapToGlobal( ev->pos() ));
        }
    }
}

void DisplayWidget::setFragmentShader(FragmentSource fs)
{

    GLint s;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s); // GL_MAX_3D_TEXTURE_SIZE GL_MAX_CUBE_MAP_TEXTURE_SIZE
    INFO ( tr( "Maximum texture size: %1x%1" ).arg ( s ) );
    
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

    requireRedraw ( true );

    initFragmentShader();
    
    if (shaderProgram == nullptr || !shaderProgram->isLinked()) { // something went wrong so do not try to setup textures or buffer shader
        return;
    }

    initBufferShader();

    if(!(nullptr == fragmentSource.bufferShaderSource)) {
        if(bufferShaderProgram == nullptr || !bufferShaderProgram->isLinked()) {
            return;
        }
    }    
}

void DisplayWidget::requireRedraw(bool clear )
{

    if (disableRedraw) {
        return;
    }

    if ( clear ) {
        doClearBackBuffer = true;
        pendingRedraws = 1;
        bufferUniformsHaveChanged = true; // fixed bad image after rebuild
    } else if ( bufferShaderOnly ) {
        pendingRedraws = 1;
    } else if ( clearOnChange ) {
        subframeCounter = 0;
    }
}

void DisplayWidget::uniformsHaveChanged(bool bshaderonly)
{

    if(fragmentSource.depthToAlpha) {
        BoolWidget *btest = dynamic_cast<BoolWidget *>(mainWindow->getVariableEditor()->getWidgetFromName("DepthToAlpha"));
        if (btest != nullptr) {
            // widget detected
            depthToAlpha = btest->isChecked();
        }
    }
    bufferShaderOnly = bshaderonly;
    bufferUniformsHaveChanged |= bufferShaderOnly;
    requireRedraw ( bufferShaderOnly ? false : clearOnChange );
}

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

#ifndef USE_OPENGL_4
                return QStringList("This build is compiled without support for OpenGL 4!");
#else

    QStringList asmList;
    if (!foundnV || format().majorVersion() < 4 ) {
                asmList = QStringList("nVidia GL > 4.0 required for this feature!");
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

        // this locates all of the uniform names
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

    // contains ALL uniforms in buffershader and shader program not optimized out by the compiler
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();
    // find a value to go with the name, index in the program may not be the same as index in our list
    for( int n=0; n < vw.count(); n++) {
        asmTxt = vw[n]->getName();
        if( asmList.indexOf( asmTxt ) != -1 ){
            int uLoc = w ? shaderProgram->uniformLocation(asmTxt) : bufferShaderProgram->uniformLocation(asmTxt);
            if(uLoc != -1) {
                // TODO: add types float vec3 etc.
                QString newLine = vw[n]->isLocked() ? vw[n]->getLockedSubstitution() : QString("%1 = %2;").arg(asmTxt).arg(vw[n]->getValueAsText());
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
#endif
}

void DisplayWidget::createErrorLineLog ( QString message, QString log, LogLevel priority, bool bS )
{

    QStringList logList = log.split ( "\n" );
    QRegExp num ( "([0-9]+)" );

    int errorCount=0;
    int errorLine=0;
    int fileIndex=0;

    LOG ( message, priority );

    foreach ( const QString &str, logList ) {
        QString newStr=str;

        QRegExp test ( "^([0-9]+)[^0-9]([0-9]+)[^0-9]" );
        if ( test.indexIn ( str ) != -1 ) {
            if ( num.indexIn ( test.cap ( 1 ) ) != -1 ) {
                fileIndex=num.cap ( 1 ).toInt();
                if(!bS)
                    newStr.replace ( 0,num.cap ( 1 ).length(), fragmentSource.sourceFileNames[fileIndex] + " " );
                else
                    newStr.replace ( 0,num.cap ( 1 ).length(), fragmentSource.bufferShaderSource->sourceFileNames[0] + " " );
            }
            if ( num.indexIn ( test.cap ( 2 ) ) != -1 )
                errorLine=num.cap ( 1 ).toInt();
            errorCount++;
        }

        // emit a single log widget line for each line in the log
        LOG ( newStr, priority );

        QSettings settings; // only jump to first error as later errors may be caused by this one so fix it first
        if ( (settings.value ( "jumpToLineOnWarn", true ).toBool() || settings.value ( "jumpToLineOnError", true ).toBool())) {
            if(errorCount == 1 && (fragmentSource.sourceFileNames[0] == fragmentSource.sourceFileNames[fileIndex]) && !bS) {
                // jump to error line in text editor
                TextEdit *te = mainWindow->getTextEdit();
                QTextCursor cursor(te->textCursor());
                cursor.setPosition(0);
                cursor.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,errorLine-1);
                te->setTextCursor( cursor );
                te->centerCursor();
            }
        }
    }
}

void DisplayWidget::initFragmentShader()
{

    if (shaderProgram != nullptr) {
        shaderProgram->release();
        delete ( shaderProgram );
        shaderProgram = nullptr;
    }

    QSettings settings;

    makeCurrent();

    shaderProgram = new QOpenGLShaderProgram ( context() );

    // Vertex shader
    bool s = shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, fragmentSource.vertexSource.join("\n"));
    if ( fragmentSource.vertexSource.count() == 0 ) {
        WARNING ( tr("No vertex shader found!") );
        s = false;
    }

    if ( !s ) {
        createErrorLineLog( tr("Could not create vertex shader: "), shaderProgram->log(), WarningLevel, false );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }
    if (!shaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Vertex shader compiled with warnings: "), shaderProgram->log(), InfoLevel ,false );
    }

    // Fragment shader
    s = shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource.getText());

    if (s) { // Requests the shader program's id to be created immediately.
        s = shaderProgram->create();
    }

    if ( !s ) {
        createErrorLineLog( tr("Could not create fragment shader: "), shaderProgram->log(), WarningLevel, false );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }

    if (!shaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Fragment shader compiled with warnings: "), shaderProgram->log(), InfoLevel, false );
    }

    s = shaderProgram->link();

    if ( !s ) {
        WARNING ( tr("Could not link buffershader: ") );
        CRITICAL ( shaderProgram->log() );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }

    if (!shaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Fragment shader compiled with warnings: "), shaderProgram->log(), InfoLevel, false );
    }

    s = shaderProgram->bind();
    if ( !s ) {
        WARNING ( tr("Could not bind shaders: ") + shaderProgram->log() );
        delete ( shaderProgram );
        shaderProgram = nullptr;
        return;
    }
    
    // Setup backbuffer texture for this shader
    if ( bufferType != 0 ) {
        // Bind first texture to backbuffer
        glActiveTexture ( GL_TEXTURE0 ); // non-standard (>OpenGL 1.3) gl extension

        int l = shaderProgram->uniformLocation ( "backbuffer" );
        if ( l != -1 ) {
              GLuint i = backBuffer->texture();
              glBindTexture ( GL_TEXTURE_2D, i );
              if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                  setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
              }
              shaderProgram->setUniformValue ( l, ( GLuint ) 0 );
        }
    } else {
        WARNING ( tr("Trying to use a backbuffer, but no bufferType set.") );
        WARNING ( tr("Use the buffer define, e.g.: '#buffer RGBA8' ") );
    }

    if (fragmentSource.textures.count() != 0) {
        initFragmentTextures();
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

bool DisplayWidget::loadHDRTexture ( QString texturePath, GLenum type, GLuint textureID, QString textureUniformName )
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

//
// Read an RGBA image using class InputFile:
//
//    - open the file
//    - allocate memory for the pixels
//    - describe the memory layout of the pixels
//    - determine the channel(s)
//    - read the pixels from the file
//

bool DisplayWidget::loadEXRTexture(QString texturePath, GLenum type, GLuint textureID, QString textureUniformName)
{

#ifndef USE_OPEN_EXR
    // Qt loads EXR files
    return loadQtTexture(texturePath, type, textureID, textureUniformName);
#else
    InputFile file ( texturePath.toLatin1().data() );
    Box2i dw = file.header().dataWindow();

    int chn = -1;
    QString chv = "";
    bool hasZ = false;
    
    if ( file.isComplete() ) {

        int channelCount=0;
        const ChannelList &channels = file.header().channels();
        for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {
            if(verbose && subframeCounter==1) {
                std::cout << "Channel:" << channelCount << " " << i.name() << std::endl;
            }
            channelCount++;
        }

        int w  = dw.max.x - dw.min.x + 1;
        int h = dw.max.y - dw.min.y + 1;
        int s;
        
        glGetIntegerv ( GL_MAX_TEXTURE_SIZE, &s );

        if (type == GL_SAMPLER_2D && (w>s || h>s) ) {
            WARNING(tr("Exrloader found EXR image: %1 x %2 is too large! max %3x%3").arg(w).arg(h).arg(s));
            return false;
        }

        if(type == GL_SAMPLER_CUBE && (w>s || h != 6*w)){
            WARNING(tr("Exrloader found EXR image: %1 x %2 is not a cube map!").arg(w).arg(h));
            return false;
        }
        
        Array2D<RGBAFLOAT>pixels ( w, 1 );
        
        if(type == GL_SAMPLER_2D) {
            SamplerWidget *sw = dynamic_cast<SamplerWidget *>(mainWindow->getVariableEditor()->getWidgetFromName(textureUniformName));
            if(sw != nullptr && !sw->getName().isNull()) {
                chv = sw->getChannelValue(); // channel name(s)
                if(!chv.contains("Z") && !chv.contains("DEPTH") && !chv.contains("D"))
                    hasZ = false;
                else
                    hasZ = (sw->channelList.contains("Z") || sw->channelList.contains("DEPTH") || sw->channelList.contains("D"));
                
                if(chv.contains("All") && sw->channelList.contains("Z")) hasZ = true;
                
//                 chn = sw->hasChannel(chv); // channel index
//                 DBOUT << "Using:" << sw->getName() << sw->getValue() << chv << chn;
            }
        } else chv = "All";

        pixels.resizeErase (w, h);
        
        size_t xs = 1 * sizeof (RGBAFLOAT);
        size_t ys = w * sizeof (RGBAFLOAT);
        
        auto *cols = new float[w * h * 4];
        
        while ( dw.min.y <= dw.max.y ) {

            RGBAFLOAT *base = &pixels[0][0] - dw.min.x - dw.min.y * w;

            FrameBuffer fb;

            fb.insert ("R", Slice (Imf::FLOAT, (char *) &base[0].r, xs, ys));
            fb.insert ("G", Slice (Imf::FLOAT, (char *) &base[0].g, xs, ys));
            fb.insert ("B", Slice (Imf::FLOAT, (char *) &base[0].b, xs, ys));
            if(!hasZ) // no Z DEPTH or 4th so add an alpha channel and fill with appropriate value
                fb.insert ("A", Slice (Imf::FLOAT, (char *) &base[0].a, xs, ys));
            else // this should allow arbitrary naming of the 4th channel
                fb.insert ("Z", Slice (Imf::FLOAT, (char *) &base[0].a, xs, ys));

            file.setFrameBuffer (fb);
            file.readPixels ( dw.min.y );

            // process scanline (pixels)
            for ( int i = 0; i<w; i++ ) {
                // convert 3D array to 1D
                int indx = ( dw.min.y*w+i ) *4;
                cols[indx] = (chv.contains("R") || chv.contains("All")) ? pixels[0][i].r : 0.0;
                cols[indx+1]=(chv.contains("G") || chv.contains("All")) ? pixels[0][i].g : 0.0;
                cols[indx+2]=(chv.contains("B") || chv.contains("All")) ? pixels[0][i].b : 0.0;
                cols[indx+3]=(chv.contains("A") || chv.contains("All") || chv.contains("Z") || hasZ) ? pixels[0][i].a : 1.0; // always put 4th channel in alpha slot?
            }
            dw.min.y ++;
        }

        glBindTexture((type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID);

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
#endif
    return true;
}

// Qt format image, Qt 5+ loads EXR format on linux
bool DisplayWidget::loadQtTexture(QString texturePath, GLenum type, GLuint textureID, QString textureUniformName)
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

    } else if (fragmentSource.textureParams.contains(textureUniformName)) {
        // set texparms
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
    if (shaderProgram == nullptr || fragmentSource.textures.count() < 1 || !checkShaderProg(shaderProgram->programId()) ) {
        return; // something went wrong so do not try to setup textures
    }

    GLuint u = 1; // the backbuffer is always 0 while textures from uniforms start at 1
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

            if ( !(l < 0) ) { // found named texture in shader program

                // 2D or Cube ?
                GLsizei bufSize = 0;
                GLsizei length = 0;
                GLint size = 0;
                GLenum type = 0;
                
                glGetProgramiv(shaderProgram->programId(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufSize);
                GLchar name[bufSize];
                
                // bugfix for textures by claude #104 index vs location
                GLuint idx;
                // get a pointer to the char array in QString
                GLchar *oneName = textureUniformName.toLatin1().data();
                // returns index of "oneName" in idx;
                glGetUniformIndices( shaderProgram->programId(), 1, &oneName, &idx);
                // use idx to acquire more info about this uniform
                glGetActiveUniform(shaderProgram->programId(), idx, bufSize, &length, &size, &type, name);
                // set current texture
                glActiveTexture(GL_TEXTURE0 + u); // non-standard (>OpenGL 1.3) gl extension

                if(textureUniformName == QString(name).trimmed() && (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE) ) {
                    // check cache first
                    if ( !TextureCache.contains ( texturePath ) ) {
                        // if not in cache then create one and try to load and add to cache
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // byte alignment 4 bytes = 32 bits
                        // allocate a texture id
                        glGenTextures ( 1, &textureID );

                        if (verbose) {
                            qDebug() << QString("Allocating texture (ID: %1) %2").arg(textureID).arg(texturePath);
                        }

                        if (texturePath.endsWith(".hdr", Qt::CaseInsensitive)) { // is HDR format image ?
                            loaded = loadHDRTexture(texturePath, type, textureID);
                        }
                        else if (texturePath.endsWith(".exr", Qt::CaseInsensitive)) { // is EXR format image ?
                            loaded = loadEXRTexture(texturePath, type, textureID, textureUniformName);
                        }
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
                        if (verbose) {
                            qDebug() << QString("Using cached texture (ID: %1) %2").arg(textureID).arg(texturePath);
                        }
                    }
                }

                if ( loaded ) {
                    glBindTexture((type == GL_SAMPLER_CUBE) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, textureID);
                    if( setTextureParms(textureUniformName, type) ) {
                        // Texture is loaded and bound successfully
//                         if(verbose && subframeCounter == 1) {
//                             qDebug() <<  "Texture index:" << u << " ID:" << textureID << " Name:" << textureUniformName << " Location:" << l;
//                             break;
//                         }
                    }
                } else {
                    WARNING(tr("Not a valid texture: ") + QFileInfo(texturePath).absoluteFilePath());
                }
                u++;
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
        createErrorLineLog( tr("Could not create buffer vertex shader: "), bufferShaderProgram->log(), WarningLevel , true );
        delete ( bufferShaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
    if (!bufferShaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Buffer vertex shader compiled with warnings: "), bufferShaderProgram->log(), InfoLevel, true );
    }

    // Fragment shader
    s = bufferShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource.bufferShaderSource->getText());

    if (s) {
        s = bufferShaderProgram->create();
    }

    if ( !s ) {
        createErrorLineLog( tr("Could not create buffer fragment shader: "), bufferShaderProgram->log(), WarningLevel, true );
        delete ( bufferShaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
    if (!bufferShaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Buffer fragment shader compiled with warnings: "), bufferShaderProgram->log(), InfoLevel, true );
    }

    s = bufferShaderProgram->link();

    if ( !s ) {
        WARNING ( tr("Could not link buffershader: ") );
        CRITICAL ( bufferShaderProgram->log() );
        delete ( bufferShaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
    if (!bufferShaderProgram->log().isEmpty()) {
        createErrorLineLog( tr("Buffer fragment shader compiled with warnings: "), bufferShaderProgram->log(), InfoLevel, true );
    }

    s = bufferShaderProgram->bind();
    if ( !s ) {
        WARNING ( tr("Could not bind shaders: ") + bufferShaderProgram->log() );
        delete ( shaderProgram );
        bufferShaderProgram = nullptr;
        return;
    }
    
    bufferUniformsHaveChanged = false;
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
    doClearBackBuffer = true;

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

bool DisplayWidget::checkShaderProg(GLuint programID)
{

    if(hasShader() && programID == shaderProgram->programId()) {
        return shaderProgram->isLinked();
    } else if (hasBufferShader() && programID == bufferShaderProgram->programId()) {
        return bufferShaderProgram->isLinked();
    } else {
            WARNING("NO ShaderProgram!!!");
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

// returns text name of type -> tp
void DisplayWidget::get32Type(GLenum type, QString &tp)
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

#ifdef USE_OPENGL_4
// returns text name of type -> tp and verified foundDouble flag
void DisplayWidget::get64Type(GLuint programID, GLenum type, QString uniformName, QString uniformValue, bool &foundDouble, QString &tp)
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
#endif

void DisplayWidget::setShaderUniforms(QOpenGLShaderProgram *shaderProg)
{
    GLuint programID = shaderProg->programId();

    if(!checkShaderProg(programID)) return;

    // contains ALL uniforms in buffershader and shader program
    QVector<VariableWidget*> vw = mainWindow->getUserUniforms();

    int count;
    // this only returns uniforms that have not been optimized out
    glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);

    for (uint i = 0; i < count; i++) {

        GLsizei bufSize = 256;
        GLsizei length;
        GLint size;
        GLenum type;
        GLchar name[bufSize];

        glGetActiveUniform(programID, i, bufSize, &length, &size, &type, name);

        QString tp = "";
        get32Type(type, tp);
        bool foundDouble = false;

        QString uniformName = (char *)name;
        QString uniformValue = "";
        checkForSpecialCase(name, uniformValue);
        if(uniformValue.contains("variable")) {
            // type name and value to console
            if (verbose && subframeCounter == 1) {
                qDebug() << tp << "\t" << uniformName << uniformValue;
            }
            continue;
        }

            // find a value to go with the name, index in the program, may not be the same as index in our list
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

        if(uniformValue.isEmpty()) {
            uniformValue = "Unused variable";
            continue;
        }

#ifdef USE_OPENGL_4
        if (format().majorVersion() > 3 && format().minorVersion() >= 0) {
            // do not try to set special, gl_ or unused uniform even if it is double type
            if (!uniformValue.contains("variable")) {
                get64Type(programID, type, uniformName, uniformValue, foundDouble, tp);
            }
        }
#endif

        // this sets User uniforms not handled above, sets sampler uniform texture ID
        if(!foundDouble) {
            for( int n=0; n < vw.count(); n++) {
                if(uniformName == vw[n]->getName()) {
                    auto *sw = dynamic_cast<SamplerWidget *>(vw[n]);
                    if (sw != nullptr) {
                        QMapIterator<QString, QString> it( fragmentSource.textures );
                        int u=1; // sampler textures start at 1
                        while( it.hasNext() ) {
                            it.next();
                            if(it.value().contains(sw->getValue())) { // the cached value is GLAPI texID, glsl wants indexOf!
                                sw->texID = u;//TextureCache[it.value()];
                                sw->setUserUniform(shaderProg);
//                                 DBOUT << "Sampler:" << uniformName << " FragMtexID:" << sw->texID << " CacheID:" << TextureCache[it.value()];
                            }
                            u++;
                        }
                    } else { 
                        vw[n]->setIsDouble(foundDouble); // ensure float sliders set to float decimals
                        vw[n]->setUserUniform(shaderProg);
                    }
                    break;
                }
            }
        } else {
            vw[i]->setIsDouble(foundDouble);
        } // this takes care of buffershader (Post) sliders :D

        // type name and value to console
        if (verbose && subframeCounter == 1) {
            qDebug() << tp << "\t" << uniformName << uniformValue;
        }
    }
    if (verbose && subframeCounter == 1) {
        qDebug() << count << " active uniforms initialized\n";
        if (shaderProg == shaderProgram) {
            if ( mainWindow->getVariableEditor()->hasEasing() ) {
                QStringList cs = curveSettings;
                while(!cs.isEmpty()) {
                    qDebug() << cs.at(0);
                    cs.removeFirst();
                }
                qDebug() << curveSettings.count() << " active easingcurve settings." << endl;
            }
        }
    }

    if(hasBufferShader() && programID == bufferShaderProgram->programId()) {
        bufferUniformsHaveChanged = false;
        bufferShaderOnly = false;
    }
}

void DisplayWidget::setupShaderVars(QOpenGLShaderProgram *shaderProg, int w, int h)
{

    cameraControl->transform(pixelWidth(), pixelHeight()); // -- Modelview + loadIdentity
    int l = shaderProg->uniformLocation ( "pixelSize" );

    if ( l != -1 ) {
        shaderProg->setUniformValue ( l, ( float ) ( 1.0/w ), ( float ) ( 1.0/h ) );
    }
    // Only in DepthBufferShader.frag & NODE-Raytracer.frag
    l = shaderProg->uniformLocation ( "globalPixelSize" );

    if ( l != -1 ) {
        shaderProg->setUniformValue ( l, ( ( float ) 1.0/w ), ( ( float ) 1.0/h ) );
    }

    l = shaderProg->uniformLocation ( "time" );

    if ( l != -1 ) {
        double t = mainWindow->getTime() / ( double ) renderFPS;
        shaderProg->setUniformValue ( l, ( float ) t );
    } else {
        mainWindow->getTime();
    }

    if ( bufferType!=0 ) {
        glActiveTexture ( GL_TEXTURE0 ); // non-standard (>OpenGL 1.3) gl extension
        GLuint i = backBuffer->texture();
        glBindTexture ( GL_TEXTURE_2D, i );

        l = shaderProg->uniformLocation ( "backbuffer" );
        if ( l != -1 ) {
            if (verbose && subframeCounter == 1) {
                qDebug() << QString("Binding backbuffer (ID: %1) to active texture %2").arg(i).arg(0);
            }
            if ( fragmentSource.textureParams.contains ( "backbuffer" ) ) {
                setGlTexParameter ( fragmentSource.textureParams["backbuffer"] );
            }
            shaderProg->setUniformValue ( l, 0 );
        }

        l = shaderProg->uniformLocation ( "subframe" );

        if ( l != -1 ) {
            shaderProg->setUniformValue ( l, subframeCounter );
            // if(verbose) qDebug() << QString("Setting subframe: %1").arg(subframeCounter);
        }
    }
    
        l = shaderProg->uniformLocation ( "frontbuffer" );

        if ( l != -1 ) {
            shaderProg->setUniformValue ( l, 0 );
        } else {
            if(shaderProg == bufferShaderProgram) {
                WARNING(tr("No front buffer sampler found in buffer shader. This doesn't make sense."));
            }
        }
}

void DisplayWidget::draw3DHints()
{

    if(!hasKeyFrames) return;

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

    shaderProgram->bind();

    // -- Viewport
    if ( toBuffer ) {
        glViewport ( 0, 0,w,h );
    } else {
        setViewPort ( w,h );
    }

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

    // builtin vars provided by FragM like time, subframes, frontbuffer, backbuffer
    // these are constantly changing so need to be set every subframe
    setupShaderVars(shaderProgram, w, h);

    // Setup User Uniforms, this should speed things up a little
    // we are only setting uniforms on the first tile and first subframe
    if (tilesCount == 0 && subframeCounter == 1) {
        setShaderUniforms(shaderProgram);
    }

    // save current state
    glPushAttrib ( GL_ALL_ATTRIB_BITS );

    glColor4f ( 1.0,1.0,1.0,1.0 );

    glDepthFunc ( GL_ALWAYS );   // always passes test so we write color
    glEnable ( GL_DEPTH_TEST );  // enable depth testing
    glDepthMask ( GL_TRUE );     // enable depth buffer writing

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

void DisplayWidget::drawToFrameBufferObject(QOpenGLFramebufferObject *buffer, bool drawLast)
{

    if (!this->isValid() || !FBOcheck()) {
        return;
    }

    QSize s = backBuffer->size();

    if ( !drawLast && !bufferShaderOnly ) {
        for ( int i = 0; i <= iterationsBetweenRedraws; i++ ) {
            if (backBuffer != nullptr) {
                // swap backbuffer
                QOpenGLFramebufferObject* temp = backBuffer;
                backBuffer = previewBuffer;
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
        WARNING ( tr("Failed to bind hires buffer") );
        return;
    }

    // Draw a textured quad using the preview texture.
    if (bufferShaderProgram != nullptr) {
        bufferShaderProgram->bind();
        setupShaderVars(bufferShaderProgram, s.width(),s.height());
        // Setup User Uniforms
        if (bufferUniformsHaveChanged) {
                setShaderUniforms ( bufferShaderProgram );
        }
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
    glDepthMask ( GL_FALSE ); // No writing: output is color data from post effects

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
    subframeCounter=0;
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
bool DisplayWidget::getRGBAFtile(Array2D<RGBAFLOAT> &array, int w, int h)
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
                array[(h - 1) - i][j] = RGBAFLOAT(
                                            myImgdata[((i * w) + j) * sizeof(float) + 0],
                                            myImgdata[((i * w) + j) * sizeof(float) + 1],
                                            myImgdata[((i * w) + j) * sizeof(float) + 2],
                                            depthToAlpha ? myDepths[((i * w) + j)] : myImgdata[((i * w) + j) * sizeof(float) + 3]);
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
    QString framesize = QString("%1x%2").arg ( (tileMax*w)/(1.0 + padding) ).arg ( (tileMax*h)/(1.0 + padding) );

    progress->setWindowTitle(tr( "Frame:%1/%2 Time:%3" )
                             .arg((int)(time * renderFPS))
                             .arg(renderToFrame)
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

    drawToFrameBufferObject( nullptr, (subframeCounter >= maxSubFrames && maxSubFrames > 0) );

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

    if (pendingRedraws != 0) {
        if (buttonDown) {
            pendingRedraws = 1;
        }
        update();
    } else if ( continuous ) {
        if ( drawingState == Progressive &&
                ( subframeCounter>=maxSubFrames && maxSubFrames>0 ) ) {
            // we're done rendering
            pendingRedraws = 0;
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
    double vertAngle = 2.0 * atan2 ( 1.0, ( 1.0/fov ) );

    glm::dmat4 matrix;
    matrix = glm::perspective ( vertAngle, aspectRatio, zNear, zFar );
    matrix = matrix * glm::lookAt ( eye,target,up );

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
}

void DisplayWidget::drawSplines()
{

    // this lets splines be visible when DEPTH_TO_ALPHA mode is active
    if (depthToAlpha) {
        glDepthFunc ( GL_ALWAYS );
    } else {
        glDepthFunc ( GL_LESS );  // closer to eye passes test
        glDepthMask ( GL_FALSE ); // no Writing to depth buffer for splines
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
