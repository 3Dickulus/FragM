#pragma once

#include <QShortcut>
#include <QAbstractScrollArea>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabBar>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextEdit>
#include <QtScript/QtScript>

#include <QOpenGLShaderProgram>

#include <qfilesystemwatcher.h>

#include "OutputDialog.h"
#include "DisplayWidget.h"
#include "Highlighter.h"
#include "VariableEditor.h"
#include "VariableWidget.h"
#include "Version.h"

#include "asmbrowser.h"
#include <QScriptEngineDebugger>

#define DBOUT qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__

class QScriptEngineDebugger;
class QAction;
class QMenu;

namespace Fragmentarium
{
namespace GUI
{

#ifdef USE_OPEN_EXR
using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
#endif

using namespace SyntopiaCore::Misc;
using namespace SyntopiaCore::Logging;
using namespace SyntopiaCore::Exceptions;
using namespace Fragmentarium::Parser;

class TextEdit;

// Information about the current tab
struct TabInfo {
    TabInfo() {}
    TabInfo ( QString filename, TextEdit *textEdit, bool unsaved = true,
              bool hasBeenSavedOnce = false )
        : filename ( filename ), textEdit ( textEdit ), unsaved ( unsaved ),
          hasBeenSavedOnce ( hasBeenSavedOnce ) {}
    QString filename = "";
    TextEdit *textEdit = 0;
    bool unsaved = true;
    bool hasBeenSavedOnce = false;
    int tabIndex;
};

// Information about a keyframe
struct KeyFrameInfo {
    KeyFrameInfo() {}
    KeyFrameInfo ( QStringList settings ) : rawsettings ( settings )
    {
        name = settings.filter ( "keyframe", Qt::CaseInsensitive ).at ( 0 );
        index = settings.filter ( name, Qt::CaseInsensitive )
                .at ( 0 )
                .split ( "." )
                .at ( 1 )
                .toInt();
        QStringList cv = settings.filter ( "Eye ", Qt::CaseInsensitive )
                         .at ( 0 )
                         .split ( "=" )
                         .at ( 1 )
                         .split ( "," );
        eye = glm::dvec3 ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
        cv = settings.filter ( "Target", Qt::CaseInsensitive )
             .at ( 0 )
             .split ( "=" )
             .at ( 1 )
             .split ( "," );
        target = glm::dvec3 ( cv.at ( 0 ).toFloat(), cv.at ( 1 ).toFloat(), cv.at ( 2 ).toFloat() );
        cv = settings.filter ( "Up", Qt::CaseInsensitive )
             .at ( 0 )
             .split ( "=" )
             .at ( 1 )
             .split ( "," );
        up = glm::dvec3 ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
    }
    QString name = QString ( "" );
    QStringList rawsettings = QStringList ( "" );
    int index = 0;
    glm::dvec3 eye = glm::dvec3 ( 0,0,0 );
    glm::dvec3 target = glm::dvec3 ( 0,0,0 );
    glm::dvec3 up = glm::dvec3 ( 0,0,0 );
    int first = 0;
    int last = 0;
};

// Information about an easing curve
struct EasingInfo {
    EasingInfo() {}
    EasingInfo ( QString settings ) : rawsettings ( settings )
    {
        QStringList easingOptions = settings.split ( ":" );
        if ( easingOptions.count() == 12 ) {
            slidername = easingOptions.at ( 0 );
            typeName = easingOptions.at ( 1 );
            typeNum = easingOptions.at ( 2 ).toInt();
            startVal = easingOptions.at ( 3 ).toFloat();
            endVal = easingOptions.at ( 4 ).toFloat();
            firstFrame = easingOptions.at ( 5 ).toInt();
            lastFrame = easingOptions.at ( 6 ).toInt();
            period = easingOptions.at ( 7 ).toFloat();
            amplitude = easingOptions.at ( 8 ).toFloat();
            overshoot = easingOptions.at ( 9 ).toFloat();
            loops = easingOptions.at ( 10 ).toInt();
            pingpong = easingOptions.at ( 11 ).toInt();
        }
    }
    QString rawsettings = QString ( "" );
    QString slidername = QString ( "" );
    QString typeName = QString ( "" );
    int typeNum = 0;
    double startVal = 0;
    double endVal = 0;
    int firstFrame = 0;
    int lastFrame = 0;
    double period = 0;
    double amplitude = 0;
    double overshoot = 0;
    int loops = 0;
    int pingpong = 0;
};

/// The main window of the application.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    virtual ~MainWindow()
    {
        if ( !fragWatch->files().isEmpty() ) {
            fragWatch->removePaths ( fragWatch->files() );
        }
        delete fragWatch;
        fragWatch=0;
        if ( cmdScriptDebugger ) {
            cmdScriptDebugger->detach();
            cmdScriptDebugger->~QScriptEngineDebugger();
            cmdScriptDebugger = 0;
        }
        
    };

