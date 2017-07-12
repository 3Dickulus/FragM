#pragma once

#include <QMainWindow>
#include <QTabBar>
#include <QStackedWidget>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QOpenGLShaderProgram>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractScrollArea>
#include <QTextBrowser>
#include <QtScript/QtScript>

#include "Highlighter.h"
#include "DisplayWidget.h"
#include "Version.h"
#include "VariableWidget.h"
#include "VariableEditor.h"

#ifdef NVIDIAGL4PLUS
#include "asmbrowser.h"
#endif

class QAction;
class QMenu;

namespace Fragmentarium {
  namespace GUI {
    
    #define DBOUT   qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__ 
    
    class TextEdit;
    
    // Information about the current tab
    struct TabInfo {
      TabInfo() {}
      TabInfo(QString filename, TextEdit* textEdit) : filename(filename), unsaved(false), textEdit(textEdit), hasBeenSavedOnce(false) {}
      TabInfo(QString filename, TextEdit* textEdit, bool unsaved, bool hasBeenSavedOnce=false) : filename(filename), unsaved(unsaved), textEdit(textEdit), hasBeenSavedOnce(hasBeenSavedOnce) {}
      QString filename;
      bool unsaved;
      TextEdit* textEdit;
      bool hasBeenSavedOnce;
    };
    