    double getTime();

    void setLastStoredTime ( float time )
    {
        lastStoredTime = time;
    }

    QVector<VariableWidget *> getUserUniforms()
    {
        return variableEditor->getUserUniforms();
    };

    DisplayWidget *getEngine()
    {
        return engine;
    }
    static QString getExamplesDir();
    void setFPS ( float fps );
    static QString getMiscDir();
    void saveImage ( QImage image );
    void resetCamera ( bool fullReset );
    QString getCameraSettings();
    QString getSettings( bool p = true )
    {
        return variableEditor->getSettings(p);
    };
    void disableAllExcept ( QWidget *w );
    void highlightBuildButton ( bool value );
    FileManager *getFileManager()
    {
        return &fileManager;
    }
    void setSubFrameDisplay ( int i );
    void setSubframeMax ( int i );
    void getBufferSize ( int w, int h, int &bufferSizeX, int &bufferSizeY, bool &fitWindow );
    int getSubFrameMax()
    {
        return frameSpinBox->value();
    }
    int getTimeSliderValue()
    {
        return ( timeSlider->value() );
    }
    int getFrameMax()
    {
        return timeSlider->maximum();
    }
    void setTimeSliderValue ( int value );

    TextEdit *getTextEdit();
    void createCommandHelpMenu ( QMenu *menu, QWidget *textEdit, MainWindow *mainWindow );
        VariableEditor *getVariableEditor()
    {
        return variableEditor;
    }
    double getTimeMax()
    {
        return timeMaxSpinBox->value();
    }
    void setCameraSettings ( glm::dvec3 e, glm::dvec3 t, glm::dvec3 u );
    int getCurrentCtrlPoint()
    {
        return variableEditor->getCurrentKeyFrame();
    }
    bool wantPaths()
    {
        return wantGLPaths;
    }

    bool autoFocusEnabled;
    int renderFPS;
    bool wantGLPaths;
    bool wantLineNumbers;
    int maxRecentFiles;

    void setRebuildStatus ( bool r )
    {
        rebuildRequired = r;
    };

    bool wantSplineOcc()
    {
        return wantSplineOcclusion;
    }
    int getTileWidth()
    {
        return bufferXSpinBox->value();
    };
    int getTileHeight()
    {
        return bufferYSpinBox->value();
    };
    bool wantSplineOcclusion;
    void runScript ( QString text )
    {
        scriptText = text;
        executeScript();
    };
    void setVerbose ( bool v )
    {
        verbose = v;
        getEngine()->setVerbose ( v );
        getVariableEditor()->setVerbose ( v );
    };
    bool isChangedUniformInBuffershaderOnly();
    void setLanguage( QString lang ) { langID = lang; };
    ListWidgetLogger *getLogger() { return logger; };

protected:
    void dragEnterEvent ( QDragEnterEvent *ev );
    void dropEvent ( QDropEvent *ev );
    void closeEvent ( QCloseEvent *ev );
    void keyReleaseEvent ( QKeyEvent *ev );

    void addToWatch ( QStringList fileList );

    int timeMax;

    // all public slots are available as script commands
public slots:
    void setDefault()
    {
        variableEditor->setDefault();
    };
    void processGuiEvents();
    void loadFragFile ( const QString &fileName );

    void setParameter ( QString settings )
    {
        variableEditor->setSettings ( settings );
    };
    void setParameter ( QString setting, bool b )
    {
        variableEditor->setSettings (QString ( "%1 = %2\n" ).arg ( setting ).arg ( b ? "true" : "false" ) );
    };
    void setParameter ( QString setting, int v )
    {
        variableEditor->setSettings ( QString ( "%1 = %2\n" ).arg ( setting ).arg ( v ) );
    };
    void setParameter ( QString setting, double v )
    {
        variableEditor->setSettings (QString ( "%1 = %2\n" ).arg ( setting ).arg ( v, 0, 'g', DDEC ) );
    };
    void setParameter ( QString setting, double x, double y )
    {
        variableEditor->setSettings ( QString ( "%1 = %2,%3\n" )
                                      .arg ( setting )
                                      .arg ( x, 0, 'g', DDEC )
                                      .arg ( y, 0, 'g', DDEC ) );
    };
    void setParameter ( QString setting, double x, double y, double z )
    {
        variableEditor->setSettings ( QString ( "%1 = %2,%3,%4\n" )
                                      .arg ( setting )
                                      .arg ( x, 0, 'g', DDEC )
                                      .arg ( y, 0, 'g', DDEC )
                                      .arg ( z, 0, 'g', DDEC ) );
    };
    void setParameter ( QString setting, double x, double y, double z, double w )
    {
        variableEditor->setSettings ( QString ( "%1 = %2,%3,%4,%5\n" )
                                      .arg ( setting )
                                      .arg ( x, 0, 'g', DDEC )
                                      .arg ( y, 0, 'g', DDEC )
                                      .arg ( z, 0, 'g', DDEC )
                                      .arg ( w, 0, 'g', DDEC ) );
    };

    // returns value as string, arrays with comma separator
    QString getParameter ( QString name )
    {
        QStringList s = getSettings().split ( "\n" );
        QString v = s.filter ( name ).at ( 0 ).split ( "=" ).at ( 1 ).trimmed();
        if ( v.isEmpty() ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return "";
        }
        return v;
    };

    float getParameter1f ( QString name )
    {
        bool ok=false;
        QStringList s = getSettings().split ( "\n" );
        float v = s.filter ( name ).at ( 0 ).split ( "=" ).at ( 1 ).trimmed().toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return 0.0;
        }
        return v;
    };

    glm::vec2 getParameter2f ( QString name )
    {
        bool ok=false;
        float x,y;
        QStringList v = getSettings().split ( "\n" );
        QStringList s = v.filter ( name ).at ( 0 ).split ( "=" ).at ( 1 ).trimmed().split(",");

        x = s.at(0).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec2(0.0,0.0);
        }