    // Information about a keyframe
    struct KeyFrameInfo {
      KeyFrameInfo() {}
      KeyFrameInfo(QString settings) : rawsettings(settings) {
        QStringList cs = settings.split ( "\n" );
        index = cs.filter ( "#preset keyframe", Qt::CaseInsensitive ).at ( 0 ).split ( "." ).at ( 1 ).toInt();
        fov = cs.filter ( "FOV", Qt::CaseInsensitive ).at ( 0 ).split ( "=" ).at ( 1 ).toFloat();
        QStringList cv = cs.filter ( "Eye ", Qt::CaseInsensitive ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
        eye = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
        cv = cs.filter ( "Target", Qt::CaseInsensitive ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
        target = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
        cv = cs.filter ( "Up", Qt::CaseInsensitive ).at ( 0 ).split ( "=" ).at ( 1 ).split ( "," );
        up = QVector3D ( cv.at ( 0 ).toFloat(),cv.at ( 1 ).toFloat(),cv.at ( 2 ).toFloat() );
      }
      QString rawsettings;
      QString index;
      double fov;
      QVector3D eye;
      QVector3D target;
      QVector3D up;
    };
    
    struct EasingInfo {
      EasingInfo() {}
      EasingInfo(QString settings) : rawsettings(settings){
        QStringList easingOptions = settings.split(":");
        slidername = easingOptions.at(0);
        typeName = easingOptions.at(1);
        typeNum = easingOptions.at(2).toInt();
        startVal = easingOptions.at(3).toFloat();
        endVal = easingOptions.at(4).toFloat();
        firstFrame = easingOptions.at(5).toInt();
        lastFrame = easingOptions.at(6).toInt();
        period = easingOptions.at(7).toFloat();
        amplitude = easingOptions.at(8).toFloat();
        overshoot = easingOptions.at(9).toFloat();
        loops = easingOptions.at(10).toInt();
        pingpong = easingOptions.at(11).toInt();
      }
      QString rawsettings;
      QString slidername;
      QString typeName;
      int typeNum;
      double startVal;
      double endVal;
      int firstFrame;
      int lastFrame;
      double period;
      double amplitude;
      double overshoot;
      int loops;
      int pingpong;
    };
    
    /// The main window of the application.
    class MainWindow : public QMainWindow
    {
      Q_OBJECT
      
    public:
      MainWindow(QSplashScreen* splashWidget);
      virtual ~MainWindow() {};
      double getTime();
      void setLastStoredTime(float time) {
        lastStoredTime = time;
      }
/// BEGIN 3DTexture
// #ifdef USE_OPEN_EXR
//       void setVoxelFile( QString vfn ){ engine->set3DTextureFileName( vfn ); };
//       void setObjFile( QString ofn );
// #endif // USE_OPEN_EXR
/// END 3DTexture
      void setUserUniforms(QOpenGLShaderProgram* shaderProgram);
      QVector<VariableWidget*> getUserUniforms(){return variableEditor->getUserUniforms();};
      void setFeedbackUniforms(QOpenGLShaderProgram* shaderProgram);
      DisplayWidget* getEngine() {
        return engine;
      }
      static QString getExamplesDir();
      void setFPS(float fps);
      static QString getMiscDir();
      void saveImage(QImage im);
      void resetCamera(bool fullReset);
      QString getCameraSettings();
      QString getSettings() {
        return variableEditor->getSettings();
      };
      void disableAllExcept(QWidget* w);
      void setSplashWidgetTimeout(QSplashScreen* w);
      void highlightBuildButton(bool value);
      FileManager* getFileManager() {
        return &fileManager;
      }
      void setSubFrameDisplay(int i);
      void setSubframeMax(int i);
      void getBufferSize(int w, int h, int& bufferSizeX, int& bufferSizeY, bool& fitWindow);
      int getSubFrameMax() {
        return frameSpinBox->value();
      }
      int getTimeSliderValue() {
        return (timeSlider->value());
      }
      void setTimeSliderValue(int value);
      
      TextEdit* getTextEdit();
      void createCommandHelpMenu(QMenu* menu, QWidget* textEdit, MainWindow* mainWindow);
      VariableEditor* getVariableEditor() {
        return variableEditor;
      }
      double getTimeMax() {
        return timeMaxSpinBox->value();
      }
      void setCameraSettings(QVector3D e, QVector3D t, QVector3D u);
      int getCurrentCtrlPoint() {
        return variableEditor->getCurrentKeyFrame();
      }
      bool wantPaths() {
        return wantGLPaths;
      }
      QStringList keyFrameList;
      bool autoFocusEnabled;
      int renderFPS;
      bool wantGLPaths;
      bool wantLineNumbers;
      int MaxRecentFiles;
      
      void needRebuild(bool r) {
        rebuildRequired = r;
      };
      bool requiresRebuild() {
        return rebuildRequired;
      };
      void setDefault() {
        variableEditor->setDefault();
      };
      bool wantSplineOcc() {
        return wantSplineOcclusion;
      }
      int getTileWidth() {
        return bufferXSpinBox->value();
      };
      int getTileHeight() {
        return bufferYSpinBox->value();
      };
      void processGuiEvents();
      bool wantSplineOcclusion;
      void runScript(QString text) {
        scriptText = text;
        executeScript();
      };
      
      // M Benesi "Spray gun" public
      bool fragHasFeedbackVars;
      void setFeebackCoords( QVector3D crds ) { feedbackcrds[feedbackindex] = crds; };
      QVector3D getFeebackCoords() { return feedbackcrds[feedbackindex]; };
      QVector3D getFeebackCoord() { QVector3D ret = feedbackcrds[feedbackindex]; return QVector3D(ret.x(),ret.y(),ret.z()); };
      int getFeedbackIndex(){ return feedbackindex;};
      int getFeedbackCount(){ return feedbackcount;};
      int getFeedbackMaxIndex(){ return feedbackmaxindex;};
      QString langID;
      
    protected:
      void dragEnterEvent(QDragEnterEvent *ev);
      void dropEvent(QDropEvent *ev);
      void closeEvent(QCloseEvent* ev);
      void keyReleaseEvent(QKeyEvent* ev);
      int timeMax;
      
    public slots:
      void loadFragFile(const QString &fileName);
      void setParameter(QString settings) {
        variableEditor->setSettings(settings);
      };
      void setParameter(QString setting, bool b) {
        variableEditor->setSettings( QString("%1 = %2\n").arg(setting).arg(b?"true":"false") );
      };
      void setParameter(QString setting, int v) {
        variableEditor->setSettings( QString("%1 = %2\n").arg(setting).arg(v) );
      };
      void setParameter(QString setting, double v) {
        variableEditor->setSettings( QString("%1 = %2\n").arg(setting).arg(v) );
      };
      void setParameter(QString setting, double x, double y) {
        variableEditor->setSettings( QString("%1 = %2,%3\n").arg(setting).arg(x).arg(y) );
      };
      void setParameter(QString setting, double x, double y, double z) {
        variableEditor->setSettings( QString("%1 = %2,%3,%4\n").arg(setting).arg(x).arg(y).arg(z) );
      };
      void setParameter(QString setting, double x, double y, double z, double w) {
        variableEditor->setSettings( QString("%1 = %2,%3,%4,%5\n").arg(setting).arg(x).arg(y).arg(z).arg(w) );
      };
      QString getParameter(QString name) {
        QStringList s = getSettings().split("\n");
        return s.filter(name).at(0).split("=").at(1);
      };
      void setAnimationLength(int m) {
        timeMaxSpinBox->setValue(m);
      };
      void setTileWidth(int w) {
        QSettings settings;
        settings.setValue("tilewidth",w);
        settings.sync();
        bufferXSpinBox->setValue(w);
      };
      void setTileHeight(int h) {
        QSettings settings;
        settings.setValue("tileheight",h);
        settings.sync();
        bufferYSpinBox->setValue(h);
      };
      void setTileMax(int m) {
        QSettings settings;
        settings.setValue("tiles", m);
        settings.sync();
      };
      void setSubFrames(int s) {
        QSettings settings;
        settings.setValue("subframes", s);
        settings.sync();
      };
      void setOutputBaseFileName(QString f) {
        QSettings settings;
        settings.setValue("filename", f);
        settings.sync();
      };
      void setFps(int fps) {
        QSettings settings;
        settings.setValue("fps", fps);
        settings.sync();
      };
      void setStartFrame(int sf) {
        QSettings settings;
        settings.setValue("startframe", sf);
        settings.sync();
      };
      void setEndFrame(int ef) {
        QSettings settings;
        settings.setValue("endframe", ef);
        settings.sync();
      };
      void setAnimation(bool a) {
        QSettings settings;
        settings.setValue("animation", a);
        settings.sync();
      };
      void setPreview(bool a) {
        QSettings settings;
        settings.setValue("preview", a);
        settings.sync();
      };
      void tileBasedRender();
      void setFrame(int value) {
        setStartFrame(value);
        setEndFrame(value+1);
      };
      
      void ByName(QString n) {
        variableEditor->setPreset(n);
      };
      
      bool scriptRunning() {
        return runningScript;
      };
      void stopScript() {
        runningScript=false;
      };
      
      void saveParameters(QString fileName);
      void loadParameters(QString fileName);
      void selectPreset();
      void addKeyFrame(QString name);
      void clearKeyFrames();
      void initKeyFrameControl();
      
      int getFrame() {
        return getTimeSliderValue();
      }
      bool initializeFragment();
      void callRedraw();
      
      void savePreview();
      
      void documentWasModified();
      void closeTab(int id);
      void rewind();
      void play();
      void stop();
      #ifdef NVIDIAGL4PLUS
      void dumpShaderAsm();
      #endif // NVIDIAGL4PLUS
      void setFeedbackId( int fbi ){ zapIndex->setValue(fbi); };
      
    private slots:
      void veDockChanged(bool t){variableEditor->dockChanged( t );}; // 05/22/17 Sabine ;)
      void clearKeyFrameControl();
      void bufferSpinBoxChanged(int);
      void timeChanged(int);
      void timeLineRequest(QPoint p);
      void videoEncoderRequest();
      void bufferActionChanged(QAction* action);
      void showWelcomeNote();
      void removeSplash();
      void maxSubSamplesChanged(int);
      void makeScreenshot();
      void showDebug();
      void pasteSelected();
      void renderModeChanged();
      void saveParameters();
      void loadParameters();
      void indent();
      void preferences();
      void insertText();
      void variablesChanged(bool lockedChanged);
      void cut();
      void copy();
      void paste();
      void search();
      void cursorPositionChanged();
      void tabChanged(int index);
      void closeTab();
      void launchSfHome();
      void launchGallery();
      void launchGLSLSpecs();
      void launchFAQ();
      void launchIntro();
      void launchReferenceHome();
      void launchReferenceHome2();
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
      
      void setEasing() {
        variableEditor->setEasingCurve();
        tabInfo[tabBar->currentIndex()].unsaved = (variableEditor->hasEasing()) ? true : tabInfo[tabBar->currentIndex()].unsaved;
      }
      
      void timeMaxChanged(int v);
      void editScript();
      
      void executeScript();
      void setupScriptEngine(void);
      void saveCmdScript();
      void loadCmdScript();
      
      QString makeImgFileName(int timeStep, int timeSteps, QString fileName);

      // M Benesi "Spray gun" private
      QVector4D grabFeedbackSettings();
      QVector4D grabFeedbackSettings2();
      QVector4D grabFeedbackRotation();
      void setFeedbackArrayData();
      void setFeedbackParameters();
      void saveFeedback();
      void loadFeedback();
      void setZapLock( bool lok ) { engine->setZapLock(lok); };
      void setZapClear() { engine->wantZappaClear = true;  engine->requireRedraw(true); };
      void setFeedbackIndex( int fbi ){
        if(fbi<feedbackmaxindex && fbi > 0) {
          feedbackindex = fbi;
          statusBar()->showMessage(QString("Zappa:%1, %2 : %3,%4,%5").
          arg(fbi).arg(feedbackcount).
          arg(feedbackcrds[fbi].x()).arg(feedbackcrds[fbi].y()).arg(feedbackcrds[fbi].z()), 0);
          
          setFeedbackParameters();
        }
      };
      
    private:
      QSpinBox* timeMaxSpinBox;
      QPushButton* animationButton;
      QPushButton* progressiveButton;
      QPushButton* bufferSizeControl;
      int bufferSizeMultiplier;
      QList<QWidget *> disabledWidgets;
      QLabel* buildLabel;
      QLabel* timeLabel;
      void setRecentFile(const QString &fileName);
      TextEdit* insertTabPage(QString filename);
      void init();
      void createActions();
      void createMenus();
      void createToolBars();
      void createStatusBar();
      void readSettings();
      void writeSettings();
      bool saveFile(const QString &fileName);
      QString strippedName(const QString &fullFileName);
      void createOpenGLContextMenu();
      bool hasBeenResized;
      QSpinBox* seedSpinBox;
      ListWidgetLogger* logger;
      QDockWidget* dockLog;
      QAction* fullScreenAction;
      QAction* screenshotAction;
      QAction* sfHomeAction;
      QAction* glslHomeAction;
      QAction* introAction;
      QAction* faqAction;
      QAction* referenceAction;
      QAction* referenceAction2;
      QAction* galleryAction;
      QAction* scriptingGeneralAction;
      QAction* scriptingParameterAction;
      QAction* scriptingHiresAction;
      QAction* scriptingControlAction;
      QMenu *fileMenu;
      QMenu *editMenu;
      QMenu *renderMenu;
      QMenu* parametersMenu;
      QMenu *helpMenu;
      QToolBar *fileToolBar;
      QToolBar *renderToolBar;
      QToolBar *renderModeToolBar;
      QToolBar *timeToolBar;
      
      QSlider* timeSlider;
      QToolBar *editToolBar;
      QToolBar *bufferToolBar;
      QSpinBox *bufferXSpinBox;
      QSpinBox *bufferYSpinBox;
      
      QTime *lastTime;
      int lastStoredTime;
      QAction *newAction;
      QAction *openAction;
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
      DisplayWidget* engine;
      QTabBar *tabBar;
      QSplitter *splitter;
      SyntopiaCore::Misc::Version version;
      QMenu *openGLContextMenu;
      bool fullScreenEnabled;
      QStackedWidget *stackedTextEdits;
      QVector<TabInfo> tabInfo;
      int oldDirtyPosition;
      QVBoxLayout* frameMainWindow;
      VariableEditor* variableEditor;
      QDockWidget* editorDockWidget;
      QVector<QAction*> recentFileActions;
      QAction* recentFileSeparator;
      QLabel* fpsLabel;
      QSplashScreen* splashWidget;
      bool rebuildRequired;
      FileManager fileManager;
      QLabel* frameLabel;
      QSpinBox* frameSpinBox;
      QAction* rewindAction;
      QAction* playAction;
      QAction* stopAction;
      bool wantLoopPlay;
      bool pausePlay;
      
      QAction* bufferAction1;
      QAction* bufferAction1_2;
      QAction* bufferAction1_4;
      QAction* bufferAction1_6;
      QAction* bufferActionCustom;
      QAction* clearTexturesAction;
      
      QString editorStylesheet;
      QLabel* subframeLabel;
      
      QScriptEngine scriptEngine;
      QScriptValue appContext;
      bool runningScript;
      QString scriptText;
      QAction* scriptAction;
      int cmdScriptLineNumber;
      
      bool exrMode;
/// BEGIN 3DTexture
//       QString voxelFileName;
/// END 3DTexture
      
      // M Benesi "Spray gun" private
      void enableZappaTools( bool doZap );
      QToolBar *zappaToolBar;
      QAction* zapLock;
      QCheckBox* zapCheck;
      QSpinBox* zapIndex;
      QAction* zapIndx;
      QPushButton* zapClear;
      QAction* zapClr;
      
      int feedbackindex;
      int feedbackcount;
      int feedbackmaxindex;
      QVector3D *feedbackcrds;
      QVector4D *feedcontrol1;
      QVector4D *feedcontrol2;
      QVector4D *feedrotation;
      QAction* loadfdbkAction;
      QAction* savefdbkAction;
      
    };
    
  }
}