        y = s.at(1).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec2(x,0.0);
        }
        
        return glm::vec2(x,y);
    };

    glm::vec3 getParameter3f ( QString name )
    {
        bool ok=false;
        float x,y,z;
        QStringList v = getSettings().split ( "\n" );
        QStringList s = v.filter ( name ).at ( 0 ).split ( "=" ).at ( 1 ).trimmed().split(",");

        x = s.at(0).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec3(0.0,0.0,0.0);
        }

        y = s.at(1).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec3(x,0.0,0.0);
        }
        
        z = s.at(2).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec3(x,y,0.0);
        }
        
        return glm::vec3(x,y,z);
    };

    glm::vec4 getParameter4f ( QString name )
    {
        bool ok=false;
        float x,y,z,w;
        QStringList v = getSettings().split ( "\n" );
        QStringList s = v.filter ( name ).at ( 0 ).split ( "=" ).at ( 1 ).trimmed().split(",");

        x = s.at(0).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec4(0.0,0.0,0.0,0.0);
        }

        y = s.at(1).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec4(x,0.0,0.0,0.0);
        }
        
        z = s.at(2).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec4(x,y,0.0,0.0);
        }
        
        w = s.at(3).toFloat(&ok);
        if ( !ok ) {
            WARNING ( QString ( "Parameter %1 not found!" ).arg ( name ) );
            return glm::vec4(x,y,z,0.0);
        }
        
        return glm::vec4(x,y,z,w);
    };

    void setAutoRun ( bool arun )
    {
        QSettings settings;
        settings.setValue ( "autorun",arun );
        settings.sync();
    };

    void setAnimationLength ( int m )
    {
        timeMaxSpinBox->setValue ( m );
    };
    void setTileWidth ( int w )
    {
        QSettings settings;
        settings.setValue ( "tilewidth",w );
        settings.sync();
        bufferXSpinBox->setValue ( w );
    };
    void setTileHeight ( int h )
    {
        QSettings settings;
        settings.setValue ( "tileheight",h );
        settings.sync();
        bufferYSpinBox->setValue ( h );
    };
    void setTileMax ( int m )
    {
        QSettings settings;
        settings.setValue ( "tiles", m );
        settings.sync();
    };
    void setSubFrames ( int s )
    {
        QSettings settings;
        settings.setValue ( "subframes", s );
        settings.sync();
    };
    void setOutputBaseFileName ( QString f )
    {
        QSettings settings;
        settings.setValue ( "filename", f );
        settings.sync();
    };
    void setFps ( int fps )
    {
        QSettings settings;
        settings.setValue ( "fps", fps );
        settings.sync();
    };
    void setStartFrame ( int sf )
    {
        QSettings settings;
        settings.setValue ( "startframe", sf );
        settings.sync();
    };
    void setEndFrame ( int ef )
    {
        QSettings settings;
        settings.setValue ( "endframe", ef );
        settings.sync();
    };
    void setAnimation ( bool a )
    {
        QSettings settings;
        settings.setValue ( "animation", a );
        settings.sync();
    };
    void setPreview ( bool a )
    {
        QSettings settings;
        settings.setValue ( "preview", a );
        settings.sync();
    };
    void setAutoSave ( bool s )
    {
        QSettings settings;
        settings.setValue ( "autosave", s );
        settings.sync();
    }
    void setAutoLoad ( bool l )
    {
        QSettings settings;
        settings.setValue ( "autoload", l );
        settings.sync();
    }
    void setUniqueID ( bool u )
    {
        QSettings settings;
        settings.setValue ( "unique", u );
        settings.sync();
    }

    void tileBasedRender();
    void setFrame ( int value )
    {
        setStartFrame ( value );
        setEndFrame ( value+1 );
    };

    bool applyPresetByName ( QString n )
    {
        rebuildRequired = variableEditor->setPreset ( n );
        if(rebuildRequired) rebuildRequired = initializeFragment();
        processGuiEvents();
        return !rebuildRequired; // if rebuild is required applying the preset failed
    };

    bool scriptRunning()
    {
        return runningScript;
    };
    void stopScript()
    {
        runningScript=false;
    };

    QString currentFileName()
    {
        if(tabBar->currentIndex() == -1) return "";
        return tabInfo[tabBar->currentIndex()].filename;
    };
    QString currentFragmentName()
    {
        QStringList parts = tabInfo[tabBar->currentIndex()]
               .filename.split ( "/" )
               .last()
               .split ( "." );
        parts.removeLast();
        return parts.join ( "." );
    };
    void scriptExitProgram ( int x = 0 )
    {
        exit ( x );
    };

    void saveParameters ( QString fileName );
    void loadParameters ( QString fileName );
    void selectPreset();
    void addKeyFrame ( QStringList kfps );
    void clearKeyFrames();
    void initKeyFrameControl();

    int getFrame()
    {
        return getTimeSliderValue();
    }
    bool initializeFragment();

    void savePreview();

    void documentWasModified();
    void closeTab ( int index );
    void rewind();
    void play( bool restart = true );
    void stop();
    // for benchmark script
    int getTileAVG()
    {
        return engine->tileAVG;
    };
    int getRenderAVG()
    {
        return engine->renderAVG;
    };

    void dumpShaderAsm();

    void loadErrorSourceFile(QString fileName, int LineNumber);

    QString getVersion()
    {
        return version.toLongString();
    };

    QString getPresetNames ( bool keyframesORpresets = false );

    void readSettings();
    
    bool isPaused() { return pausePlay; };

    void testVersions() { getEngine()->testVersions(); };
private slots:
#ifdef USE_OPEN_EXR
    void initTools();
    void runTool();
#endif // USE_OPEN_EXR

    void veDockChanged ( bool t )
    {
        variableEditor->dockChanged ( t );
    }; // 05/22/17 Sabine ;)
    void clearKeyFrameControl();
    void bufferSpinBoxChanged ( int value );
    void timeChanged ( int value );
    void timeLineRequest ( QPoint p );
    void timeLineRequest ()
    {
        timeLineRequest ( QPoint(0,0) );
    }; // 11/22/19 ClaudeHA
    void videoEncoderRequest();
    void bufferActionChanged ( QAction *action );
    void showWelcomeNote();
    void maxSubSamplesChanged ( int value );
    void makeScreenshot();
    void showPreprocessedScript();
    void pasteSelected();
    void renderModeChanged();
    void saveParameters();
    void loadParameters();
    void indent();
    void preferences();
    void insertText();
    void variablesChanged ( bool lockedChanged );
    void cut();
    void copy();
    void paste();
    void search();
    void cursorPositionChanged();
    void tabChanged ( int index );
    void closeTab();
    void launchSfHome();
    void launchGallery();
    void launchGLSLSpecs();
    void launchFAQ();
    void launchIntro();
    void launchReferenceHome();
    void launchReferenceHome2();
    void launchDocumentation();
    void openFile();
    void newFile();
    void insertPreset();
    void open();
    bool save();
    bool saveAs();
    void about();
    void showControlHelp();
    void showScriptingHelp();
    void toggleFullScreen();
    void clearTextures();
    void testCompileGLSL();

    void setEasing()
    {
        variableEditor->setEasingCurve();
        tabInfo[tabBar->currentIndex()].unsaved = ( variableEditor->hasEasing() ) ? true : tabInfo[tabBar->currentIndex()].unsaved;
    }

    void timeMaxChanged ( int value );
    void editScript();

    void executeScript();
    void setupScriptEngine ( void );
    void saveCmdScript();
    void loadCmdScript();
    void reloadFrag();
    void reloadFragFile ( int );
    void reloadFragFile ( QString );
    QString makeImgFileName ( int timeStep, int timeSteps, QString fileName );
    void showHelpMessage ( QString title, QString mess );
    void hideUnusedVariableWidgets();
    // slot for F6 hotkey handlers
    void slotShortcutF6();
    void slotShortcutShiftF6();

#ifdef USE_OPEN_EXR
    bool writeTiledEXR(int maxTiles, int tileWidth, int tileHeight, int padding, int maxSubframes, int &steps, QString name, QProgressDialog &progress, QVector<QImage> &cachedTileImages, QTime &totalTime, double time);
#endif
    void renderTiled(int maxTiles, int tileWidth, int tileHeight, int padding, int maxSubframes, int &steps, QProgressDialog &progress, QVector<QImage> &cachedTileImages, QTime &totalTime, double time);

private:

    QString langID;
    QScriptEngineDebugger *cmdScriptDebugger;
    QSpinBox *timeMaxSpinBox;
    QPushButton *animationButton;
    QPushButton *progressiveButton;
    QPushButton *bufferSizeControl;
    int bufferSizeMultiplier;
    QList<QWidget *> disabledWidgets;
    QLabel *buildLabel;
    QLabel *timeLabel;
    void setRecentFile ( const QString &fileName );
    TextEdit *insertTabPage ( QString filename );
    void init();
    void buildExamplesMenu();
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void writeSettings();
    bool saveFile ( const QString &fileName );
    QString strippedName ( const QString &fullFileName );
    void createOpenGLContextMenu();
    QSpinBox *seedSpinBox;
    ListWidgetLogger *logger;

    bool loggingToFile;
    QString logFilePath;
    int maxLogFileSize;

    QDockWidget *dockLog;
    QAction *fullScreenAction;
    QAction *screenshotAction;
    QAction *sfHomeAction;
    QAction *glslHomeAction;
    QAction *introAction;
    QAction *faqAction;
    QAction *referenceAction;
    QAction *referenceAction2;
    QAction *referenceAction3;
    QAction *galleryAction;
    QAction *scriptingGeneralAction;
    QAction *scriptingParameterAction;
    QAction *scriptingHiresAction;
    QAction *scriptingControlAction;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *renderMenu;
    QMenu *parametersMenu;
    QMenu *examplesMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *renderToolBar;
    QToolBar *renderModeToolBar;
    QToolBar *timeToolBar;

    QSlider *timeSlider;
    QToolBar *editToolBar;
    QToolBar *bufferToolBar;
    QSpinBox *bufferXSpinBox;
    QSpinBox *bufferYSpinBox;

    QTime *lastTime;
    int lastStoredTime;
    QAction *newAction;
    QAction *openAction;
    QAction *reloadAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *closeAction;
    QAction *exitAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *findAction;
    QAction *asmAction;
    QAction *aboutAction;
    QAction *welcomeAction;
    QAction *controlAction;
    QAction *renderAction;
    QAction *videoEncoderAction;
    DisplayWidget *engine;
    QTabBar *tabBar;
    QSplitter *splitter;
    SyntopiaCore::Misc::Version version;
    QMenu *openGLContextMenu;
    bool fullScreenEnabled;
    QStackedWidget *stackedTextEdits;
    QVector<TabInfo> tabInfo;
    QVBoxLayout *frameMainWindow;
    VariableEditor *variableEditor;
    QDockWidget *editorDockWidget;
    QVector<QAction *> recentFileActions;
    QAction *recentFileSeparator;
    QLabel *fpsLabel;
    bool rebuildRequired;
    FileManager fileManager;
    QLabel *frameLabel;
    QSpinBox *frameSpinBox;
    QAction *rewindAction;
    QAction *playAction;
    QAction *stopAction;
    bool wantLoopPlay;
    bool pausePlay;

    QAction *bufferAction1;
    QAction *bufferAction1_2;
    QAction *bufferAction1_4;
    QAction *bufferAction1_6;
    QAction *bufferActionCustom;
    QAction *clearTexturesAction;
    QAction *testCompileGLSLAction;

    QString guiStylesheet;
    QString editorStylesheet;
    QLabel *subframeLabel;

    QScriptEngine scriptEngine;
    QScriptValue appContext;
    bool runningScript;
    QString scriptText;
    QAction *scriptAction;
    int cmdScriptLineNumber;

    int editorTheme;
    bool exrMode;
    bool verbose;
    bool fullPathInRecentFilesList;
    bool includeWithAutoSave;
    bool playRestartMode;
    bool useMimetypes;
    
#ifdef USE_OPEN_EXR
    QMenu *exrToolsMenu;
    QStringList exrBinaryPath;
#endif // USE_OPEN_EXR

    QMap<int, KeyFrameInfo *> keyframeMap;
    QMap<int, EasingInfo *> easingMap;

    QFileSystemWatcher *fragWatch;

    QShortcut       *keyF6;           // Entity of F6 hotkey
    QShortcut       *keyShiftF6;      // Entity of Shift+F6 hotkey
    
    QPixmap enginePixmap;
    QLabel* engineOverlay;


};
}
} // namespace Fragmentarium
