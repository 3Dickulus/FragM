#include <thread>
#include <iostream>
#include <QShortcut>
#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QImage>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPixmap>
#include <QSpacerItem>
#include <QStack>
#include <QString>
#include <QTabWidget>
#include <QTextBlockUserData>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <QtGui>
#include <QtNetwork/QtNetwork>

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QtScriptTools/QScriptEngineDebugger>

#include <glm/glm.hpp>

#include "MainWindow.h"
#include "TextEdit.h"
#include "TimeLine.h"
#include "VideoDialog.h"

#include "Exception.h"
#include "ListWidgetLogger.h"
#include "Misc.h"
#include "OutputDialog.h"
#include "PreferencesDialog.h"
#include "Preprocessor.h"
#include "VariableEditor.h"

#include "qtgradienteditor/qtgradientdialog.h"

#include "ggr2glsl.h"

#define DBOUT qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__

namespace Fragmentarium
{
namespace GUI
{

MainWindow::MainWindow(QWidget* parent)
    : cmdScriptDebugger(nullptr)
{
    pausePlay = true;
    bufferXSpinBox = nullptr;
    bufferYSpinBox = nullptr;
    lastStoredTime = 0;
    exrMode = false;
    exrToolsMenu = nullptr;
    supportProgramsMenu = nullptr;
    maxRecentFiles = 5;
    editorTheme = 0;
    engine = nullptr;

    setAcceptDrops(true);

    setRebuildStatus(true);

    setFocusPolicy(Qt::WheelFocus);

    // make today's build number
    version = Version(FRAGM_MAJOR_VERSION, FRAGM_MINOR_VERSION, FRAGM_REVISION, PACKAGE_BUILD, "Digilantism");
    setAttribute(Qt::WA_DeleteOnClose);

    fullScreenEnabled = false;

    QDir::setCurrent(QCoreApplication::applicationDirPath()); // Otherwise we cannot find examples + templates
    fragWatch = new QFileSystemWatcher();
    connect(fragWatch, SIGNAL(fileChanged(QString)), this, SLOT(reloadFragFile(QString)));

    logFilePath = "fragm.log";
    loggingToFile = false;
    maxLogFileSize = 125000;
    fullPathInRecentFilesList = false;
    includeWithAutoSave = true;

    lockedAspect = false;

    init();

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    splitter->restoreState(settings.value("splitterSizes").toByteArray());
    fullScreenEnabled = settings.value("fullScreenEnabled", false).toBool();
    if (fullScreenEnabled) {
        fullScreenEnabled = false;
        toggleFullScreen();
    }

    keyF6 = new QShortcut(this);   // Initialize the object
    keyF6->setKey(Qt::Key_F6);    // Set the key code
    // connect handler to keypress
    connect(keyF6, SIGNAL(activated()), this, SLOT(slotShortcutF6()));
    keyShiftF6 = new QShortcut(this);   // Initialize the object
    keyShiftF6->setKey( QKeySequence("Shift+F6"));    // Set the key code
    // connect handler to keypress
    connect(keyShiftF6, SIGNAL(activated()), this, SLOT(slotShortcutShiftF6()));

    connect(splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(movedSplitter(int, int)));
}

void MainWindow::createCommandHelpMenu(QMenu *menu, QWidget *textEdit,
                                       MainWindow *mainWindow)
{

    QMenu *preprocessorMenu = new QMenu(tr("Host Preprocessor Commands"), nullptr);
    preprocessorMenu->addAction("#info sometext", textEdit , SLOT(insertText()));
    preprocessorMenu->addAction("#include \"some.frag\"", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#camera 2D", textEdit , SLOT(insertText()));
    preprocessorMenu->addAction("#camera 3D", textEdit , SLOT(insertText()));
    preprocessorMenu->addAction("#group parameter_group_name", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#preset preset_name", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#endpreset", textEdit , SLOT(insertText()));
    preprocessorMenu->addAction("#define DontClearOnChange", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#define IterationsBetweenRedraws 10", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#define SubframeMax 20", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#define providesColor", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#define providesBackground", textEdit, SLOT(insertText()));
    preprocessorMenu->addAction("#define providesNormal", textEdit, SLOT(insertText()));

    QMenu *textureFlagsMenu = new QMenu(tr("2D Texture Options"), nullptr);
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MAG_FILTER GL_NEAREST", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_WRAP_S GL_CLAMP", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_WRAP_T GL_CLAMP", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MAX_LEVEL 1000", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_WRAP_S GL_REPEAT", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_WRAP_T GL_REPEAT", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MAG_FILTER GL_LINEAR", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MIN_FILTER GL_LINEAR_MIPMAP_LINEAR", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MIN_FILTER GL_LINEAR_MIPMAP_NEAREST", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MIN_FILTER GL_NEAREST_MIPMAP_LINEAR", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MIN_FILTER GL_NEAREST_MIPMAP_NEAREST", textEdit, SLOT(insertText()));
    textureFlagsMenu->addAction("#TexParameter textureName GL_TEXTURE_MAX_ANISOTROPY float(>1.0 <16.0)", textEdit, SLOT(insertText()));

    QMenu *uniformMenu = new QMenu(tr("Special Uniforms"), nullptr);
    uniformMenu->addAction("uniform float time;", textEdit , SLOT(insertText()));
    uniformMenu->addAction("uniform int subframe;", textEdit , SLOT(insertText()));
    uniformMenu->addAction("uniform int i; slider[0,1,2]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform float f; slider[0,1,2]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform vec2 pixelSize;", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform vec2 v; slider[(0,0),(1,1),(1,1)]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform vec3 v; slider[(0,0,0),(1,1,1),(1,1,1)]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform vec4 v; slider[(0,0,0,0),(1,1,1,1),(1,1,1,1)]", textEdit, SLOT(insertText()));

    if ((engine->format().majorVersion() > 3 && engine->format().minorVersion() >= 0)) {
        uniformMenu->addAction("uniform double f; slider[0,1,2]", textEdit, SLOT(insertText()));
        uniformMenu->addAction("uniform dvec2 v; slider[(0,0),(1,1),(1,1)]", textEdit, SLOT(insertText()));
        uniformMenu->addAction("uniform dvec3 v; slider[(0,0,0),(1,1,1),(1,1,1)]", textEdit, SLOT(insertText()));
        uniformMenu->addAction("uniform dvec4 v; slider[(0,0,0,0),(1,1,1,1),(1,1,1,1)]", textEdit, SLOT(insertText()));
    }

    uniformMenu->addAction("uniform bool b; checkbox[true]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform sampler2D tex; file[tex.jpg]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform samplerCube cubetex; file[cubetex.jpg]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform vec3 color; color[0.0,0.0,0.0]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform vec4 color; color[0.0,1.0,0.0,0.0,0.0,0.0]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform bool DepthToAlpha; checkbox[true]", textEdit, SLOT(insertText()));
    uniformMenu->addAction("uniform bool AutoFocus; checkbox[true]", textEdit, SLOT(insertText()));

    uniformMenu->insertMenu(nullptr, textureFlagsMenu);

    QMenu *includeMenu = new QMenu(tr("Include (from Preferences Paths)"), nullptr);

    QStringList filter;
    filter << "*.frag";
    QStringList files = getFileManager()->getFiles(filter);
    foreach (QString s, files) {
        includeMenu->addAction(QString("#include \"%1\"").arg(s), mainWindow, SLOT(insertText()));
    }

    QAction *before = nullptr;
    if (menu->actions().count() > 0) {
        before = menu->actions()[0];
    }
    menu->insertMenu(before, preprocessorMenu);
    menu->insertMenu(before, uniformMenu);
    menu->insertMenu(before, includeMenu);

    menu->insertSeparator(before);
    menu->addAction(tr("Insert Preset from Current Settings"), mainWindow, SLOT(insertPreset()));
}

void MainWindow::closeEvent(QCloseEvent *ev)
{

    bool modification = false;
    for (auto &i : tabInfo) {
        if (i.unsaved) {
            modification = true;
        }
    }

    if (modification) {
        QString mess = tr("There are unsaved changes.\r\n%1\r\nContinue will discard changes.")
            .arg(variableEditor->hasEasing() ? tr("\r\nTip: Update easing curves in preset\r\nand save to file before closing.\r\n") : "\r\n");

        int i = QMessageBox::warning(this, tr("Unsaved changes"), mess, tr("Continue"), tr("Cancel"));
        if (i == 1) {
            // Cancel
            ev->ignore();
            return;
        }

        // OK
        ev->accept();
        return;
    }

    writeSettings();

    if (keyframeMap.count() > 0) {
        keyframeMap.clear();
    }
    if (easingMap.count() > 0) {
        easingMap.clear();
    }

    ev->accept();

}

void MainWindow::new2DFile()
{
    QString s(tr("// New 2D fragment...") + "\r\n\
#include \"MathUtils.frag\" \r\n\
#include \"Complex.frag\" \r\n\
#include \"Progressive2D.frag\" \r\n\
 \r\n\
// Escape time fractals iterate a function for each point \r\n\
// in the plane, and checks if the sequence generated converges. \r\n\
//  \r\n\
// Just implement the \'color\' function below. \r\n\
// It is possible to draw Mandelbrots and Julias \r\n\
// and customize the coloring. \r\n\
// \r\n\
// Here is an example of a Mandelbrot: \r\n\
 \r\n\
// The name for the tab for our set of control sliders \r\n\
#group Mandelbrot \r\n\
 \r\n\
// Number of iterations \r\n\
uniform int  Iterations; slider[10,200,1000] \r\n\
 \r\n\
uniform float R; slider[0,0,1] \r\n\
uniform float G; slider[0,0.4,1] \r\n\
uniform float B; slider[0,0.7,1] \r\n\
 \r\n\
uniform float Power; slider[1,2,10] \r\n\
uniform float Bailout; slider[0,64,384] \r\n\
 \r\n\
// The color scheme here is based on one from Inigo Quilez\'s Shader Toy: \r\n\
// http://www.iquilezles.org/www/articles/mset_smooth/mset_smooth.htm \r\n\
vec3 IQColor(float i, vec2 z) { \r\n\
      float co;  // equivalent optimized smooth interation count \r\n\
      co = 3.1415+i - log(log2(length(z)))/log(Power); \r\n\
      co = 6.2831*sqrt(co); \r\n\
      return .5+.5*cos(co+vec3(R,G,B) ); \r\n\
} \r\n\
 \r\n\
vec3 color(vec2 c) { \r\n\
 \r\n\
	vec2 z = vec2(0.0,0.0); \r\n\
 \r\n\
	int i; \r\n\
 \r\n\
	for (i = 0; i < Iterations; i++) { \r\n\
		// calculate Z \r\n\
		z = cPow(z, Power) + c; \r\n\
		// check bailout \r\n\
		if (dot(z,z) > Bailout) break; \r\n\
	} \r\n\
 \r\n\
	vec3 rColor=vec3(0.0); \r\n\
 \r\n\
	if(i<Iterations) \r\n\
		rColor = IQColor(float(i),z); \r\n\
 \r\n\
	return rColor; \r\n\
} \r\n\
 \r\n\
#preset Default \r\n\
Center = -0.5,0.0 \r\n\
Zoom = 0.75 \r\n\
#endpreset \r\n" );

    insertTabPage(s);
    variableEditor->resetUniforms();
    applyPresetByName("Default");
}

void MainWindow::new3DFile()
{
    QString s(tr("// New 3D fragment...") + "\r\n\
#include \"MathUtils.frag\"\r\n\
#include \"DE-Raytracer.frag\" \r\n\
\r\n\
float DE(vec3 pos) {\r\n\
	return abs(length(abs(pos)+vec3(-1.0))-1.2);\r\n\
}\r\n\
\r\n\
// minimum number of settings for a 3D preset\r\n\
\r\n\
#preset Default\r\n\
FOV = 0.4\r\n\
Eye = 5.582327,4.881191,-6.709066\r\n\
Target = -0.44036019,-0.145859154,0.288424741\r\n\
Up = -0.115126834,0.845289908,0.508173856\r\n\
#endpreset\r\n");

    insertTabPage(s);
    variableEditor->resetUniforms();
    applyPresetByName("Default");
}

void MainWindow::insertPreset()
{
    bool ok = false;
    QString newPreset;
    QTextCursor tc = getTextEdit()->textCursor();

    if(tc.hasSelection() &&
            tc.selectedText().contains("#preset", Qt::CaseInsensitive) &&
            tc.selectedText().contains("#endpreset", Qt::CaseInsensitive)) { /// if we have selected text try to extract name
        newPreset = tc.selection().toPlainText();
        QStringList tmp = newPreset.split('\n');
        newPreset=tmp.at( tmp.indexOf( QRegExp("^#[Pp]reset.*$") ) );
        tmp = newPreset.split(" ");
        newPreset=tmp.at(1);
    } else { /// no block marked or not a preset so move to the end of script to add a new one
        tc.movePosition(QTextCursor::End);
        getTextEdit()->setTextCursor(tc);
        if (engine->cameraID() == "3D") {
            newPreset = QStringLiteral("KeyFrame.%1").arg(variableEditor->getKeyFrameCount() + 1, 4, 10, QLatin1Char('0'));
        }
    }

    /// confirm keyframe name
    QString newPresetName = QInputDialog::getText(this, tr("Add Preset"),
                            tr("Change the name for Preset, KeyFrame or Range"),
                            QLineEdit::Normal, newPreset.toLatin1(), &ok);
    setRebuildStatus(ok);

    if (ok && !newPresetName.isEmpty()) {

        if(newPresetName.contains("KeyFrame.", Qt::CaseInsensitive)) { /// adding keyframe
            getTextEdit()->insertPlainText("\n#preset " + newPresetName + "\n" + getCameraSettings() + "\n#endpreset\n");
        } else if (newPresetName.contains("Range", Qt::CaseInsensitive)) { /// adding parameter easing range
            if(variableEditor->hasEasing()) {
                getTextEdit()->insertPlainText("\n#preset " + newPresetName + "\n" + getEngine()->getCurveSettings().join("\n") + "\n#endpreset\n");
            } else {
                QMessageBox::warning(this, tr("Warning!"), tr("Setup some parameter Easing Curves first!"));
                INFO(tr("%1 Failed!").arg(newPresetName));
                return;
            }
        } else { /// adding a named preset in program so no provenance lines
            getTextEdit()->insertPlainText("\n#preset " + newPresetName + "\n" + getSettings(false) + "\n#endpreset\n");
        }

        if (rebuildRequired)
            setRebuildStatus(initializeFragment()); // once to initialize any textures

        rebuildRequired = variableEditor->setPreset(newPresetName); // apply the settings

        INFO(tr("Added %1").arg(newPresetName));
    }
}

void MainWindow::open()
{

    QString filter = tr("Fragment Source (*.frag);;All Files (*.*)");
    QString fileName =
        QFileDialog::getOpenFileName(this, QString(), QString(), filter);
    if (!fileName.isEmpty()) {
        loadFragFile(fileName);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *ev)
{

    if (ev->key() == Qt::Key_Escape) {
        toggleFullScreen();
    } else if (ev->key() == Qt::Key_Space) {
        if(engine->hasFocus()) {
            pausePlay = !pausePlay;
            if (pausePlay && (engine->getState() == DisplayWidget::Animation)) {
                stop();
            } else {
                play();
            }
        }
    } else {
        ev->ignore();
    }

}

void MainWindow::clearTextures()
{

    engine->clearTextureCache(nullptr);
}

void MainWindow::testCompileGLSL()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  engine->testVersions();
  QApplication::restoreOverrideCursor();

}

void MainWindow::setCameraPathLoop(bool l) {
    highlightBuildButton(l != loopCameraPath);
    loopCameraPath = l;
    engine->setCameraPathLoop(loopCameraPath);
}

void MainWindow::movedSplitter(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)
    bufferActionChanged(lockedToWindowSize ? bufferAction1 : bufferActionCustom);
}

// finished editing ie:pressed return or clicked on something else
void MainWindow::bufferSizeXChanged()
{
    if(engine->getState() != DisplayWidget::Tiled && !lockedToWindowSize) {
        bufferActionChanged(bufferActionCustom);
    }
}

// finished editing ie:pressed return or clicked on something else
void MainWindow::bufferSizeYChanged()
{
    if(engine->getState() != DisplayWidget::Tiled && !lockedToWindowSize) {
        bufferActionChanged(bufferActionCustom);
    }
}
#define MIN(a,b)(a<b?a:b)
#define MAX(a,b)(a>b?a:b)
// value changed
void MainWindow::bufferXSpinBoxChanged(int value)
{
    if(engine->getState() != DisplayWidget::Tiled && !lockedToWindowSize) {
        if(lockedAspect) {
            bufferYSpinBox->blockSignals(true);
            if(currentAspect > 1)
                bufferYSpinBox->setValue( MIN(value, round(value / currentAspect)));
            else bufferYSpinBox->setValue( MAX(value, round(value / currentAspect)));
            bufferYSpinBox->blockSignals(false);
        }
    }
}

void MainWindow::bufferYSpinBoxChanged(int value)
{
    if(engine->getState() != DisplayWidget::Tiled && !lockedToWindowSize) {
        if(lockedAspect) {
            bufferXSpinBox->blockSignals(true);
            if(currentAspect > 1)
                bufferXSpinBox->setValue(MAX(value, round(value * currentAspect)));
            else bufferXSpinBox->setValue(MIN(value, round(value * currentAspect)));
            bufferXSpinBox->blockSignals(false);
        }
    }
}

bool MainWindow::save()
{

    int index = tabBar->currentIndex();
    if (index == -1) {
        WARNING(tr("No open tab"));
        return false;
    }
    TabInfo t = tabInfo[index];

    if (t.hasBeenSavedOnce) {
        return saveFile(t.filename);
    }
    return saveAs();
}

bool MainWindow::saveAs()
{

    int index = tabBar->currentIndex();
    if (index == -1) {
        WARNING(tr("No open tab"));
        return false;
    }

    TabInfo t = tabInfo[index];

    QString filter = tr("Fragment Source (*.frag);;All Files (*.*)");

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), t.filename, filter);
    if (fileName.isEmpty()) {
        return false;
    }

    if (saveFile(fileName)) {
        t.filename = fileName;
        tabBar->setTabToolTip(index,fileName);
        return true;
    }
    return false;
}

void MainWindow::showHelpMessage(const QString title, const QString mess)
{

    QMessageBox mb(this);
    mb.setText(mess);
    mb.setWindowTitle(title);
    mb.setIconPixmap(getMiscDir() + "/" + "Fragmentarium-sm.png");
    QGridLayout *glayout = (QGridLayout *)mb.layout();
    glayout->setColumnMinimumWidth( 2, 640);
    mb.exec();

}

void MainWindow::about()
{

    QString text = QString("<!DOCTYPE html><html lang=\"%1\">").arg(langID);

    text += tr("<h1>Fragmentarium</h1>"
               "<p>Version %1. </p>").arg(version.toLongString());

    text += tr("<p>An integrated environment for exploring GPU pixel graphics. </p>"
    "<p>Created by Mikael Hvidtfeldt Christensen.<br />Licensed and distributed under the LPGL or GPL license.</p>"
    "<p><b>Notice</b>: some fragment (GLSL) shaders are copyright by other authors, and may carry other licenses. Please check the fragment file header before redistributing."
    "<h1>Acknowledgement</h1>"
    "<p>"
    "Much of the inspiration and formulas for Fragmentarium came from the community at <a href=http://www.fractalforums.com>FractalForums.com</a>, including Tom Beddard, Jan Kadlec, Iñigo Quilez, Buddhi, Jesse, and others. Special thanks goes out to Knighty and Kali for their great fragments and claude for all his help with improvements. All fragments should include information about their origins - please notify me, if I made any mis-attributions."
    "</p>"
    "<p>The icons used are part of the <a href=\"http://www.everaldo.com/crystal/\">Everaldo: Crystal</a> project. </p>"
    "<p>Fragmentarium is built using the <a href=\"http://trolltech.com/developer/downloads/qt/index\">Qt cross-platform GUI framework</a>. </p>"
    "</p>"
    "<p>"
    "<table cellspacing=20><th colspan=2 align=left>Translations by Fractal Forums users</th>"
    "<tr><td>Russian</td><td align=center>SCORPION</td></tr>"
    "<tr><td>Russian</td><td align=center>Crist-JRoger</td></tr>"
    "<tr><td>German</td><td align=center>Sabine</td></tr>"
    "<tr><td>Dutch</td><td align=center>Sabine</td></tr>"
    "</table>"
    "</p>");

    showHelpMessage(tr("About Fragmentarium"), text);

}

void MainWindow::showControlHelp()
{

  QString text = QString("<!DOCTYPE html><html lang=\"%1\">").arg(langID);
    text += tr("<p>"
  "Notice: the 3D view must have keyboard focus!"
  "</p>"
  "<h2>2D</h2>"
  "<p>"
  "<ul>"
  "<li>Left mousebutton: translate center.</li>"
  "<li>Right mousebutton: zoom.</li>"
  "<li>Wheel: zoom</li>"
  "<li>A/D: left/right</li>"
  "<li>W/S: up/down</li>"
  "<li>Q/E: zoom in/out</li>"
  "</ul>"
  "</p>"
  "<h2>3D</h2>"
  "<p>"
  "<ul>"
  "<li>Shift+Right mouse button: shows menus when in fullscreen mode.</li>"
  "<li>Left mouse button: change camera direction.</li>"
  "<li>Right mouse button: move camera in screen plane.</li>"
  "<li>Left+Right mouse button: zoom.</li>"
  "<li>Shift+Left mouse button: rotate object (around origin).</li>"
  "<li>Shift+Alt+Left mouse button: rotate object (around target).</li>"
  "<li>Shift+Tilde (~) resets the view to look through origin (0,0,0)</li>"
  "<li>Wheel: Move forward/backward</li>"
  "<li>W/S: move forward/back.</li>"
  "<li>A/D: move left/right.</li>"
  "<li>Q/E: roll</li>"
  "<li>1/3: increase/decrease step size x2</li>"
  "<li>2: increase/decrease step size x10</li>"
  "<li>Shift+Wheel: change step size</li>"
  "<li>T/G: move up/down.</li>"
  "<li>R/F: yaw</li>"
  "<li>Y/H: pitch</li>"
  "</ul>"
  "</p>"
  "<h2>Sliders</h2>"
  "<p>"
  "When a (float) slider recieves a Right Mouse Button Click it opens an input dialog to set the step size.<br>"
  "<b>F7 Key</b> opens the easing curve editor for the currently selected slider."
  "</p>");

  text += "</html>";

  showHelpMessage(tr("Mouse and Keyboard Control"), text);

}

void MainWindow::showScriptingHelp()
{

  QString reqObj = sender()->objectName();

  QString text = QString("<!DOCTYPE html><html lang=\"%1\">").arg(langID);
  if( reqObj == "scriptingGeneralAction")
    text += tr("<h2>General commands</h2>"
    "<p>"
    "<ul>"
    "<p><li><b>void setFrame(int);</b></li>"
    "Sets the current frame number.</p>"

    "<p><li><b>int getFrame();</b></li>"
    "Returns the current frame number.</p>"

    "<p><li><b>void loadFragFile(String);</b></li>"
    "Opens a new editor tab, loads the named fragment file, initializes default preset,<br>"
    "initializes keyframes and easing curves if the file contains these settings.</p>"

    "<p><li><b>bool initializeFragment();</b></li>"
    "Returns success or fail.<br>"
    "Must be called after altering a locked variable before rendering an image.</p>"
    "</ul>"
    "</p>");
  if( reqObj == "scriptingParameterAction")
    text += tr("<h2>Parameter commands</h2>"
    "<p>"
    "<ul>"
    "<p><li><b>void setParameter(String);</b></li>"
    "Set a parameter from String in the form of \"parameter = value\" also accepts parameter file formated string.</p>"
    "<p><li><b>void setParameter(String, bool);</b></li>"
    "Sets a boolean parameter where String is the parameter name and bool is TRUE or FALSE</p>"
    "<p><li><b>void setParameter(String, int);</b></li>"
    "Sets an integer parameter where String is the parameter name and int is any integer.</p>"
    "<p><li><b>void setParameter(String, x);</b></li>"
    "Sets a float parameter where String is the parameter name and x is any floating point number.</p>"
    "<p><li><b>void setParameter(String, x, y);</b></li>"
    "Sets a float2 parameter where String is the parameter name and x,y are any floating point numbers.</p>"
    "<p><li><b>void setParameter(String, x, y, z);</b></li>"
    "Sets a float3 parameter where String is the parameter name and x,y,z are any floating point numbers.</p>"
    "<p><li><b>void setParameter(String, x, y, z, w);</b></li>"
    "Sets a float4 parameter where String is the parameter name and x,y,z,w are any floating point numbers.</p>"
    "<p><li><b>String getParameter(String);</b></li>"
    "Returns a string representing the value(s) for the named parameter, user must parse this into usable values.</p>"
    "<p><li><b>void applyPresetByName(String);</b></li>"
    "Applies the named preset.</p>"
    "</ul>"
    "</p>");
  if( reqObj == "scriptingHiresAction")
    text += tr("<h2>Hires image and animation dialog commands</h2>"
    "<p>"
    "<ul>"
    "<p><li><b>void setAnimationLength(int);</b></li>"
    "Sets the total animation duration in seconds.</p>"
    "<p><li><b>void setTileWidth(int);</b></li>"
    "<li><b>void setTileHeight(int);</b></li>"
    "Sets the tile width and height.</p>"
    "<p><li><b>void setTileMax(int);</b></li>"
    "Sets the number of row and column tiles, this value squared = total tiles.</p>"
    "<p><li><b>void setSubFrames(int);</b></li>"
    "Sets the number of frames to accumulate.</p>"
    "<p><li><b>void setOutputBaseFileName(String);</b></li>"
    "Sets the filename for saved image,<br>"
    "if script has total control this must be set by the script for every frame,<br>"
    "if animation is using frag file settings, keyframes etc., then this only needs to be set once to basename and Fragmentarium will add an index padded to 5 digits.</p>"
    "<p><li><b>void setFps(int);</b></li>"
    "Sets the frames per second for rendering.</p>"
    "<p><li><b>void setStartFrame(int);</b></li>"
    "Sets the start frame number for rendering a range of frames.</p>"
    "<p><li><b>void setEndFrame(int);</b></li>"
    "Sets the end frame number for rendering a range of frames.</p>"
    "<p><li><b>void setAnimation(bool);</b></li>"
    "FALSE sets animation to script control exclusively.<br>"
    "TRUE enables control from keyframes and easing curves.</p>"
    "<p><li><b>void setPreview(bool);</b></li>"
    "TRUE will preview frames in a window on the desktop instead of saving image files.<br>"
    "WARNING!!! this will open a window FOR EACH FRAME and close it when the next one is ready for display.</p>"
    "<p><li><b>void setAutoSave(bool);</b></li>"
    "TRUE will save files in subfolder.<br>"
    "FALSE will use the path set by setOutputBaseFileName(String)</p>"
    "<p><li><b>void setUniqueID(bool);</b></li>"
    "Does the same thing as \"Add unique ID to filename\" in the HiResolution and Animation Dialog.<br></p>"
    "</ul>"
    "</p>");
  if( reqObj == "scriptingControlAction")
    text += tr("<h2>Control commands</h2>"
    "<p>"
    "<ul>"
    "<p><li><b>bool scriptRunning();</b></li>"
    "Returns FALSE when the user selects the [Stop] button in the script editor."
    "For user implemented test in script to break out of the script control loop.</p>"
    "<p><li><b>void stopScript();</b></li>"
    "For user implemented test in script to break out of the script control loop or error like file not found, initialization fail etc.</p>"
    "<p><li><b>void tileBasedRender();</b></li>"
    "Begins rendering the current frame or range of frames applying the current state for keyframes and active easing settings.</p>"

    "</ul>"
    "</p>");

  text += "</html>";

  showHelpMessage(tr("Fragmentarium script commands."), text);

}

void MainWindow::documentWasModified()
{

    if (tabBar->currentIndex() < 0) {
        return;
    }
    // when all is undone
    if (tabInfo[tabBar->currentIndex()].textEdit->document()->availableUndoSteps() == 0) {
        tabInfo[tabBar->currentIndex()].unsaved = false;
        highlightBuildButton(false);
    } else {
        tabInfo[tabBar->currentIndex()].unsaved = true;
        highlightBuildButton(true);
    }

    if (tabBar->currentIndex() > tabInfo.size()) {
        return;
    }

    TabInfo t = tabInfo[tabBar->currentIndex()];
    QString tabTitle = QString("%1%2").arg(strippedName(t.filename)).arg(t.unsaved ? "*" : "");
    setWindowTitle(QString("%1 - %2").arg(tabTitle).arg("Fragmentarium"));
    stackedTextEdits->setCurrentWidget(t.textEdit);
    tabBar->setTabText(tabBar->currentIndex(), tabTitle);

    if(tabInfo[tabBar->currentIndex()].textEdit->fh->changeGLSLVersion()) {
        tabInfo[tabBar->currentIndex()].textEdit->fh->rehighlight();
    }
}

void MainWindow::init()
{

    lastTime = new QElapsedTimer();
    lastTime->start();

    splitter = new QSplitter(this);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setOrientation(Qt::Horizontal);

    stackedTextEdits = new QStackedWidget(splitter);
    splitter->addWidget(stackedTextEdits);

    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();

    engine = new DisplayWidget(this, splitter );

    connect(engine, SIGNAL(easingCurveError(QString)), this, SLOT(easingMessage(QString)));

    if(fullScreenEnabled)
        engine->setMaximumSize(screenGeometry.width(),screenGeometry.height());
    else
        engine->setMaximumSize(screenGeometry.width()*0.925,screenGeometry.height()*0.65);

    engine->setObjectName(QString::fromUtf8("DisplayWidget"));
    engine->setMinimumSize(64,32);
    engine->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    splitter->addWidget( engine );
    tabBar = new QTabBar(this);
    tabBar->setObjectName(QString::fromUtf8("TabBar"));
    tabBar->setTabsClosable(true);

    tabBar->setMovable(false);
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);

    connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

    fpsLabel = new QLabel(this);
    statusBar()->addPermanentWidget(fpsLabel);

    QFrame* f = new QFrame(this);
    frameMainWindow = new QVBoxLayout();
    frameMainWindow->setSpacing(0);
    frameMainWindow->setMargin(4);
    f->setLayout(frameMainWindow);
    f->layout()->addWidget(tabBar);
    f->layout()->addWidget(splitter);
    setCentralWidget(f);

    QDir d(getExamplesDir());

    setDockOptions(AnimatedDocks | AllowTabbedDocks);

    // Log widget (in dockable window)
    dockLog = new QDockWidget(this);
    dockLog->setWindowTitle(tr("Log"));
    dockLog->setObjectName(QString::fromUtf8("dockWidget"));
    dockLog->setAllowedAreas(Qt::RightDockWidgetArea|Qt::BottomDockWidgetArea);
    QWidget* dockLogContents = new QWidget(dockLog);
    dockLogContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
    QVBoxLayout *vboxLayout1 = new QVBoxLayout(dockLogContents);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    vboxLayout1->setContentsMargins(0, 0, 0, 0);
    logger = new ListWidgetLogger(dockLog);
    connect (logger->getListWidget(), SIGNAL(loadSourceFile(QString, int)), this, SLOT(loadErrorSourceFile(QString, int)));
    vboxLayout1->addWidget(logger->getListWidget());
    dockLog->setWidget(dockLogContents);
    addDockWidget(Qt::BottomDockWidgetArea, dockLog);

    // Variable editor (in dockable window)
    dockVariableEditor = new QDockWidget(this);
    dockVariableEditor->setMinimumWidth(320);
    dockVariableEditor->setWindowTitle(tr("Variable Editor (uniforms)"));
    dockVariableEditor->setObjectName(QString::fromUtf8("dockVariableEditor"));
    dockVariableEditor->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    QWidget* editorLogContents = new QWidget(dockLog);
    editorLogContents->setObjectName(QString::fromUtf8("editorLogContents"));
    QVBoxLayout *vboxLayout2 = new QVBoxLayout(editorLogContents);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    vboxLayout2->setContentsMargins(0, 0, 0, 0);
    variableEditor = new VariableEditor(dockVariableEditor, this);
    variableEditor->setObjectName(QString::fromUtf8("VariableEditor"));
    variableEditor->setMinimumWidth(320);
    connect(variableEditor, SIGNAL(changed(bool)), this, SLOT(variablesChanged(bool)));
    vboxLayout2->addWidget(variableEditor);
    dockVariableEditor->setWidget(editorLogContents);
    addDockWidget(Qt::RightDockWidgetArea, dockVariableEditor);

    connect(dockVariableEditor, SIGNAL(topLevelChanged(bool)), this, SLOT(veDockChanged(bool))); // 05/22/17 Sabine ;)

    setMouseTracking(true);

    INFO(tr("Welcome to Fragmentarium version %1. A Syntopia Project.").arg(version.toLongString()));
    WARNING(tr("This is an experimental 3Dickulus build."));
    INFO("");

    connect(this->tabBar, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    {
        if (settings.value("firstRun", true).toBool()) {
            showWelcomeNote();
        }
        settings.setValue("firstRun", false);
    }

    {
        if (settings.value("isStarting", false).toBool()) {
            QString s = tr("It looks like the Fragmentarium program crashed during the last startup.\n"
                        "\nTo prevent repeated crashes, you may choose to disable 'Autorun on Load'."
                        "\nThis option may be re-enabled through Preferences");

            QMessageBox msgBox(this);
            msgBox.setWindowFlag(Qt::WindowStaysOnTopHint);
            msgBox.setText(s);
            msgBox.setIcon(QMessageBox::Warning);
            QAbstractButton *b = msgBox.addButton(tr("Disable Autorun"), QMessageBox::AcceptRole);
            msgBox.addButton(tr("Enable Autorun"),QMessageBox::RejectRole);

            msgBox.exec();
            if (msgBox.clickedButton() == b) {
                settings.setValue("autorun", false);
            } else {
                settings.setValue("autorun", true);
            }
        }
        settings.setValue("isStarting", true);
    }

    readSettings();

    if (loggingToFile) {
        logger->setLogToFile();
    }

    createActions();

    createToolBars();
    createStatusBar();
    createMenus();
    renderModeChanged();

    initTools();

    highlightBuildButton( !(QSettings().value("autorun", true).toBool()) );
    setupScriptEngine();
    play();
    veDockChanged(true);
    QString styleSheetFile = "file:///"+guiStylesheet;
    if(guiStylesheet.isEmpty()) styleSheetFile = guiStylesheet;
    qApp->setStyleSheet(styleSheetFile);

    int x = settings.value("tilewidth",456).toInt();
    int y = settings.value("tileheight",256).toInt();

    if(!lockedToWindowSize){
        bufferSizeControl->blockSignals(true);
        bufferSizeControl->setText( bufferActionCustom->text() );
        bufferSizeControl->blockSignals(false);

        bufferXSpinBox->blockSignals(true);
        bufferYSpinBox->blockSignals(true);
        bufferXSpinBox->setValue(x);
        bufferYSpinBox->setValue(y);
        bufferXSpinBox->blockSignals(false);
        bufferYSpinBox->blockSignals(false);

        bufferYSpinBox->setEnabled(!lockedToWindowSize);
        bufferXSpinBox->setEnabled(!lockedToWindowSize);
    }
    currentAspect = (double)x/(double)y;
    lockAspect(lockedAspect);
    aspectLock->setEnabled(!lockedToWindowSize);
}

void MainWindow::initTools()
{
    // FIXME need to destroy the old one because none of this will happen if exrToolsMenu has already been built
    // like when the preferences are changed changes wont take effect without restarting the program.
    // TODO integrate these commands so that users get more than just the help info for commandline execution.
    // parse help info and create a dialog that accurately represents the available options?
#ifdef Q_OS_LINUX
    // do we have any paths?
    if(exrBinaryPath.count() != 0) {
        // has it been done already?
        if (exrToolsMenu == nullptr) {
            // iterate over the list of paths
            QStringListIterator pathIterator(exrBinaryPath);
            while (pathIterator.hasNext()) {
                // validate existence
                QDir exrbp(pathIterator.next());
                // if found at least one valid path
                if(exrbp.exists()) {
                    QStringList filters; filters << "exr*";
                    exrbp.setNameFilters(filters);
                    QStringList filesList = exrbp.entryList();
                    // if found at least one valid tool iterate over the list of tools adding menu items
                    if(filesList.count() > 0) {
                        QStringListIterator toolIterator(filesList);
                        while (toolIterator.hasNext()) {
                            // get abs path to file
                            QString toolName = exrbp.absoluteFilePath(toolIterator.next());
                            QFileInfo qfi(toolName);
                            // validate as executable
                            if(qfi.isExecutable()) {
                                // we have path and files so add menu to bar just once though
                                if (exrToolsMenu == nullptr) {
                                    exrToolsMenu = menuBar()->addMenu(tr("EXR Tools"));
                                }
                                // add item to menu
                                QAction *a = new QAction(qfi.fileName(), this);
                                a->setData(toolName);
                                a->setObjectName(toolName);
                                connect(a, SIGNAL(triggered()), this, SLOT(runEXRTool()));
                                exrToolsMenu->addAction(a);
                            }
                        }
                    }
                }
            }
        }
    }
#endif // Q_OS_LINUX
    // do we have any paths?
    if(supportProgramsBinaryPath.count() != 0) {
        // has it been done already?
        if(supportProgramsMenu == nullptr) {
            // iterate over the list of paths
            QStringListIterator pathIterator(supportProgramsBinaryPath);
            while (pathIterator.hasNext()) {
                // validate existence
                QDir spbp(pathIterator.next());
                // if found at least one valid path
                if(spbp.exists()) {
                    QStringList filters; filters << "*2glsl";
                    spbp.setNameFilters(filters);
                    QStringList filesList = spbp.entryList();
                    // if found at least one valid tool iterate over the list of tools adding menu items
                    if(filesList.count() > 0) {
                        QStringListIterator toolIterator(filesList);
                        while (toolIterator.hasNext()) {
                            // get abs path to file
                            QString toolName = spbp.absoluteFilePath(toolIterator.next());
                            QFileInfo qfi(toolName);
                            // validate as executable
                            if(qfi.isExecutable()) {
                                // we have path and files so add menu to bar just once though
                                if (supportProgramsMenu == nullptr) {
                                    supportProgramsMenu = menuBar()->addMenu(tr("GLSL Tools"));
                                }
                                // add item to menu
                                QAction *a = new QAction(qfi.fileName(), this);
                                a->setData(toolName);
                                a->setObjectName(toolName);
                                connect(a, SIGNAL(triggered()), this, SLOT(runSupportProgram()));
                                supportProgramsMenu->addAction(a);
                            }
                        }
                    }
                }
            }
            // the above code only looks for the ggr2glsl binary in the support programs path
            // because the EXR tools have their own menu and ggr2glsl is the only one supported
            // TODO add the ability to use shell scripts from this menu ???
            // anyhoo this is where we add the action to access the internal ggr2glsl option
            // add item to menu
            // we have builtin ggr2glsl now so make sure the menu exists to add an item... again
            if (supportProgramsMenu == nullptr) {
                supportProgramsMenu = menuBar()->addMenu(tr("GLSL Tools"));
            }
            QAction *a = new QAction(tr("Import Gimp Gradient"), this);
            a->setObjectName("convertgradient");
            connect(a, SIGNAL(triggered()), this, SLOT(runSupportProgram()));
            supportProgramsMenu->addAction(a);
        }
    }
}

void MainWindow::runEXRTool()
{

    QString cmnd = sender()->objectName();
    // execute once with -h option and capture the output
    cmnd += " -h &> .htxt"; // > filename 2>&1
    if(system( cmnd.toStdString().c_str() ) != -1) {

        // open the resulting textfile and parse for command information
        QFile file(".htxt");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }
        // grab the general help text
        QString helpText;
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            if(QString(line).contains(QDir::separator())) line = QString(line).split(QDir::separator()).last().toLocal8Bit();
            if (QString(line).contains("Options") && QString(line).contains(":")) {
                break;
            }
            helpText += QString(line);
        }
        // grab the option details
        QString detailedText;
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            if (QString(line).contains("Options:")) {
                continue;
            }
            detailedText += QString(line);
        }
        file.remove();

        // display for user
        QMessageBox msgBox(this);
        msgBox.setText(helpText);
        msgBox.setDetailedText(detailedText);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
}

void MainWindow::runSupportProgram()
{
    QString cmnd = "\"" + sender()->objectName() + "\""; // in case the program name has spaces
    bool cmndOk = false;
    // for now it's ggr2glsl program only
    if(cmnd.contains("ggr2glsl", Qt::CaseInsensitive)) {
        // get input file file
        QString filter = tr("GIMP gradients (*.ggr);;All Files (*.*)");
        QString startFolder = gimpGradientsPaths.split(";").at(0);
        if(!QFile::exists(startFolder) && gimpGradientsPaths.split(";").count() > 1) startFolder = gimpGradientsPaths.split(";").at(1);
        if(!QFile::exists(startFolder)) startFolder = ""; // tried the first 2 no luck? set to empty
        QString ggrFileName = QFileDialog::getOpenFileName(this, tr("Read GIMP Gradient"), startFolder, filter);
        if (ggrFileName.isEmpty() || !ggrFileName.endsWith(".ggr") ) {
            return;
        }
        cmnd += " < \"" + ggrFileName + "\""; // in case the gradient name has spaces
        // get output file
        filter = tr("Fragments (*.frag);;All Files (*.*)");
        QString glslFileName = QFileDialog::getSaveFileName(this, tr("Write GLSL Fragment"), ggrFileName.replace(".ggr","-gradient.frag"), filter);
        if (glslFileName.isEmpty()) {
            return;
        }
        cmnd += " > \"" + glslFileName + "\""; // in case the fragment name has spaces

        // confirm user request
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText( cmnd );
        msgBox.setInformativeText(tr("Do you want to execute this command?"));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Ok:
            cmndOk = true;
            break;
        case QMessageBox::Cancel:
            cmnd = ""; cmndOk = false;
            break;
        default:
            // should never be reached
            cmnd = ""; cmndOk = false;
            break;
        }

        bool success = (cmndOk && system( cmnd.toStdString().c_str() ) != -1);

        // confirm success
        msgBox.setIcon(success ? QMessageBox::Information : QMessageBox::Critical);
        msgBox.setText( cmnd );
        msgBox.setInformativeText(success ? tr("Command succeeded!") : tr("Execute command Failed!"));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
    }

    if(cmnd.contains("convertgradient", Qt::CaseInsensitive)) {
        // get input file file
        QString filter = tr("GIMP gradients (*.ggr);;All Files (*.*)");
        QString startFolder = gimpGradientsPaths.split(";").at(0);
        if(!QFile::exists(startFolder) && gimpGradientsPaths.split(";").count() > 1) startFolder = gimpGradientsPaths.split(";").at(1);
        if(!QFile::exists(startFolder)) startFolder = ""; // tried the first 2 no luck? set to empty

        QString ggrFileName = QFileDialog::getOpenFileName(this, tr("Read GIMP Gradient"), startFolder, filter);
        if (ggrFileName.isEmpty() || !ggrFileName.endsWith(".ggr") ) {
            return;
        } else insertTabPage(ggrFileName);
    }
}

void MainWindow::showWelcomeNote()
{

    QString s =
        tr("This is your first run of Fragmentarium.\nPlease read this:\n\n"
        "(1) Fragmentarium requires a decent GPU, preferably a NVIDIA or ATI discrete graphics card with recent drivers.\n\n"
        "(2) On Windows there is a built-in GPU watchdog, requiring each frame to render in less than 2 seconds. Some fragments may exceed this limit, especially on low-end graphics cards. It is possible to circumvent this, see the Fragmentarium FAQ (in the Help menu) for more information.\n\n"
        "(3) Many examples in Fragmentarium use progressive rendering, which requires Fragmentarium to run in Continuous mode. When running in this mode, Fragmentarium uses 100% GPU power (but you may use the 'Subframe : Max' spinbox to limit the number of frames rendered. A value of zero means there is no maximum count.)\n\n");
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(s);
    msgBox.exec();

}

bool MainWindow::isChangedUniformInBuffershaderOnly()
{
    if(!engine->hasShader() || !engine->hasBufferShader()) return false;
    QStringList lastSet;
    if(getTextEdit() != 0) lastSet = getTextEdit()->testParms().split("\n");
    bool inBufferShaderOnly = false;
    if (!lastSet.isEmpty()) {
        QStringList thisSet = variableEditor->getSettings().split("\n");
        if (!thisSet.isEmpty()) { // test last setting against new settings to determine changed uniform name and provenance
            if (lastSet.count() == thisSet.count()) {
                foreach(QString t, thisSet) {
                    int index = thisSet.indexOf(t);
                    if(t != lastSet[index]) {
                        QString changedUniformName = t.split(" ").at(0); // determine if uniform lives in buffershader only
                        inBufferShaderOnly = ( engine->getBufferShader()->uniformLocation(changedUniformName ) != -1 && engine->getShader()->uniformLocation(changedUniformName ) == -1 );
                    }
                }
            } // else DBOUT << "Index mismatch.";
        } // else DBOUT << "Empty settings.";
        getTextEdit()->parmsToTest( thisSet.join("\n") );    // track the most recent set
    }
    return inBufferShaderOnly;
}

void MainWindow::variablesChanged(bool lockedChanged)
{
    if (lockedChanged) {
        highlightBuildButton(true);
        engine->clearTextureCache(nullptr);
    }
    bool bso = isChangedUniformInBuffershaderOnly();
    engine->uniformsHaveChanged(bso);
}

void MainWindow::createOpenGLContextMenu()
{

    openGLContextMenu = new QMenu();
    openGLContextMenu->addAction(fullScreenAction);
    openGLContextMenu->addMenu(fileMenu);
    openGLContextMenu->addMenu(renderMenu);
    openGLContextMenu->addMenu(examplesMenu);
    openGLContextMenu->addMenu(helpMenu);
    openGLContextMenu->addMenu(optionsMenu);
    openGLContextMenu->addAction(exitAction);
    engine->setContextMenu(openGLContextMenu);
}

void MainWindow::toggleFullScreen()
{
        QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    if (fullScreenEnabled) { // it's on so toggle it off
        frameMainWindow->setMargin(4);
        engine->setMaximumSize(screenGeometry.width()*0.925,screenGeometry.height()*0.65);
        showNormal();
        fullScreenEnabled = false;
        fullScreenAction->setChecked(false);
        stackedTextEdits->show();
        menuBar()->show();
        statusBar()->show();
        tabBar->show();
        bufferToolBar->show();
    } else {
        frameMainWindow->setMargin(0);
        fullScreenAction->setChecked(true);
        fullScreenEnabled = true;
        bufferToolBar->hide();
        tabBar->hide();
        stackedTextEdits->hide();
        menuBar()->hide();
        statusBar()->hide();
        engine->setMaximumSize(screenGeometry.width(),screenGeometry.height());
        showFullScreen();
    }
}

void MainWindow::createActions()
{

    fullScreenAction = new QAction(tr("Fullscreen (ESC key toggles)"), this);
    connect(fullScreenAction, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));

    screenshotAction = new QAction(tr("Save Screen Shot..."), this);
    connect(screenshotAction, SIGNAL(triggered()), this, SLOT(makeScreenshot()));

    new2DAction = new QAction(QIcon(":/Icons/new2D.png"), tr("&New 2D Fragment"), this);
    new2DAction->setShortcut(tr("Ctrl+N"));
    new2DAction->setStatusTip(tr("Create a new 2D fragment"));
    connect(new2DAction, SIGNAL(triggered()), this, SLOT(new2DFile()));

    new3DAction = new QAction(QIcon(":/Icons/new3D.png"), tr("&New 3D Fragment"), this);
    new3DAction->setShortcut(tr("Ctrl+Shift+N"));
    new3DAction->setStatusTip(tr("Create a new 3D fragment"));
    connect(new3DAction, SIGNAL(triggered()), this, SLOT(new3DFile()));

    openAction = new QAction(QIcon(":/Icons/open.png"), tr("&Open..."), this);
    openAction->setShortcut(tr("Ctrl+O"));
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

    reloadAction = new QAction(QIcon(":/Icons/view-refresh.png"), tr("&Reload..."), this);
    reloadAction->setShortcut(tr("Ctrl+R"));
    reloadAction->setStatusTip(tr("Reload file in current tab"));
    connect(reloadAction, SIGNAL(triggered()), this, SLOT(reloadFrag()));

    saveAction = new QAction(QIcon(":/Icons/save.png"), tr("&Save"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save the script to disk"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAction = new QAction(QIcon(":/Icons/filesaveas.png"), tr("Save &As..."), this);
    saveAsAction->setShortcut(tr("Ctrl+Shift+S"));
    saveAsAction->setStatusTip(tr("Save the script under a new name"));
    connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    closeAction = new QAction(QIcon(":/Icons/fileclose.png"), tr("&Close Tab"), this);
    closeAction->setShortcut(tr("Ctrl+W"));
    closeAction->setStatusTip(tr("Close this tab"));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(closeTab()));

    exitAction = new QAction(QIcon(":/Icons/exit.png"), tr("E&xit Application"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    cutAction = new QAction(QIcon(":/Icons/cut.png"), tr("Cu&t"), this);
    cutAction->setShortcut(tr("Ctrl+X"));
    cutAction->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));

    copyAction = new QAction(QIcon(":/Icons/copy.png"), tr("&Copy"), this);
    copyAction->setShortcut(tr("Ctrl+C"));
    copyAction->setStatusTip(tr("Copy the current selection's contents to the clipboard"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAction = new QAction(QIcon(":/Icons/paste.png"), tr("&Paste"), this);
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setStatusTip(tr("Paste the clipboard's contents into the current selection"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));

    findAction = new QAction(QIcon(":/Icons/find.png"), tr("&Find"), this);
    findAction->setShortcut(tr("Ctrl+F"));
    findAction->setStatusTip(tr("Finds the current selection"));
    connect(findAction, SIGNAL(triggered()), this, SLOT(search()));

    asmAction = new QAction(QIcon(":/Icons/find.png"), tr("Shader &Asm"), this);
    asmAction->setShortcut(tr("Ctrl+A"));
    asmAction->setStatusTip(tr("NV objdump"));
    connect(asmAction, SIGNAL(triggered()), this, SLOT(dumpShaderAsm()));

    scriptAction = new QAction(tr("&Edit Cmd Script"), this);
    scriptAction->setShortcut(tr("Ctrl+E"));
    scriptAction->setStatusTip(tr("Edit the currently loaded script"));
    connect(scriptAction, SIGNAL(triggered()), this, SLOT(editScript()));

    renderAction = new QAction(QIcon(":/Icons/render.png"), tr("Compile &GLSL"), this);
    renderAction->setShortcut(tr("F5"));
    renderAction->setStatusTip(tr("Render the current ruleset"));
    renderAction->setObjectName("renderAction");
    connect(renderAction, SIGNAL(triggered()), this, SLOT(rebuildUniforms()));

    videoEncoderAction = new QAction(QIcon(":/Icons/player_eject.png"), tr("&Video Encoding"), this);
    videoEncoderAction->setStatusTip(tr("Encode rendered frames to video"));
    connect(videoEncoderAction, SIGNAL(triggered()), this, SLOT(videoEncoderRequest()));

    // Help menu local
    aboutAction = new QAction(QIcon(":/Icons/documentinfo.png"), tr("&About"), this);
    aboutAction->setStatusTip(tr("Shows the About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    welcomeAction = new QAction(QIcon(":/Icons/documentinfo.png"), tr("Show Welcome Note"), this);
    connect(welcomeAction, SIGNAL(triggered()), this, SLOT(showWelcomeNote()));

    controlAction = new QAction(QIcon(":/Icons/documentinfo.png"), tr("&Mouse and Keyboard Help"), this);
    controlAction->setStatusTip(tr("Shows information about how to control Fragmentarium"));
    connect(controlAction, SIGNAL(triggered()), this, SLOT(showControlHelp()));

    scriptingGeneralAction = new QAction(QIcon(":/Icons/documentinfo.png"), tr("&Scripting General Help"), this);
    scriptingGeneralAction->setStatusTip(tr("Shows information about how to control Fragmentarium via Script"));
    scriptingGeneralAction->setObjectName(QString::fromUtf8("scriptingGeneralAction"));
    connect(scriptingGeneralAction, SIGNAL(triggered()), this, SLOT(showScriptingHelp()));

    scriptingParameterAction = new QAction(QIcon(":/Icons/documentinfo.png"), tr("&Scripting Parameter Help"), this);
    scriptingParameterAction->setStatusTip(tr("Shows information about how to control Fragmentarium via Script"));
    scriptingParameterAction->setObjectName(QString::fromUtf8("scriptingParameterAction"));
    connect(scriptingParameterAction, SIGNAL(triggered()), this, SLOT(showScriptingHelp()));

    scriptingHiresAction = new QAction(QIcon(":/Icons/documentinfo.png"), tr("&Scripting Image Anim Dialog Help"), this);
    scriptingHiresAction->setStatusTip(tr("Shows information about how to control Fragmentarium via Script"));
    scriptingHiresAction->setObjectName(QString::fromUtf8("scriptingHiresAction"));
    connect(scriptingHiresAction, SIGNAL(triggered()), this, SLOT(showScriptingHelp()));

    scriptingControlAction = new QAction(QIcon(":/Icons/documentinfo.png"), tr("&Scripting Control Help"), this);
    scriptingControlAction->setStatusTip(tr("Shows information about how to control Fragmentarium via Script"));
    scriptingControlAction->setObjectName(QString::fromUtf8("scriptingControlAction"));
    connect(scriptingControlAction, SIGNAL(triggered()), this, SLOT(showScriptingHelp()));

    // Help menu remote
    sfHomeAction = new QAction(QIcon(":/Icons/agt_internet.png"), tr("&Project Homepage (web link)"), this);
    sfHomeAction->setStatusTip(tr("Open the project page in a browser."));
    connect(sfHomeAction, SIGNAL(triggered()), this, SLOT(launchSfHome()));

    ffReferenceAction = new QAction(QIcon(":/Icons/agt_internet.png"), tr("&Fragmentarium@FractalForums (web link)"), this);
    ffReferenceAction->setStatusTip(tr("Open a FractalForums.com Fragmentarium web page in a browser."));
    connect(ffReferenceAction, SIGNAL(triggered()), this, SLOT(launchReferenceHome()));

    fragmReferenceAction = new QAction(QIcon(":/Icons/agt_internet.png"), tr("&Fragmentarium Documentation (web link)"), this);
    fragmReferenceAction->setStatusTip(tr("Open a Fragmentarium reference web page in a browser."));
    connect(fragmReferenceAction, SIGNAL(triggered()), this, SLOT(launchDocumentation()));

    galleryAction = new QAction(QIcon(":/Icons/agt_internet.png"), tr("&Flickr Fragmentarium Group (web link)"), this);
    galleryAction->setStatusTip(tr("Opens the main Flickr group for Fragmentarium creations."));
    connect(galleryAction, SIGNAL(triggered()), this, SLOT(launchGallery()));

    glslHomeAction = new QAction(QIcon(":/Icons/agt_internet.png"), tr("&GLSL Specifications (web link)"), this);
    glslHomeAction->setStatusTip(tr("The official specifications for all GLSL versions."));
    connect(glslHomeAction, SIGNAL(triggered()), this, SLOT(launchGLSLSpecs()));

    introAction = new QAction(QIcon(":/Icons/agt_internet.png"), tr("Introduction to Distance Estimated Fractals (web link)"), this);
    connect(introAction, SIGNAL(triggered()), this, SLOT(launchIntro()));

    faqAction = new QAction(QIcon(":/Icons/agt_internet.png"), tr("Fragmentarium FAQ (web link)"), this);
    connect(faqAction, SIGNAL(triggered()), this, SLOT(launchFAQ()));

    // Options menu
    clearTexturesAction = new QAction(tr("Clear Texture Cache"), this);
    clearTexturesAction->setObjectName(QString::fromUtf8("clearTexturesAction"));
    connect(clearTexturesAction, SIGNAL(triggered()), this, SLOT(clearTextures()));

    clearAnimationObjectsAction = new QAction(tr("Clear Animation Actions"), this);
    clearAnimationObjectsAction->setStatusTip(tr("Clears camera control and Easingcurve cache"));
    clearAnimationObjectsAction->setObjectName(QString::fromUtf8("clearAnimationObjectsAction"));
    connect(clearAnimationObjectsAction, SIGNAL(triggered()), this, SLOT(clearKeyFrames()));

    testCompileGLSLAction = new QAction(tr("Test versions"), this);
    testCompileGLSLAction->setStatusTip(tr("Tests the current fragment against all supported GLSL versions."));
    testCompileGLSLAction->setObjectName(QString::fromUtf8("testCompileGLSLAction"));
    connect(testCompileGLSLAction, SIGNAL(triggered()), this, SLOT(testCompileGLSL()));

    loopCameraPathAction = new QAction(tr("Loop Camera Path"), this);
    loopCameraPathAction->setStatusTip(tr("Makes a looping camera path."));
    loopCameraPathAction->setCheckable(true);
    loopCameraPathAction->setChecked(loopCameraPath);
    connect(loopCameraPathAction, SIGNAL(toggled(bool)), this, SLOT(setCameraPathLoop(bool)));

    for (int i = 0; i < maxRecentFiles; ++i) {
        QAction *a = new QAction(this);
        a->setVisible(false);
        connect(a, SIGNAL(triggered()), this, SLOT(openFile()));
        recentFileActions.append(a);
    }

    qApp->setWindowIcon(QIcon(":/Icons/fragmentarium.png"));
}

void MainWindow::createMenus()
{

    // -- File Menu --
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(new2DAction);
    fileMenu->addAction(new3DAction);
    fileMenu->addAction(openAction);
    fileMenu->addAction(reloadAction);
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);

    recentFileSeparator = fileMenu->addSeparator();
    for (int i = 0; i < maxRecentFiles; ++i) {
        fileMenu->addAction(recentFileActions[i]);
    }
    fileMenu->addSeparator();
    fileMenu->addAction(closeAction);
    fileMenu->addAction(exitAction);

    // -- Edit Menu --
    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);
    editMenu->addAction(findAction);

    editMenu->addAction(asmAction);

    editMenu->addSeparator();
    editMenu->addAction(tr("Indent Script"), this, SLOT(indent()));
    QMenu* m = editMenu->addMenu(tr("Insert Command"));
    createCommandHelpMenu(m, this, this);
    editMenu->addAction(tr("Add Easing Curve"), this, SLOT(setEasing()), QKeySequence("F7"));
    editMenu->addAction(tr("Add Key Frame"), this, SLOT(insertPreset()), QKeySequence("F8"));
    editMenu->addAction(tr("Select Preset"), this, SLOT(selectPreset()), QKeySequence("F9"));
    editMenu->addSeparator();
    editMenu->addAction(tr("Timeline Editor"), this, SLOT(timeLineRequest()));
    editMenu->addAction(tr("Preferences..."), this, SLOT(preferences()));

    // -- Render Menu --
    renderMenu = menuBar()->addMenu(tr("&Render"));
    renderMenu->addAction(renderAction);
    renderMenu->addAction(QIcon(":/Icons/render.png"), tr("High Resolution and Animation Render"), this, SLOT(tileBasedRender()));
    renderMenu->addSeparator();
    renderMenu->addAction(tr("Output Preprocessed Script (for Debug)"), this, SLOT(showPreprocessedScript()));
    renderMenu->addSeparator();
    renderMenu->addAction(fullScreenAction);
    renderMenu->addAction(screenshotAction);
    renderMenu->addAction(scriptAction);
    renderMenu->addAction(videoEncoderAction);

    // -- Parameters Menu --
    parametersMenu = menuBar()->addMenu(tr("&Parameters"));
    parametersMenu->addAction(tr("Reset All"), variableEditor, SLOT(resetUniforms()), QKeySequence("F1"));
    parametersMenu->addSeparator();
    parametersMenu->addAction(tr("Copy Settings"), variableEditor, SLOT(copy()), QKeySequence("F2"));
    parametersMenu->addAction(tr("Copy Group"), variableEditor, SLOT(copyGroup()), QKeySequence("Shift+F2"));
    parametersMenu->addAction(tr("Paste from Clipboard"), variableEditor, SLOT(paste()), QKeySequence("F3"));
    parametersMenu->addAction(tr("Paste from Selected Text"), this, SLOT(pasteSelected()), QKeySequence("Shift+F3"));
    parametersMenu->addAction(tr("Group to preset"), variableEditor, SLOT(groupToPreset()), QKeySequence("F4"));
    parametersMenu->addSeparator();
    parametersMenu->addAction(tr("Save to File"), this, SLOT(saveParameters()));
    parametersMenu->addAction(tr("Load from File"), this, SLOT(loadParameters()));
    parametersMenu->addSeparator();

    // -- Examples Menu --
    examplesMenu = menuBar()->addMenu(tr("&Examples"));
    buildExamplesMenu();

    // RMB in menu bar for "windows" menu access
    QMenu* mc = createPopupMenu();
    mc->setTitle(tr("Windows"));
    menuBar()->addMenu(mc);

    helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(aboutAction);
    helpMenu->addAction(welcomeAction);
    helpMenu->addAction(controlAction);
    helpMenu->addAction(scriptingGeneralAction);
    helpMenu->addAction(scriptingParameterAction);
    helpMenu->addAction(scriptingHiresAction);
    helpMenu->addAction(scriptingControlAction);

    helpMenu->addSeparator();

    helpMenu->addAction(sfHomeAction);
    helpMenu->addAction(ffReferenceAction);
    helpMenu->addAction(fragmReferenceAction);
    helpMenu->addAction(galleryAction);
    helpMenu->addAction(glslHomeAction);
    helpMenu->addAction(faqAction);
    helpMenu->addAction(introAction);

    optionsMenu = menuBar()->addMenu(tr("&Options"));
    optionsMenu->addMenu(mc); // "windows" menu
    optionsMenu->addAction(clearTexturesAction);
    optionsMenu->addAction(clearAnimationObjectsAction);
    optionsMenu->addAction(testCompileGLSLAction);
    optionsMenu->addAction(loopCameraPathAction);

    createOpenGLContextMenu();

}

void MainWindow::buildExamplesMenu()
{

    QStringList filters;
    // Scan examples dir...
    QDir d(getExamplesDir());
    filters.clear();
    filters << "*.frag";
    d.setNameFilters(filters);
    if (!d.exists()) {
        QAction* a = new QAction(tr("Unable to locate: ")+d.absolutePath(), this);
        a->setEnabled(false);
        examplesMenu->addAction(a);
    } else {
        // first we clean it
        examplesMenu->clear();
        // we will recurse the dirs...
        QStack<QString> pathStack;
        pathStack.append(QDir(getExamplesDir()).absolutePath());

        QMap< QString , QMenu* > menuMap;
        while (!pathStack.isEmpty()) {

            QMenu* currentMenu = examplesMenu;
            QString path = pathStack.pop();
            if (menuMap.contains(path)) {
                currentMenu = menuMap[path];
            }
            QDir dir(path);

            QStringList sl = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (int i = 0; i < sl.size(); i++) {
                if(!sl[i].contains("Include")) {
                    QMenu *menu = new QMenu(sl[i]);
                    QString absPath = QDir(path + "/" +  sl[i]).absolutePath();
                    menuMap[absPath] = menu;
                    currentMenu->addMenu(menu);
                    menu->setIcon(QIcon(":/Icons/folder.png"));
                    pathStack.push(absPath);
                }
            }

            dir.setNameFilters(filters);

            sl = dir.entryList();
            for (int i = 0; i < sl.size(); i++) {
                QAction *a = new QAction(sl[i], this);
                a->setIcon(QIcon(":/Icons/mail_new.png"));

                QString absPath = QDir(path ).absoluteFilePath(sl[i]);

                a->setData(absPath);
                connect(a, SIGNAL(triggered()), this, SLOT(openFile()));
                currentMenu->addAction(a);
            }
        }
    }
}

QString MainWindow::makeImgFileName(int timeStep, int timeSteps, const QString fileName)
{

    QString name = fileName;

    if (timeSteps > 1) {
        int digits = 5;
        if (timeSteps > 99999) {
            digits = 6;
        } // possible, feature-film length
        if (timeSteps > 999999) {
            digits = 7;
        } // unlikely
        int lastPoint = fileName.lastIndexOf(".");
        name = QString("%1.%2.%3")
               .arg(fileName.left(lastPoint))
               .arg(timeStep,digits,10,QChar('0'))
               .arg(fileName.right(fileName.size()-lastPoint-1));
    }
    return name;
}

void MainWindow::renderTiled(int maxTiles, int tileWidth, int tileHeight, int padding, int maxSubframes, int &steps, QProgressDialog &progress, QVector<QImage> &cachedTileImages, QElapsedTimer &totalTime, double time)
{
    QElapsedTimer imagetime;

    if (!scriptRunning() && engine->getState() != DisplayWidget::Animation) {
       TIME(QString("Image Time"));
       TIME(QString("Start: %1").arg(QTime::currentTime().toString("hh:mm:ss")));
       imagetime.start();
    }

    for (int tile = 0; tile<maxTiles*maxTiles; tile++) {
        QElapsedTimer tiletime;
        tiletime.start();

        if (!progress.wasCanceled()) {

            QImage im(tileWidth, tileHeight, QImage::Format_ARGB32);
            im.fill(Qt::black);

            // Added sleep of 10 ms so that CPU does not submit too much work to GPU
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));

            engine->renderTile(padding, time, maxSubframes, tileWidth, tileHeight, tile, maxTiles, &progress, &steps, &im, totalTime);

            if (padding>0.0)  {
                int nw = (int)(tileWidth / (1.0 + padding));
                int nh = (int)(tileHeight / (1.0 + padding));
                int ox = (tileWidth-nw)/2;
                int oy = (tileHeight-nh)/2;
                im = im.copy(ox,oy,nw,nh);
            }

            if (tileWidth * maxTiles < 32769 && tileHeight * maxTiles < 32769) {
                cachedTileImages.insert(tile,im);
            }

            // display scaled tiles
            float wScaleFactor = enginePixmap.width() / maxTiles;
            float hScaleFactor = enginePixmap.height() / maxTiles;
            int dx = (tile / maxTiles);
            int dy = (maxTiles-1)-(tile % maxTiles);
            QRect source ( 0, 0, wScaleFactor, hScaleFactor );
            QRect target( (dx * wScaleFactor), (dy * hScaleFactor), wScaleFactor, hScaleFactor );
            im = im.scaled(wScaleFactor, hScaleFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            // render scaled tiles into the overlay pixmap
            QPainter painter2( &enginePixmap );
            painter2.drawImage ( target, im, source );
            painter2.end();

            // update the GL area overlay
            engineOverlay->setPixmap(enginePixmap);
        } else {
            stopScript();
            tile = maxTiles*maxTiles;
        }

        if ((maxTiles * maxTiles) == 1) {
            engine->tileAVG = tiletime.elapsed();
        } else {
            engine->tileAVG += tiletime.elapsed();
        }
    }

    if (!scriptRunning() && engine->getState() != DisplayWidget::Animation) {
        QTime thisTime = QTime::fromMSecsSinceStartOfDay(imagetime.elapsed());
        TIME(QString("Elapsed: %1").arg(thisTime.toString("hh:mm:ss")) );
        TIME(QString("Stop: %1").arg(QTime::currentTime().toString("hh:mm:ss")));
    }
}

bool MainWindow::writeTiledEXR(int maxTiles, int tileWidth, int tileHeight, int padding, int maxSubframes, int &steps, QString name, QProgressDialog &progress, QVector<QImage> &cachedTileImages, QElapsedTimer &totalTime, double time)
{
    QElapsedTimer imagetime;

    if (!scriptRunning() && engine->getState() != DisplayWidget::Animation) {
       TIME(QString("Image Time"));
       TIME(QString("Start: %1").arg(QTime::currentTime().toString("hh:mm:ss")));
       imagetime.start();
    }
    //
    // Write a tiled image with one level using a tile-sized framebuffer.
    //

    bool d2a = engine->wantsDepthToAlpha();

    Header header (maxTiles*tileWidth, maxTiles*tileHeight);
    header.channels().insert ("R", Channel (Imf::FLOAT));
    header.channels().insert ("G", Channel (Imf::FLOAT));
    header.channels().insert ("B", Channel (Imf::FLOAT));
    if(d2a)
        header.channels().insert ("Z", Channel (Imf::FLOAT));
    else
        header.channels().insert ("A", Channel (Imf::FLOAT));

    header.setTileDescription (TileDescription (tileWidth, tileHeight, ONE_LEVEL));

    TiledOutputFile out(name.toLatin1(), header);

    Array2D<RGBAFLOAT> pixels (tileHeight, tileWidth);

    FrameBuffer frameBuffer;
    frameBuffer.insert ("R", Slice (Imf::FLOAT, (char *) &pixels[0][0].r, sizeof (pixels[0][0]) * 1, sizeof (pixels[0][0]) * tileWidth, 1, 1, 0.0, true, true));
    frameBuffer.insert ("G", Slice (Imf::FLOAT, (char *) &pixels[0][0].g, sizeof (pixels[0][0]) * 1, sizeof (pixels[0][0]) * tileWidth, 1, 1, 0.0, true, true));
    frameBuffer.insert ("B", Slice (Imf::FLOAT, (char *) &pixels[0][0].b, sizeof (pixels[0][0]) * 1, sizeof (pixels[0][0]) * tileWidth, 1, 1, 0.0, true, true));
    if(d2a)
        frameBuffer.insert ("Z", Slice (Imf::FLOAT, (char *) &pixels[0][0].a, sizeof (pixels[0][0]) * 1, sizeof (pixels[0][0]) * tileWidth, 1, 1, 0.0, true, true));
    else
        frameBuffer.insert ("A", Slice (Imf::FLOAT, (char *) &pixels[0][0].a, sizeof (pixels[0][0]) * 1, sizeof (pixels[0][0]) * tileWidth, 1, 1, 0.0, true, true));


    for (int tile = 0; tile<maxTiles*maxTiles; tile++) {

        QElapsedTimer tiletime;
        tiletime.start();

        if (!progress.wasCanceled()) {

            QImage im(tileWidth, tileHeight, QImage::Format_ARGB32);
            im.fill(Qt::black);

            // Added sleep of 10 millisecs so that CPU does not submit too much work to GPU
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));

            engine->renderTile(padding, time, maxSubframes, tileWidth, tileHeight, tile, maxTiles, &progress, &steps, &im, totalTime);

            if (padding>0.0)  {
                int w = im.width();
                int h = im.height();
                int nw = (int)(w / (1.0 + padding));
                int nh = (int)(h / (1.0 + padding));
                int ox = (w-nw)/2;
                int oy = (h-nh)/2;
                im = im.copy(ox,oy,nw,nh);
            }

            if (tileWidth * maxTiles < 32769 && tileHeight * maxTiles < 32769) {
                cachedTileImages.append(im);
            }

            int dx = (tile / maxTiles);
            int dy = (maxTiles-1)-(tile % maxTiles);

            if(engine->getRGBAFtile( pixels, tileWidth, tileHeight )) {

                out.setFrameBuffer (frameBuffer);
                out.writeTile (dx, dy);

                if(!runningScript) {
                    // display scaled tiles
                    float wScaleFactor = enginePixmap.width() / maxTiles;
                    float hScaleFactor = enginePixmap.height() / maxTiles;
                    int dx = (tile / maxTiles);
                    int dy = (maxTiles-1)-(tile % maxTiles);
                    QRect source ( 0, 0, wScaleFactor, hScaleFactor );
                    QRect target( (dx * wScaleFactor), (dy * hScaleFactor), wScaleFactor, hScaleFactor );
                    im = im.scaled(wScaleFactor, hScaleFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    // render scaled tiles into the overlay pixmap
                    QPainter painter2( &enginePixmap );
                    painter2.drawImage ( target, im, source );
                    painter2.end();
                    // update the GL area overlay
                    engineOverlay->setPixmap(enginePixmap);
                }
            }
        } else {
            stopScript();
            tile = maxTiles*maxTiles;
        }
        if ((maxTiles * maxTiles) == 1) {
            engine->tileAVG = tiletime.elapsed();
        } else {
            engine->tileAVG += tiletime.elapsed();
        }
    }

    if (!scriptRunning() && engine->getState() != DisplayWidget::Animation) {
        QTime thisTime = QTime::fromMSecsSinceStartOfDay(imagetime.elapsed());
        TIME(QString("Elapsed mins: %1").arg(thisTime.toString("hh:mm:ss")) );
        TIME(QString("Stop: %1").arg(QTime::currentTime().toString("hh:mm:ss")));
    }

    engine->update();

    return out.isValidLevel(0,0);

}

void MainWindow::tileBasedRender()
{

    int tmpX = bufferXSpinBox->value();
    int tmpY = bufferYSpinBox->value();
    if(tileSizeFromScreen && !runningScript) {
        settings.setValue("tilewidth",tmpX);
        settings.setValue("tileheight",tmpY);
        settings.setValue("lockedAspect", lockedAspect);
        settings.sync();
    }
    OutputDialog od(this);
retry:
    if(tileSizeFromScreen && !runningScript) {
        od.tileXSizeChanged(tmpX);
        od.tileYSizeChanged(tmpY);
        od.lockAspect(lockedAspect);
    }

    od.setAspectLock(lockedAspect);

    od.setMaxTime(timeMax);
    bool runFromScript = runningScript;

    if(!runFromScript) {
        if (od.exec() != QDialog::Accepted) {
            return;
        }
    } else {
        od.readOutputSettings();
    }

    // get tile size from output dialog
    int tileWidth = od.getTileWidth();
    int tileHeight = od.getTileHeight();

    QRect r = QRect(QPoint(0, 0), QSize(-1, -1));
    // calculate an offset if aspect ratios don't match
    if((double)tmpX/(double)tmpY != (double)tileWidth/(double)tileHeight) {
        int tw = tileWidth;
        int th = tileHeight;
        double aspect = (double)tw/(double)th;
        // adjust to X
        double fw = (double)tw/(double)tmpX;
        if (fw!=1.0) {
            tw = tmpX;
            th = tmpX/aspect;
        }
        // adjust to Y if needed
        double fh = (double)th/(double)tmpY;
        if (fh>1.0) {
            tw = tmpY*aspect;
            th = tmpY;
        }

        r = QRect( QPoint((tmpX-tw)/2, (tmpY-th)/2), QSize(tw, th) );
    }

    // before clearing the buffer grab a copy for render progress background
    enginePixmap = engine->grab(r);

    // no locked aspect calculations during tile render
    bool ta = lockedAspect;
    lockAspect(false);
    bufferXSpinBox->setValue(tileWidth);
    bufferYSpinBox->setValue(tileHeight);
    // save the source files if required
    if ( (od.doSaveFragment() || od.doAnimation()) && !od.preview()) {
        QString fileName = od.getFragmentFileName();
        logger->getListWidget()->clear();
        if (tabBar->currentIndex() == -1) {
            WARNING(tr("No open tab"));
            return;
        }
        QString inputText = getTextEdit()->toPlainText();
        readSettings();
        Preprocessor p(&fileManager);

        QString file = tabInfo[tabBar->currentIndex()].filename;
        FragmentSource fs = p.createAutosaveFragment(inputText,file);
        // if the first line is the #version preprocessor command it must stay as
        // the first line
        QString firstLine = fs.source[0].trimmed() + "\n";
        if (firstLine.startsWith("#version")) {
            fs.source.removeAt(0);
            fs.lines.removeAt(0);
        } else {
            firstLine = "";
        }

        QString prepend = firstLine + tr("// Output generated from file: ") + file + "\n";
        prepend += tr("// Created: ") + QDateTime::currentDateTime().toString() + "\n";
        QString append = "\n\n#preset Default\n" + variableEditor->getSettings() + "\n";

        append += "#endpreset\n\n";

        QString final = prepend + fs.getText() + append;

        QString f = od.getFileName();
        QDir oDir(QFileInfo(f).absolutePath());
        QString subdirName = od.getFolderName();
        bool overWrite = false;
        if (oDir.exists(subdirName)){
            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText( QString("%1<br>Already exists!").arg(subdirName) );
            msgBox.setInformativeText(tr("Do you want to use it? <br><br>This will overwrite any existing files!"));
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Ok);
            int ret = msgBox.exec();
            switch (ret) {
            case QMessageBox::Ok:
                overWrite = true;
                break;
            case QMessageBox::Cancel:
                goto retry;
                break;
            default:
                return;
            }
        } else
        if (!oDir.mkdir(subdirName)) {

            QMessageBox::warning(this, tr("Fragmentarium"), tr("Could not create directory %1:\n.").arg(oDir.filePath(subdirName)));
            goto retry;
        }
        subdirName = oDir.filePath(subdirName); // full name

        try {
            if(od.doSaveFragment()) {
                QFile fileStream(subdirName + "/" + fileName);
                if (!fileStream.open(QFile::WriteOnly | QFile::Text)) {
                    QMessageBox::warning(this, tr("Fragmentarium"), tr("Cannot write file %1:\n%2.").arg(fileName).arg(fileStream.errorString()));
                    return;
                }

                if(od.doReleaseFiles()) {
                // if releaseFiles flag copy all textures to frag dir and adjust for local referencing
                // can't assume that local dist files have not been added or altered by user so copy all
                // the texture references in currently running FragmentSource are full path names
                // test if actual reference in final output is full path, truncate to filename only
                // remap texture file references in final output to local before saving frag

                    QMapIterator<QString, QString> it( engine->getFragmentSource()->textures );

                    while( it.hasNext() ) {
                        it.next();
                        QString localReference = it.value().split("/").last();
                        // there may be other textures still in the cache so test final for filename
                        // only copy textures that are actually used
                        if(final.contains(localReference)) {
                            // remap full path to local reference
                            final.replace(it.value(), localReference);
                            QString newFullName = subdirName + "/" + localReference;
                            QFile(it.value()).copy(newFullName);
                            if(QFile::exists(newFullName) && overWrite) {
                                if (!QFile::remove(newFullName)) {
                                    QMessageBox::warning(
                                        this, tr("Fragmentarium"), tr("Could not remove existing:\n'%1'").arg(newFullName));
                                }
                            }
                            if (!QFile::copy(it.value(),newFullName)) {
                                QMessageBox::warning(
                                    this, tr("Fragmentarium"), tr("Could not copy dependency:\n'%1' to \n'%2'.").arg(it.value()).arg(newFullName));
                            }
                        }
                    }
                }
                // save fragment source
                QTextStream out(&fileStream);
                out << final;

                INFO(tr("Saved fragment + settings as: ") + subdirName + "/" + fileName);

                if(includeWithAutoSave) {
                    // Copy files.
                    QStringList ll = p.getDependencies();
                    foreach (QString from, ll) {
                        QString to(QDir(subdirName).absoluteFilePath(QFileInfo(from).fileName()));
                        if(QFile::exists(to) && overWrite) {
                            if (!QFile::remove(to)) {
                                QMessageBox::warning(
                                    this, tr("Fragmentarium"), tr("Could not remove existing:\n'%1'").arg(to));
                            }
                        }
                        if (!QFile::copy(from,to)) {
                            QMessageBox::warning(
                                this, tr("Fragmentarium"), tr("Could not copy dependency:\n'%1' to \n'%2'.").arg(from).arg(to));
                        }
                    }
                }
            }
        } catch (Exception& e) {
            WARNING(e.getMessage());
        }
    }

    DisplayWidget::DrawingState oldState = engine->getState();
    engine->setState(DisplayWidget::Tiled);
    engine->clearTileBuffer();
    engine->tilesCount = 0;

    double padding = od.getPadding();
    int maxSubframes = od.getSubFrames();
    QString fileName = od.getFileName();
    int fps = od.getFPS();
    int maxTime = od.getMaxTime();
    bool preview = od.preview();
    int startTime = od.doAnimation() ? od.startAtFrame() : 0;
    int endTime = od.doAnimation() ? od.endAtFrame()+1 : 0;
    int maxTiles = od.getTiles();
    int timeSteps = fps*maxTime;
    bool imageSaved = false;

    exrMode = fileName.endsWith(".exr", Qt::CaseInsensitive);
    engine->setEXRmode(exrMode);
    // check for excessive tile size and allow option to change it
    if ((tileWidth * maxTiles > 32768 || tileHeight * maxTiles > 32768) && !exrMode) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText( QString("%1x%2 %3").arg(tileWidth*maxTiles).arg(tileHeight*maxTiles).arg(tr("is too large!\nMust be less than 32769x32769")));
        msgBox.setInformativeText(tr("Do you want to try again?"));
        msgBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Retry);
        int ret = msgBox.exec();
        switch (ret) {
        case QMessageBox::Retry:
            goto retry;
            break;
        case QMessageBox::Cancel:
            return;
            break;
        default:
            // should never be reached
            break;
        }
    }

    if(startTime == endTime) {timeSteps = 1; endTime += 1; }

    if (timeSteps==0) {
        startTime = getFrame();
        timeSteps = startTime+1;
    } else if (endTime > startTime) {
        timeSteps = endTime;
    }

    int totalSteps= timeSteps*maxTiles*maxTiles*maxSubframes;
    int steps = startTime*maxTiles*maxTiles*maxSubframes;

    engine->tileAVG = 0;
    engine->renderAVG = 0;
    engine->renderETA =  "";
    engine->renderToFrame = endTime-1;
    // create our progress dialog
    QProgressDialog progress(tr("Rendering"), tr("Abort"), 0, totalSteps, this);
    progress.setMinimumDuration(1000);
    progress.setValue ( 0 );
    progress.move((width() - progress.width()) / 2, (height() - progress.height()) / 2);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(steps);
    QLabel *lab;
    lab = new QLabel();
    lab->setTextFormat(Qt::RichText);
    lab->setAlignment(Qt::AlignmentFlag::AlignLeft);
    progress.setLabel(lab);
    progress.resize(400, 180);

    QElapsedTimer totalTime;
    totalTime.start();

    // create an overlay using enginePixmap as background
    engineOverlay = new QLabel();
    engineOverlay->setPixmap(enginePixmap);
    // hide the GL area and add our image progress overlay
    QList<int> splitSizes = splitter->sizes();
    splitSizes << 0;
    splitter->insertWidget(1,engineOverlay);
    splitter->setSizes(splitSizes);
    int handleWidth = splitter->handleWidth();
    splitter->setHandleWidth(handleWidth/2);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // render tiles and update progress
    for (int timeStep = startTime; timeStep<timeSteps ; timeStep++) {
        double time = (double)timeStep/(double)fps;

        if (progress.wasCanceled() || (runFromScript != runningScript)) {
            break;
        }
        if((totalSteps/maxTiles/maxTiles/maxSubframes) > 1) {
            if(variableEditor->hasEasing()) {
                engine->updateEasingCurves( timeStep );
            }

            if ( variableEditor->hasKeyFrames() ) {
                if (engine->eyeSpline != nullptr) {
                    int index = timeStep+1;
                    glm::dvec3 e = engine->eyeSpline->getSplinePoint(index);
                    glm::dvec3 t = engine->targetSpline->getSplinePoint(index);
                    glm::dvec3 u = engine->upSpline->getSplinePoint(index);
                    glm::dvec3 zero = glm::dvec3(0.0,0.0,0.0);
                    if( e!=zero && t!=zero && u!=zero ) {
                        setCameraSettings( e,t,u );
                    }
                }
            }
        }

        QVector<QImage> cachedTileImages;

        QString name = fileName; // prevent double numbering in file name when under script control
        if (!fileName.contains(QRegExp(".[0-9]{5,7}."))) {
            name=makeImgFileName(timeStep, timeSteps, fileName);
        }

        if (od.doSaveFragment() || od.doAnimation()) {
            QString subdirName = od.getFolderName();
            QDir filedir ( QFileInfo ( subdirName ).absolutePath() );

            if ( !filedir.exists() ) {
                filedir.mkdir ( QFileInfo ( subdirName ).absolutePath() );
              }

            name = ( QFileInfo ( name ).absolutePath() + "/" +
                     subdirName + "/" + ( od.doAnimation() ?"Images/": "" ) +
                     QFileInfo ( name ).fileName() );

            QDir imgdir ( QFileInfo ( name ).absolutePath() );
            if ( !imgdir.exists() ) {
                imgdir.mkdir ( QFileInfo ( name ).absolutePath() );
          }
        }

        QElapsedTimer frametime;
        frametime.start();

        bool pause = pausePlay;
        stop();

        statusBar()->showMessage ( QString ( "Rendering: %1" ).arg ( name ) );

        if(exrMode && !preview) {
            imageSaved = writeTiledEXR(maxTiles, tileWidth, tileHeight, padding, maxSubframes, steps, name, progress, cachedTileImages, totalTime, time);
        } else
            renderTiled(maxTiles, tileWidth, tileHeight, padding, maxSubframes, steps, progress, cachedTileImages, totalTime, time);

        pause ? stop() : play();

        engine->tileAVG /= maxTiles*maxTiles;
        if ((maxTiles * maxTiles) == 1) {
            engine->renderAVG = frametime.elapsed();
        } else {
            engine->renderAVG += frametime.elapsed();
        }
        // FIXME for sub-set ETA
//         if(isFirst) {
//             totalTime = totalTime.addMSecs( -(startTime*frametime.elapsed()) );
//             isFirst = false;
//         }
        // Now assemble image
        if (!progress.wasCanceled() && (!exrMode || (preview && tileWidth * maxTiles < 32769 && tileHeight * maxTiles < 32769))) {
            int w = cachedTileImages[0].width();
            int h = cachedTileImages[0].height();
            QImage finalImage(w * maxTiles, h * maxTiles, cachedTileImages[0].format());
            // There IS a Qt function to copy entire images!
            QPainter painter(&finalImage);
            for (int i = 0; i < maxTiles*maxTiles; i++) {
                int dx = (i / maxTiles);
                int dy = (maxTiles-1)-(i % maxTiles);
                int xoff = dx*w;
                int yoff = dy*h;
                QRect target(xoff, yoff, w, h);
                QRect source(0, 0, w, h);
                painter.drawImage(target, cachedTileImages[i], source);
            }

            INFO(QString("Created combined image (%1,%2)").arg(w * maxTiles).arg(h * maxTiles));

            if (preview) {
                static QDialog* qd;
                /// prevent multiple previews
                if (findChild<QDialog *>("PREVIEW") != 0) {
                    qd = findChild<QDialog*>("PREVIEW");
                    qd->close();
                    qd->~QDialog();
                }
                qd = new QDialog(this);
                qd->setObjectName(QString::fromUtf8("PREVIEW"));

                QVBoxLayout *l = new QVBoxLayout(qd);

                QLabel* label = new QLabel(qd);
                label->setObjectName(QString::fromUtf8("previewImage"));
                label->setPixmap(QPixmap::fromImage(finalImage));

                QScrollArea *scrollArea = new QScrollArea;
                scrollArea->setBackgroundRole(QPalette::Dark);
                scrollArea->setWidget(label);
                l->addWidget(scrollArea);

                QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
                connect(buttonBox, SIGNAL(rejected()), qd, SLOT(reject()));
                connect(buttonBox, SIGNAL(accepted()), this, SLOT(savePreview()));
                l->addWidget(buttonBox);

                qd->setLayout(l);
                qd->show();

            } else if (!exrMode) {
              finalImage.setText("frAg", variableEditor->getSettings());
              imageSaved=finalImage.save(name);
            }
            engineOverlay->update();
        }
        if(!preview) {
            if(imageSaved && !progress.wasCanceled()) {
                INFO(tr("Saved file : ") + name);
            } else {
                WARNING(tr("Render canceled! : ") + name + tr(" not saved!"));
            }
        }
    }

    QApplication::restoreOverrideCursor();

    delete engineOverlay;

    splitter->setHandleWidth(handleWidth);
    splitter->setSizes(splitSizes);

    engine->tilesCount = 0;
    engine->setState(oldState);

    progress.setValue(totalSteps);

    if (preview || progress.wasCanceled()) {
        engine->requireRedraw(true);
    }

    bufferXSpinBox->setValue(tmpX);
    bufferYSpinBox->setValue(tmpY);

    engine->clearTileBuffer();

    engine->updateBuffers();

    // reset locked aspect status
    lockAspect(ta);
}

void MainWindow::savePreview()
{

    QDialog *qd;
    if (findChild<QDialog *>("PREVIEW") != 0) {
        qd = findChild<QDialog*>("PREVIEW");

        QStringList extensions;
        QList<QByteArray> a = QImageWriter::supportedImageFormats();
        foreach ( QByteArray s, a ) {
            extensions.append ( QString( "%1.%2" ).arg("*").arg( QString(s) ) );
        }
        QString ext = QString(tr("Images (")) + extensions.join ( " " ) + tr(")");

        QString fn;
        fn = QFileDialog::getSaveFileName(qd, tr("Save preview image..."), "preview.png", ext);
        if(!fn.isEmpty()) {
            QLabel *label = qd->findChild<QLabel *>("previewImage");
            if (label != nullptr) {
                QImage img = label->pixmap(Qt::ReturnByValue).toImage();
                img.setText("frAg", variableEditor->getSettings());
                img.save(fn);
                qd->close();
                INFO(tr("Saved file : ") + fn);
            }
        }
    }
}

void MainWindow::pasteSelected()
{

    QString settings = getTextEdit()->textCursor().selectedText();
    // Note: If the selection obtained from an editor spans a line break,
    // the text will contain a Unicode U+2029 paragraph separator character
    // instead of a newline \n character. Use QString::replace() to replace these
    // characters with newlines
    settings = settings.replace(QChar::ParagraphSeparator,"\n");
    variableEditor->setSettings(settings);
    INFO(tr("Pasted selected settings"));
}

void MainWindow::saveParameters()
{

    QString filter = tr("Fragment Parameters (*.fragparams);;All Files (*.*)");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), "", filter);

    saveParameters(fileName);

}

void MainWindow::saveParameters(const QString fileName)
{

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fragmentarium"), tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }

    QTextStream out(&file);

    out << variableEditor->getSettings();
    INFO(tr("Settings saved to file"));
}

void MainWindow::loadParameters(const QString fileName)
{

    QFile file(fileName);
    if (fileName.toLower().endsWith(".png") && file.exists()) {
      variableEditor->setSettings(QImage(fileName).text("frAg"));
      return;
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fragmentarium"), tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QString settings = in.readAll();
    variableEditor->setSettings(settings);
    INFO(tr("Settings loaded from file") );
}

void MainWindow::loadParameters()
{

    QString filter = tr("Fragment Parameters (*.fragparams);;All Files (*.*)");
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load"), "", filter);
    if (fileName.isEmpty()) {
        return;
    }

    loadParameters(fileName);
}

void MainWindow::createToolBars()
{

    fileToolBar = addToolBar(tr("File Toolbar"));
    fileToolBar->addAction(new2DAction);
    fileToolBar->addAction(new3DAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    fileToolBar->addAction(saveAsAction);
    fileToolBar->setObjectName(QString::fromUtf8("FileToolbar"));

    settings.value("showFileToolbar").toBool() ? fileToolBar->show() : fileToolBar->hide();

    editToolBar = addToolBar(tr("Edit Toolbar"));
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    editToolBar->setObjectName(QString::fromUtf8("EditToolbar"));

    settings.value("showEditToolbar").toBool() ? editToolBar->show() : editToolBar->hide();

    bufferToolBar = addToolBar(tr("Buffer Dimensions"));
    bufferToolBar->addWidget(new QLabel(tr("Buffer Size. X: "), this));

    bufferXSpinBox = new QSpinBox(bufferToolBar);
    bufferXSpinBox->setRange(1,4096);
    bufferXSpinBox->setSingleStep(1);
    bufferXSpinBox->setEnabled(false);
    connect(bufferXSpinBox, SIGNAL(valueChanged(int)), this, SLOT(bufferXSpinBoxChanged(int)));
    connect(bufferXSpinBox, SIGNAL(editingFinished()), this, SLOT(bufferSizeXChanged()));
    bufferToolBar->addWidget(bufferXSpinBox);

    aspectLock = new QPushButton(bufferToolBar);
    aspectLock->setObjectName("lockbutton");
    aspectLock->setFlat(true);
    aspectLock->setStyleSheet("* {background: none; border: none; outline: none;}");
    aspectLock->setIcon(QIcon(":/Icons/padlockb.png"));
    aspectLock->setFixedSize(12,18);
    aspectLock->setCheckable(true);
    aspectLock->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    connect(aspectLock, SIGNAL(toggled(bool)), this, SLOT(lockAspect(bool)));
    bufferToolBar->addWidget(aspectLock);

    bufferToolBar->addWidget(new QLabel(tr("Y: "), this));
    bufferYSpinBox = new QSpinBox(bufferToolBar);
    bufferYSpinBox->setRange(1,4096);
    bufferYSpinBox->setSingleStep(1);
    bufferYSpinBox->setEnabled(false);
    connect(bufferYSpinBox, SIGNAL(valueChanged(int)), this, SLOT(bufferYSpinBoxChanged(int)));
    connect(bufferYSpinBox, SIGNAL(editingFinished()), this, SLOT(bufferSizeYChanged()));
    bufferSizeControl = new QPushButton(tr("Lock to window size"), bufferToolBar);
    QMenu *menu = new QMenu(bufferToolBar);
    bufferAction1 = menu->addAction(tr("Lock to window size"));
    menu->addSeparator();
    bufferActionCustom = menu->addAction(tr("Custom size"));
    bufferSizeControl->setMenu(menu);

    connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(bufferActionChanged(QAction *)));

    bufferToolBar->addWidget(bufferYSpinBox);
    bufferToolBar->addWidget(bufferSizeControl);

    bufferToolBar->setObjectName(QString::fromUtf8("BufferDimensions"));

    renderToolBar = addToolBar(tr("Render Toolbar"));
    renderToolBar->addAction(renderAction);
    buildLabel = new QLabel(tr("Build"), renderToolBar);
    renderToolBar->addWidget(buildLabel);
    renderToolBar->setObjectName(QString::fromUtf8("RenderToolbar"));

    renderModeToolBar = addToolBar(tr("Rendering Mode"));

    progressiveButton = new QPushButton( tr("Progressive"),renderModeToolBar);
    progressiveButton->setCheckable(true);
    progressiveButton->setChecked(true);
    animationButton = new QPushButton( tr("Animation"),renderModeToolBar);
    animationButton->setCheckable(true);

    QButtonGroup *bg = new QButtonGroup(renderModeToolBar);
    bg->addButton(progressiveButton);
    bg->addButton(animationButton);

    connect(progressiveButton, SIGNAL(clicked()), this, SLOT(renderModeChanged()));
    connect(animationButton, SIGNAL(clicked()), this, SLOT(renderModeChanged()));

    renderModeToolBar->addWidget(progressiveButton);
    renderModeToolBar->addWidget(animationButton);

    rewindAction = new QAction(QIcon(":/Icons/player_rew.png"), tr("Rewind"), this);
    rewindAction->setShortcut(tr("F10"));
    rewindAction->setStatusTip(tr("Rewinds animation."));
    connect(rewindAction, SIGNAL(triggered()), this, SLOT(rewind()));

    playAction = new QAction(QIcon(":/Icons/player_play.png"), tr("Start"), this);
    playAction->setShortcut(tr("F11"));
    playAction->setStatusTip(tr("Starts animation or subframe rendering."));
    connect(playAction, SIGNAL(triggered()), this, SLOT(play()));
    playAction->setEnabled(false);

    stopAction = new QAction(QIcon(":/Icons/player_stop.png"), tr("Stop"), this);
    stopAction->setShortcut(tr("F12"));
    stopAction->setStatusTip(tr("Stops animation or subframe rendering."));
    connect(stopAction, SIGNAL(triggered()), this, SLOT(stop()));
    stopAction->setEnabled(true);

    renderModeToolBar->addAction(rewindAction);
    renderModeToolBar->addAction(playAction);
    renderModeToolBar->addAction(stopAction);

    subframeLabel = new QLabel(tr(" Subframe Max: "), renderModeToolBar);
    renderModeToolBar->addWidget(subframeLabel);
    frameSpinBox = new QSpinBox(renderModeToolBar);
    frameSpinBox->setRange(0,10000);
    frameSpinBox->setValue( settings.value("maxSubframes", 10).toInt() );
    frameSpinBox->setSingleStep(5);

    connect(frameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(maxSubSamplesChanged(int)));

    renderModeToolBar->addWidget(frameSpinBox);

    frameLabel = new QLabel(tr(" 0 rendered."), renderModeToolBar);
    renderModeToolBar->addWidget(frameLabel);
    renderModeToolBar->setObjectName(QString::fromUtf8("RenderingMode"));

    addToolBarBreak();
    timeToolBar = addToolBar(tr("Time"));

    timeLabel = new QLabel(tr("Time: 0s "), renderModeToolBar);
    timeLabel->setFixedWidth(100);
    timeToolBar->addWidget(timeLabel);

    timeSlider = new QSlider(Qt::Horizontal, this);
    timeSlider->setMinimum(1);
    timeSlider->setValue(1);
    timeSlider->setMinimumSize(160,20);
    timeSlider->setMaximumSize(2048,20);
    timeSlider->setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

    timeSlider->setMaximum( 10 * renderFPS); // seconds * frames per second = length of anim
    connect(timeSlider, SIGNAL(valueChanged(int)), this, SLOT(timeChanged(int)));
    timeToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(timeToolBar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(timeLineRequest(QPoint)));

    timeToolBar->addWidget(timeSlider);

    timeMaxSpinBox = new QSpinBox(renderModeToolBar);
    timeMaxSpinBox->setRange(0,10000);
    timeMaxSpinBox->setValue(timeMax);
    connect(timeMaxSpinBox, SIGNAL(valueChanged(int)), this, SLOT(timeMaxChanged(int)));
    timeToolBar->addWidget(timeMaxSpinBox);
    timeToolBar->setObjectName(QString::fromUtf8("Time"));

    timeMaxChanged(timeMaxSpinBox->value()); // call when timeMax changes to setup the time slider for frame accuracy
    autoFocusEnabled = false;

}

void MainWindow::setTimeSliderValue(int value)
{

    // timeslider max = animation length in frames set here to keep in sync with framerate/duration changes
    timeSlider->setMaximum(timeMaxSpinBox->value()*renderFPS);

    if (value >= timeMaxSpinBox->value() * renderFPS) {
        if (wantLoopPlay && engine->getState() == DisplayWidget::Animation && !pausePlay) {
            rewind();
            play();
            return;
        }
        playAction->setEnabled(true);
        stopAction->setEnabled(false);
        lastStoredTime = value;
        engine->setContinuous(false);
        value = timeMaxSpinBox->value() * renderFPS;
    }
    timeSlider->setValue(value);
}

void MainWindow::bufferActionChanged(QAction *action)
{
    bufferSizeControl->setText(action->text());
    if (action == bufferAction1) {
        lockedToWindowSize = true;
        lockAspect(false);
        // because there is no layout available in a QSplitter we have to resize manually here
        engine->resize( splitter->width() - splitter->handle(1)->width() - (splitter->handle(1)->pos().x()-1) , splitter->handle(1)->height() );
    } else if (action == bufferActionCustom) {
        lockedToWindowSize = false;
        lockAspect(lockedAspect);
        engine->resize(bufferXSpinBox->value(),bufferYSpinBox->value());
    }

    bufferYSpinBox->setEnabled(!lockedToWindowSize);
    bufferXSpinBox->setEnabled(!lockedToWindowSize);
    aspectLock->setEnabled(!lockedToWindowSize);
}

void MainWindow::timeLineRequest(QPoint p)
{

    Q_UNUSED(p)

    TimeLineDialog *timeDialog = new TimeLineDialog(this, keyframeMap);
    timeDialog->setWindowTitle(strippedName(tabInfo[tabBar->currentIndex()].filename));
    timeDialog->exec();

};

void MainWindow::videoEncoderRequest()
{

    VideoDialog *vDialog = new VideoDialog(this);
    vDialog->exec();

};

void MainWindow::timeChanged(int value)
{

    Q_UNUSED(value)
    lastTime->restart();
    lastStoredTime = getTimeSliderValue();
    getTime();
    engine->requireRedraw(true);
}

void MainWindow::timeMaxChanged(int value)
{
    // so the render output dialog picks up the change immediately
    settings.setValue("timeMax", value);

    lastTime->restart();
    lastStoredTime = getTimeSliderValue();
    getTime();
    timeSlider->setMaximum(value * renderFPS);       // timeslider max = animation length in frames
    timeSlider->setSingleStep(1);        // should be one frame
    timeSlider->setPageStep(renderFPS); // should be one second
    timeMax = value;
    if (variableEditor->hasKeyFrames()) {
        initKeyFrameControl();
    }
}

void MainWindow::rewind()
{

    lastTime->restart();
    lastStoredTime = 0;
    getTime();
    if(variableEditor->hasEasing()) engine->updateEasingCurves(0);
    engine->requireRedraw(true);
    engine->update();
}

void MainWindow::play()
{

    playAction->setEnabled(false);
    stopAction->setEnabled(true);
    lastTime->restart();
    getTime();
    pausePlay=false;
    engine->setContinuous(true);
}

void MainWindow::stop()
{
    playAction->setEnabled(true);
    stopAction->setEnabled(false);
    lastStoredTime = getTime();
    engine->setContinuous(false);

    if (engine->getState() == DisplayWidget::Animation) {
        INFO( tr("Stopping: last stored time set to %1").arg((double)lastStoredTime / renderFPS) );
    }

    getTime();
    pausePlay=true;
}

void MainWindow::maxSubSamplesChanged(int value)
{

    engine->setMaxSubFrames(value);
}

void MainWindow::setSubframeMax(int i)
{

    frameSpinBox->setValue(i);
}

double MainWindow::getTime()
{

    DisplayWidget::DrawingState state = engine->getState();

    int time = 1;
    if (!engine->isContinuous() || state == DisplayWidget::Tiled) {
        // The engine is not in 'running' mode. Return last stored paused time.
        time = lastStoredTime;
    } else {
        if (state == DisplayWidget::Progressive) {
            time = lastStoredTime;
        } else if (state == DisplayWidget::Animation) {
            time = lastStoredTime + ((lastTime->elapsed()/1000.0)*renderFPS);
        }
    }
    int ct = time != 0 ? time/renderFPS : 0;
    timeLabel->setText(
        QString("%1 %2s:%3f ")
        .arg(tr("Time:"))
        .arg(ct, 3, 'g', -1, '0')
        .arg((((double)time / (double)renderFPS) - ct) * renderFPS, 2, 'g', -1, '0'));
    timeSlider->blockSignals(true);
    setTimeSliderValue(time);
    timeSlider->blockSignals(false);
    return time;
}

void MainWindow::renderModeChanged()
{

    engine->setMaxSubFrames(frameSpinBox->value());
    setFPS(-1);
    QObject* o = QObject::sender();
    lastStoredTime = getTime();
    if (o == nullptr || o == progressiveButton) {
        engine->requireRedraw(true);
        engine->setState(DisplayWidget::Progressive);
        getTime();
    } else if (o == animationButton) {
        engine->setState(DisplayWidget::Animation);
        lastTime->restart();
        getTime();
    }

    engine->setFocus();
    engine->setDisableRedraw(false);
}

void MainWindow::setSubFrameDisplay(int i)
{

    frameLabel->setText(QString(" %1 %2").arg(tr("Done")).arg(i));
}

void MainWindow::disableAllExcept(QWidget *w)
{

    disabledWidgets.clear();
    disabledWidgets = findChildren<QWidget *>("");
    while (w != nullptr) {
        disabledWidgets.removeAll(w);
        w=w->parentWidget();
    }

    foreach (QWidget *w, disabledWidgets) {
        w->setEnabled(false);
    }
    processGuiEvents();
}

void MainWindow::createStatusBar()
{

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{

    maxRecentFiles = settings.value("maxRecentFiles", 5).toInt();
    renderFPS = settings.value("fps", 25).toInt();
    timeMax = settings.value("timeMax", 10).toInt();
    wantGLPaths = settings.value("drawGLPaths", true).toBool();
    wantSplineOcclusion = settings.value("splineOcc", true).toBool();
    wantLineNumbers = settings.value("lineNumbers", true).toBool();
    wantLoopPlay = settings.value("loopPlay", true).toBool();
    editorStylesheet = settings.value("editorStylesheet", "font: 9pt Courier;").toString();
    variableEditor->updateGeometry();
    variableEditor->setSaveEasing(settings.value("saveEasing", true).toBool());
    fileManager.setIncludePaths(settings.value("includePaths", "Examples/Include;").toString().split(";"));
    loggingToFile = settings.value("logToFile", false).toBool();
    logFilePath = settings.value("logFilePath", "fragm.log").toString();
    maxLogFileSize = settings.value("maxLogFileSize", 125).toInt();
    fullPathInRecentFilesList = settings.value("fullPathInRecentFilesList", false).toBool();
    includeWithAutoSave = settings.value("includeWithAutoSave", false).toBool();
    useMimetypes = settings.value("useMimetypes", false).toBool();
    exrBinaryPath = settings.value("exrBinPaths", "/usr/bin;bin;").toString().split(";");
    supportProgramsBinaryPath = settings.value("supportProgramBinPaths", "/usr/bin;bin;").toString().split(";");
    editorTheme = settings.value("editorTheme", 0).toInt();
    guiStylesheet = settings.value("guiStylesheet", "").toString();
    lockedToWindowSize = settings.value("lockedToWindowSize", true).toBool();
    currentAspect = settings.value("currentAspect", 1.7778).toDouble();
    lockedAspect = settings.value("lockedAspect", false).toBool();
    tileSizeFromScreen = settings.value ( "tileSizeFromScreen", false ).toBool();
    gimpGradientsPaths = settings.value ( "gimpGradientsPaths", "/usr/share/gimp/2.0/gradients/;~/.config/GIMP/2.10/gradients/").toString();
    loopCameraPath = settings.value("loopCameraPath", false).toBool();
}

void MainWindow::writeSettings()
{

    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("splitterSizes", splitter->saveState());
    settings.setValue("fullScreenEnabled", fullScreenEnabled);
    settings.setValue("maxSubframes", getSubFrameMax());
    settings.setValue("maxRecentFiles", maxRecentFiles);
    settings.setValue("fps", renderFPS);
    settings.setValue("timeMax", timeMax);
    settings.setValue("drawGLPaths", wantGLPaths);
    settings.setValue("splineOcc", wantSplineOcclusion);
    settings.setValue("lineNumbers", wantLineNumbers);
    settings.setValue("loopPlay", wantLoopPlay);
    settings.setValue("editorStylesheet", editorStylesheet);

    QString ipaths = fileManager.getIncludePaths().join(";");
    if (fileManager.getIncludePaths().count() == 1) {
        ipaths += ";";
    }
    settings.setValue("includePaths", ipaths);

    QString ebpaths = exrBinaryPath.join(";");
    if (exrBinaryPath.count() == 1) {
        ebpaths += ";";
    }
    settings.setValue("exrBinPaths", ebpaths);

    QString sppaths = supportProgramsBinaryPath.join(";");
    if (supportProgramsBinaryPath.count() == 1) {
        sppaths += ";";
    }
    settings.setValue("supportProgramBinPaths", sppaths);

    settings.setValue("showFileToolbar", !fileToolBar->isHidden() );
    settings.setValue("showEditToolbar", !editToolBar->isHidden() );
    settings.setValue("fullPathInRecentFilesList", fullPathInRecentFilesList );
    settings.setValue("includeWithAutoSave", includeWithAutoSave );
    settings.setValue("useMimetypes", useMimetypes );
    settings.setValue("editorTheme", editorTheme);
    settings.setValue("guiStylesheet", guiStylesheet);

    settings.setValue("lockedToWindowSize", lockedToWindowSize);
    settings.setValue("lockedAspect", lockedAspect);
    settings.setValue("currentAspect", currentAspect);
    settings.setValue("loopCameraPath", loopCameraPath);

    QStringList openFiles;
    if (!tabInfo.isEmpty()) {
        for (auto &i : tabInfo) {
            openFiles << i.filename;
        }
    }

    settings.setValue("openFiles", openFiles);
    settings.setValue("tilewidth",bufferXSpinBox->value());
    settings.setValue("tileheight",bufferYSpinBox->value());
    settings.setValue("tileSizeFromScreen", tileSizeFromScreen);

    settings.setValue("gimpGradientsPaths", gimpGradientsPaths);

    settings.sync();

}

void MainWindow::openFile()
{

    QAction *action = qobject_cast<QAction *>(sender());
    if (action != nullptr) {
        loadFragFile(action->data().toString());
    } else {
        WARNING(tr("No data!"));
    }
}

void MainWindow::reloadFrag()
{

    int index = tabBar->currentIndex();
    if (index == -1) {
        WARNING(tr("No open tab"));
        return;
    }
    reloadFragFile( index );
}

void MainWindow::reloadFragFile( int index )
{

    QString filename = tabInfo[index].filename;
    TextEdit* te = getTextEdit();

    QFile file(filename);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            WARNING(tr("Cannot read file %1:\n%2.").arg(filename).arg(file.errorString()));
        } else {
            te->clear();
            QTextStream in(&file);
            QApplication::setOverrideCursor(Qt::WaitCursor);
            te->setPlainText(in.readAll());
            QApplication::restoreOverrideCursor();
            INFO(tr("Reloaded file: %1").arg(filename));
        }

}

/// fragWatch
void MainWindow::reloadFragFile( QString f )
{

    static bool first = true; // TODO: why this gets called twice ???

    bool autoLoad = QSettings().value("autoload", false).toBool();

    if (!autoLoad && first) {
        QString s = tr("It looks like the file: %1\n has been changed by another program.\nWould you like to reload it?").arg(f.split("/").last());
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(tr("Reload"),QMessageBox::AcceptRole);
        msgBox.addButton(tr("Ignore"),QMessageBox::RejectRole);
        msgBox.setText(s);
        first = false;
        autoLoad = msgBox.exec()==1 ? false : true;
    }

    if (autoLoad) {
        for (int i = 0; i < tabInfo.size(); i++) {
            if (tabInfo[i].filename == f) {
                tabBar->setCurrentIndex(i);
                if (!tabInfo[i].unsaved) { // won't autoload if the frag is changed in the editor
                    reloadFrag();
                    highlightBuildButton(true); // won't autoload again until this frag is built
                    processGuiEvents(); // make sure gui is updated

                } else {
                    QMessageBox::warning(this, tr("Fragmentarium"),
                             tr("Will not read file %1:\n%2.")
                             .arg(f.split("/").last())
                             .arg(tr("Conflict with changes in the editor!")));
                }
                break;
            }
        }
    }

    first = true;

}

void MainWindow::loadFragFile(const QString &fileName)
{
    // calling with nonexistant filename before test prevents crash
    // fi a non-quoted filename with an unescaped space will appear as 2 file
    // names, both are wrong the first appears as non frag the second appears as
    // frag but non-existent passing bogus name to insertTabPage() will cause it
    // to load the minimum default GLSL source so initializeFragment() gets valid
    // code later on
    TextEdit *te = insertTabPage(fileName);
    if(editorTheme) te->setTheme(editorTheme);
    te->highlightCurrentLine();
    processGuiEvents(); // make sure the widgets are there

    if (fileName.toLower().endsWith(".frag") && QFile(fileName).exists()) {
        DisplayWidget::DrawingState oldstate = engine->getState();
        engine->setState(DisplayWidget::Progressive);
        bool pp = pausePlay;
        stop();
        int sfmax = getSubFrameMax();
        setSubframeMax(1);

        if (QSettings().value("autorun", true).toBool()) {
            setRebuildStatus( true );
            variableEditor->resetUniforms(true); // set all values and initialize fragment
            setRebuildStatus( variableEditor->setDefault() ); // set vars with default preset
            if (rebuildRequired) {
                INFO(tr("Rebuilding to update uniform state..."));
                setRebuildStatus( initializeFragment() );
            }
            processGuiEvents();
        }
        setSubframeMax(sfmax);

        QSettings().setValue("isStarting", false);
        engine->setState(oldstate);
        pp?stop():play();
    } else if(scriptRunning()) {
        stopScript();    // file failed to load or doesn't exist
    }
}

bool MainWindow::saveFile(const QString &fileName)
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return false;
    }

    if(fragWatch->files().contains(fileName)) {
      fragWatch->removePath(fileName);
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fragmentarium"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << getTextEdit()->toPlainText();
    QApplication::restoreOverrideCursor();
    out.flush();

    tabInfo[tabBar->currentIndex()].hasBeenSavedOnce = true;
    tabInfo[tabBar->currentIndex()].unsaved = false;
    tabInfo[tabBar->currentIndex()].filename = fileName;

    // to update displayed name;
    QString tabTitle = QString("%1").arg(strippedName(fileName));
    tabBar->setTabText(tabBar->currentIndex(), tabTitle);
    setWindowTitle(QString("%1 - %2").arg(tabTitle).arg("Fragmentarium"));

    statusBar()->showMessage(tr("File saved"), 2000);
    setRecentFile(fileName);

    if(QFile(fileName).exists() && !fragWatch->files().contains(fileName)) {
        fragWatch->addPath(fileName);
    }

    return true;
}

QString MainWindow::strippedName(const QString &fullFileName)
{

    return QFileInfo(fullFileName).fileName();
}

void MainWindow::showPreprocessedScript()
{

    logger->getListWidget()->clear();
    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }
    INFO(tr("Showing preprocessed output in new tabs"));
    QString inputText = getTextEdit()->toPlainText();
    QString filename = tabInfo[tabBar->currentIndex()].filename;
    readSettings();

    try {
        FragmentSource fs = *(engine->getFragmentSource()); // point at the patched glsl
        insertTabPage("")->setPlainText(fs.getText());
        // Use a real name instead of "unnamed" suggested by FF user Sabine62 18/10/12
        QString fname = QString("FragmentShader_%1").arg(strippedName(filename));
        tabBar->setTabText(tabBar->currentIndex(), fname);
        tabInfo[tabBar->currentIndex()].filename = fname;
        if (fs.bufferShaderSource != nullptr) {
            // present the buffershader as well
            insertTabPage("")->setPlainText(fs.bufferShaderSource->getText());
            fname = QString("BufferShader_%1").arg(strippedName(filename));
            tabBar->setTabText(tabBar->currentIndex(), fname);
            tabInfo[tabBar->currentIndex()].filename = fname;
        }
    } catch (Exception& e) {
        WARNING(e.getMessage());
    }
}

void MainWindow::highlightBuildButton(bool value)
{
    if(qApp->styleSheet().isEmpty()) {
        QWidget* w = buildLabel->parentWidget();
        if (value) {
            QPalette pal = buildLabel->palette();
            pal.setColor(buildLabel->backgroundRole(), Qt::yellow);
            buildLabel->setPalette(pal);
            buildLabel->setAutoFillBackground(true);
            w->setPalette(pal);
            w->setAutoFillBackground(true);
        } else {
            buildLabel->setPalette(QApplication::palette(buildLabel));
            buildLabel->setAutoFillBackground(false);
            w->setPalette(QApplication::palette(w));
            w->setAutoFillBackground(false);
        }
    } else {
        if (value) {
            buildLabel->setStyleSheet("* {border: none; color: black; background: QRadialGradient(cx:0.5, cy:0.5, radius: 0.6, fx:0.5, fy:0.5, stop:0 rgb(255,255,0,100%), stop:1 rgb(255,255,0,0%)); }");
        } else {
            buildLabel->setStyleSheet("* {;}");
        }
    }
    setRebuildStatus(value);
}

void MainWindow::addToWatch(QStringList fileList)
{

    foreach(QString f, fileList) {
        if(QFile(f).exists() && !fragWatch->files().contains(f)) {
            fragWatch->addPaths(fileList);
        }
    }
}



bool MainWindow::initializeFragment()
{
    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return false;
    }
    // always rebuild when called by user otherwise only rebuild if source has changed
    if(sender() != nullptr && !sender()->objectName().isEmpty()) {
        if(sender()->objectName() == QString("renderAction")) {
            rebuildRequired=true;
        }
    }

    logger->getListWidget()->clear();

    // Show info first...clearTextureCache
    INFO(tr("Vendor: ") + engine->vendor + "\n" + tr("Renderer: ") + engine->renderer + "\n" + tr("GL Driver: ") + engine->glvers);
    // report the profile that was actually created in the engine
    int prof = engine->format().profile();
    if (prof == 1 || prof == 2) {
        INFO(QString(tr("Display using GL %1.%2 %3 profile")).arg(engine->format().majorVersion()).arg(engine->format().minorVersion()).arg(prof == 1 ? "Core" : prof == 2 ? "Compatibility" : ""));
    } else if (prof == 0) {
        INFO( tr("No GL profile.") );
    } else {
        WARNING( tr("Something went wrong!!!") );
        return false;
    }

    GLint s;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &s); // GL_MAX_3D_TEXTURE_SIZE GL_MAX_CUBE_MAP_TEXTURE_SIZE
    INFO ( tr( "Maximum texture size: %1x%1" ).arg( s ) );
    INFO("");
    INFO(tr("GLSL versions: ") + engine->glslvers.join(", "));

    QString inputText = getTextEdit()->toPlainText();
    QString versionProfileLine = inputText.split("\n").at(0);
    if ( versionProfileLine.contains("#version"))
        INFO(tr("Current target: GLSL ") + versionProfileLine.split(" ").at(1) + ((versionProfileLine.contains("compatibility"))?" Compatibility":""));
    else
        INFO(tr("Current target: GLSL 110"));

    INFO(tr("Shaders that do not include a #version directive will be treated as targeting GLSL version 110"));

    // test for nVidia card and GL > 4.0
    if (!(engine->format().majorVersion() > 3 && engine->format().minorVersion() > 0) || !engine->foundnV) {
        if (asmAction != nullptr) {
            editMenu->removeAction(asmAction);
        }
        WARNING(tr("Failed to resolve OpenGL functions required to enable AsmBrowser"));
    }

    QStringList imgFileExtensions;
    QList<QByteArray> a = QImageWriter::supportedImageFormats();
    a.append ( "exr" );
    foreach ( QByteArray s, a ) {
        imgFileExtensions.append ( QString ( s ) );
    }

    DisplayWidget::DrawingState oldState = engine->getState();
    bool pause = pausePlay;
    engine->setState(DisplayWidget::Progressive);
    stop();

    INFO("");
    INFO ( tr("Available image formats: ") + imgFileExtensions.join ( ", " ) );
    INFO("");

    QString filename = tabInfo[tabBar->currentIndex()].filename;
    bool moveMain = settings.value("moveMain", true).toBool();

    fileManager.setOriginalFileName(filename);

    if (variableEditor->hasKeyFrames()) {
        clearKeyFrames();
    }

    if (inputText.contains("#donotrun")) {
        INFO(tr("Not a runnable fragment."));
        //return false;
    }

    Preprocessor p(&fileManager);
    // BUG Fixs Up vector
    QString camSet = getCameraSettings();

    variableEditor->locksUseDefines(QSettings().value("useDefines", true).toBool());
    int ms = 0;
    FragmentSource fs = p.parse(inputText,filename,moveMain);

    engine->useCompat( !fs.source[0].contains("core") );
    engine->setCameraPathLoop(loopCameraPath);

    if (filename != "Unnamed" && !fragWatch->files().contains(filename)) { // file has been saved
            addToWatch( QStringList(filename) );
    }

    // BUG Up vector gets trashed on Build or Save
    variableEditor->updateFromFragmentSource(&fs);
    variableEditor->updateTextures(&fs, &fileManager);
    variableEditor->substituteLockedVariables(&fs);
    variableEditor->updateCamera(engine->getCameraControl());

    if (fs.bufferShaderSource != nullptr) {
        variableEditor->substituteLockedVariables(fs.bufferShaderSource);
    }

    if ( fs.subframeMax != -1 ) {
        setSubframeMax ( fs.subframeMax );
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    static int lastTime;
    QTime start = QTime::currentTime();
    if(rebuildRequired) {
        try {
            engine->setFragmentShader(fs);
            ms = start.msecsTo(QTime::currentTime());
            lastTime = ms;
            rebuildRequired = false;
        } catch (Exception& e) {
            WARNING(e.getMessage());
            rebuildRequired = true;
        }
    } else ms = lastTime;
    QApplication::restoreOverrideCursor();

    // Added sleep of 10 millisecs so that CPU waits a bit for GPU
    std::this_thread::sleep_for(std::chrono::microseconds(10000));

    if (engine->hasShader()) {
        // BUG Fixs Up vector
        if (getCameraSettings() != camSet) {
            variableEditor->setSettings(camSet);
        }
        dockVariableEditor->setHidden( variableEditor->getWidgetCount() == 0 );
        engine->requireRedraw(true);
        engine->resetTime();

        INFO(tr("Compiled script in %1 ms.").arg(ms));
        engine->setState(oldState);
        pause ? stop() : play();

        hideUnusedVariableWidgets();

        highlightBuildButton(false);

        return false; // does not need rebuild
    }
    WARNING(tr("Failed to compile script (%1 ms).").arg(ms));

    highlightBuildButton(true);

    return true;
}

void MainWindow::hideUnusedVariableWidgets()
{
    //TODO: remove group tab if it's empty, if all widgets in group are hidden
    //      they have been optimized out by the glsl compiler

    /// hide unused widgets unless the default state is locked
    QStringList wnames = variableEditor->getWidgetNames();
    for (int i = 0; i < wnames.count(); i++) {
        // find a widget in the variable editor
        VariableWidget *vw = variableEditor->findChild<VariableWidget *>(wnames.at(i));
        if (vw != nullptr) {
            /// get the uniform location from the shader
            int uloc = vw->uniformLocation(engine->getShader());
            vw->setLabelStyle( false ); // no buffershader indicator in gui
            if (uloc == -1 && engine->hasBufferShader()) {
                /// get the uniform location from the buffershader
                uloc = vw->uniformLocation(engine->getBufferShader());
                vw->setLabelStyle( uloc != -1 ); // indicator in gui if exists in buffershader only
            }
            /// locked widgets are transformed into const or #define so don't show up as uniforms
            /// AutoFocus is a dummy so does not exist inside shader program
            if(uloc == -1 &&
                    !(vw->getLockType() == Parser::Locked ||
                      vw->getDefaultLockType() == Parser::AlwaysLocked ||
                      vw->getDefaultLockType() == Parser::NotLockable) &&
                    !wnames.at(i).contains("AutoFocus") )  {
                vw->hide();
            } else {
                vw->show();
            }
        }
    }

    variableEditor->hideUnusedTabs();
}

namespace
{
// Returns the first valid directory.
QString findDirectory(QStringList guesses)
{

    QStringList invalid;
    for (int i = 0; i < guesses.size(); i++) {
        if (QFile::exists(guesses[i])) {
            return guesses[i];
        }
        invalid.append(QFileInfo(guesses[i]).absoluteFilePath());
    }

    // not found.
    WARNING(QCoreApplication::tr("Could not locate directory in: ") + invalid.join(",") + ".");
    return QCoreApplication::tr("[not found]");
}
} // namespace

// Mac needs to step two directories up, when debugging in XCode...
QString MainWindow::getExamplesDir()
{

    QStringList examplesDir;
    examplesDir << "Examples"
                << "../Examples"
                << "../../Examples"
                << "../../../Examples";
    return findDirectory(examplesDir);
}

QString MainWindow::getMiscDir()
{

    QStringList miscDir;
    miscDir << "Misc"
            << "../Misc"
            << "../../Misc"
            << "../../../Misc";
    return findDirectory(miscDir);
}

TextEdit *MainWindow::getTextEdit()
{

    return (TextEdit *)stackedTextEdits->currentWidget();
}

void MainWindow::cursorPositionChanged()
{

    TextEdit *te = getTextEdit();

    if (te == 0) {
        return;
    }

    int pos = te->textCursor().position();
    int blockNumber = te->textCursor().blockNumber();
    // Do reverse look up...
    FragmentSource* fs = engine->getFragmentSource();
    QString x;
    QStringList ex;
    QString filename = tabInfo[tabBar->currentIndex()].filename;

    int incFudge = 1;
    if (!(fs->source.count())) return;
    for (int i = 0; i < fs->lines.count(); i++) {
        if (fs->lines[i] == blockNumber && QString::compare(filename, fs->sourceFileNames[fs->sourceFile[i]], Qt::CaseInsensitive) == 0) {
            ex.append(QString::number(i+incFudge));
        }
    }
    if (ex.count() != 0) {
        x = tr(" Line in preprocessed script: ") + ex.join(",");

    } else {
        x = tr(" (Not part of current script) ");
    }

    statusBar()->showMessage(tr("Position: %1, Line: %2.").arg(pos).arg(blockNumber+incFudge) + x, 5000);
}

TextEdit *MainWindow::insertTabPage(QString filename)
{

    TextEdit *textEdit = new TextEdit(this);
    if(editorStylesheet.isEmpty()) textEdit->setStyleSheet("* {font: 9pt Courier;}");
    else textEdit->setStyleSheet("* {" + editorStylesheet + "}");

    connect(textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));

    textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    textEdit->setTabStopDistance(20);
    textEdit->fh = new FragmentHighlighter(textEdit->document());

    bool loadingSucceded = false;
    if (filename.startsWith("// ")) {
        textEdit->setPlainText(filename);
    } else
    if (filename.toLower().endsWith(".ggr") && QFile(filename).exists()) {
        Ggr2Glsl *gradientCode = new Ggr2Glsl(filename);
        if(gradientCode->isLoaded()) {
            // we can now build a gradient editor...
            QGradient gr = gradientCode->getGradient();
            bool ok = false;
            QGradient gradient = QtGradientDialog::getGradient(&ok, gr, this);
            if(ok) {
                QApplication::setOverrideCursor(Qt::WaitCursor);
                gradient.setCoordinateMode(gr.coordinateMode());
                if(gr != gradient)
                    gradientCode->setGradient(&gradient);

                QStringList glslObjectColorText;
                gradientCode->ggr2glsl( glslObjectColorText, true);
                QStringList glslBackgroundColorText;
                gradientCode->ggr2glsl( glslBackgroundColorText, false);

                QStringList glslText = glslObjectColorText;
                glslText << glslBackgroundColorText;

                textEdit->setPlainText( glslText.join("\n"));

                INFO(tr("Converted file: %1").arg(filename));
                loadingSucceded = true;
                filename.replace(".ggr",".frag");
                QApplication::restoreOverrideCursor();

            } else textEdit->setPlainText(tr("// User canceled ggr2glsl conversion.\n// %1\n").arg(filename));

            loadingSucceded = ok;
        }
    }
    else
    {
        QFile file(filename);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            textEdit->setPlainText(tr("// Cannot read file %1:\n// %2\n").arg(filename).arg(file.errorString()));
        } else {
            QTextStream in(&file);
            QApplication::setOverrideCursor(Qt::WaitCursor);
            textEdit->setPlainText(in.readAll());
            QApplication::restoreOverrideCursor();
            INFO(tr("Loaded file: %1").arg(filename));
            loadingSucceded = true;
        }
    }

    QString displayName = loadingSucceded ? filename : "";
    if (displayName.isEmpty()) {
        // Find a new name
        displayName = tr("Unnamed");
        QString suggestedName = displayName;

        bool unique = false;
        int counter = 1;
        while (!unique) {
            unique = true;
            for (auto &i : tabInfo) {
                if (i.filename == suggestedName) {
                    unique = false;
                    break;
                }
            }
            if (!unique) {
                suggestedName = displayName + " " + QString::number(counter++);
            }
        }
        displayName = suggestedName;
    }

    stackedTextEdits->addWidget(textEdit);

    if (loadingSucceded) {
        tabInfo.append(TabInfo(displayName, textEdit, false, true));
        setRecentFile(filename);
        textEdit->saveSettings( variableEditor->getSettings(false) );

    } else {
        tabInfo.append(TabInfo(displayName, textEdit, true));
    }

    QString tabTitle = QString("%1%2") .arg(strippedName(displayName)).arg(!loadingSucceded ? "*" : "");
    tabBar->setCurrentIndex(tabBar->addTab(strippedName(tabTitle)));

    tabBar->setTabToolTip(tabBar->currentIndex(),filename);

    tabInfo.last().tabIndex = tabBar->currentIndex();

    connect(textEdit->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));

    return textEdit;
}

void MainWindow::resetCamera(bool fullReset)
{

    engine->resetCamera(fullReset);
}

void MainWindow::tabChanged(int index)
{
    if (index > tabInfo.size()) {
        return;
    }
    if (index < 0) {
        return;
    }
    TextEdit *te = getTextEdit();
    te->saveSettings( variableEditor->getSettings(false) );

    TabInfo ti = tabInfo[index];
    QString tabTitle = QString("%1%3").arg(strippedName(ti.filename)).arg(ti.unsaved ? "*" : "");
    stackedTextEdits->setCurrentWidget(ti.textEdit);
    tabBar->setTabText(tabBar->currentIndex(), tabTitle);
    setWindowTitle(QString("Fragmentarium - %1").arg(tabTitle));

    clearKeyFrames();
    setRebuildStatus(true);

    if (!(QSettings().value("autorun", true).toBool())) {
        WARNING(tr("Auto run is disabled! You must select \"Build\" and apply a preset."));
        WARNING(tr("If the preset alters locked variables \"Build\" will be required again."));
        highlightBuildButton(true);
        return;
    }

    setRebuildStatus(initializeFragment());
    // this bit of fudge resets the tab to its last settings
    if(stackedTextEdits->count() > 1 ) {
        setRebuildStatus(initializeFragment());
        te = getTextEdit(); // the currently active one
        if (!te->lastSettings().isEmpty()) {
            setRebuildStatus( variableEditor->setSettings(te->lastSettings()) );
        }
    }
}

void MainWindow::closeTab()
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }
    int index = tabBar->currentIndex();
    closeTab(index);
}

void MainWindow::closeTab(int index)
{
    TabInfo t = tabInfo[index];
    if (t.unsaved) {
        QString mess =
            tr("There are unsaved changes.\r\n%1\r\nContinue will discard changes.")
                .arg(variableEditor->hasEasing()
                ? tr("\r\nTo keep Easing curves you must\r\nadd a preset named \"Range\"\r\nand save before closing!")
                : "\r\n");
        int answer = QMessageBox::warning(this, tr("Unsaved changes"), mess, tr("Continue"), tr("Cancel"));
        if (answer == 1) {
            return;
        }
    }

    if(fragWatch->files().contains(t.filename)) {
      fragWatch->removePath(t.filename);
    }

    tabInfo.remove(index);
    tabBar->removeTab(index);

    stackedTextEdits->removeWidget(t.textEdit);
    delete (t.textEdit); // widget is gone but textedit remains so manually delete it

    clearKeyFrames();
    // if no more tabs don't try to reset to last saved settings
    if (tabBar->currentIndex() == -1) {
        return;
    }
    // this bit of fudge resets the tab to its last settings and preserves textures ???
    setRebuildStatus(initializeFragment());
}

void MainWindow::clearKeyFrames()
{

    // clear the easingcurve settings
    if(variableEditor->hasEasing()) {
        engine->setCurveSettings( QStringList() );
        variableEditor->setEasingEnabled(false);
    }
    // clear the spline data
    if (variableEditor->hasKeyFrames()) {
        clearKeyFrameControl();
}
}

void MainWindow::launchDocumentation()
{

    INFO(tr("Launching web browser..."));
    bool s = QDesktopServices::openUrl(QUrl("https://github.com/3Dickulus/FragM/wiki"));
    if (!s) {
        WARNING(tr("Failed to open browser..."));
    }
}

void MainWindow::launchSfHome()
{

    INFO(tr("Launching web browser..."));
    bool s = QDesktopServices::openUrl(QUrl("http://syntopia.github.com/Fragmentarium/"));
    if (!s) {
        WARNING(tr("Failed to open browser..."));
    }
}

void MainWindow::launchGLSLSpecs()
{

    INFO(tr("Launching web browser..."));
    bool s = QDesktopServices::openUrl(QUrl("http://www.opengl.org/registry/"));
    if (!s) {
        WARNING(tr("Failed to open browser..."));
    }
}

void MainWindow::launchIntro()
{

    INFO("Launching web browser...");
    bool s = QDesktopServices::openUrl(QUrl("http://blog.hvidtfeldts.net/index.php/2011/06/distance-estimated-3d-fractals-part-i/"));
    if (!s) {
        WARNING(tr("Failed to open browser..."));
    }
}

void MainWindow::launchFAQ()
{

    INFO("Launching web browser...");
    bool s = QDesktopServices::openUrl(QUrl("http://blog.hvidtfeldts.net/index.php/2011/12/fragmentarium-faq/"));
    if (!s) {
        WARNING(tr("Failed to open browser..."));
    }
}

void MainWindow::launchReferenceHome()
{

    INFO("Launching web browser...");
    bool s = QDesktopServices::openUrl(QUrl("http://www.fractalforums.com/fragmentarium/"));
    if (!s) {
        WARNING(tr("Failed to open browser..."));
    }
}

void MainWindow::launchGallery()
{

    INFO("Launching web browser...");
    bool s = QDesktopServices::openUrl(QUrl("http://flickr.com/groups/fragmentarium/"));
    if (!s) {
        WARNING(tr("Failed to open browser..."));
    }
}

void MainWindow::makeScreenshot()
{

    engine->update();
    saveImage(engine->grabFramebuffer());
}

void MainWindow::saveImage(QImage image)
{

    QString filename = GetImageFileName(this, tr("Save screenshot as:"));
    if (filename.isEmpty()) {
        return;
    }

    image.setText("frAg", variableEditor->getSettings());

    bool succes = image.save(filename);
    if (succes) {
        INFO(tr("Saved screenshot as: ") + filename);
    } else {
        WARNING(tr("Save failed! Filename: ") + filename);
    }
}

void MainWindow::copy()
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }
    getTextEdit()->copy();
}

void MainWindow::cut()
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }
    getTextEdit()->cut();
}

void MainWindow::paste()
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }
    getTextEdit()->paste();
}

void MainWindow::search()
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }

    QString text;
    bool ok = false;
    /// do we have selected text ? use it or ask for search term
    if(getTextEdit()->textCursor().hasSelection()) {
        text = getTextEdit()->textCursor().selectedText();
    } else {
        text = QInputDialog::getText(this, tr("Search"), tr("Text to find"), QLineEdit::Normal, text, &ok);
    }
restart:
    if(!getTextEdit()->find(text)) {
        text = QInputDialog::getText(this, tr("Not found"), tr("Try again from the start?"), QLineEdit::Normal, text, &ok);
        /// move to beginning and search again
        if(ok && !text.isEmpty()) {
            QTextCursor cursor(getTextEdit()->textCursor());
            cursor.setPosition(0);
            getTextEdit()->setTextCursor( cursor );
            goto restart;
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{

    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    } else {
        INFO(tr("Cannot accept MIME object: ") + ev->mimeData()->formats().join(" - "));
    }
}

void MainWindow::dropEvent(QDropEvent *ev)
{

    if (ev->mimeData()->hasUrls() && ev->source() == nullptr) {
        QList<QUrl> urls = ev->mimeData()->urls();
        for (auto &url : urls) {
            QString file = url.toLocalFile();
            INFO(tr("Loading: ") + file);
            if (file.toLower().endsWith(".fragparams")) {
                loadParameters(file);
            } else if (file.toLower().endsWith(".frag")) {
                loadFragFile(file);
            } else if (file.toLower().endsWith(".png")) {
                // get the frAg parameters
                QString fromPNG = QImage(file).text("frAg");
                // no params ?
                if (fromPNG.isEmpty()) {
                    // try the old tag
                    fromPNG = QImage(file).text("fRAg");
                }

                if (fromPNG.isEmpty()) {
                    // if empty say so
                    INFO(tr("No parameters found in file."));
                } else {
                    // display provenance in log window
                    // Generated by: and Created on: lines
                    QStringList strl = fromPNG.split("\n");
                    if (strl[0].startsWith(tr("// Generated by:"))) {
                        INFO( strl[0].replace("//","  "));
                    }
                    if (strl[1].startsWith(tr("// Created on:"))) {
                        INFO(strl[1].replace("//","  "));
                    }
                    // use the params found in png
                    variableEditor->setSettings(fromPNG);
//                     qApp->clipboard()->setText(fromPNG); // for testing
                }
            } else {
                INFO(tr("Must be a .frag or .fragparams file."));
            }
        }
    } else {
        INFO(tr("Cannot accept MIME object: ") + ev->mimeData()->formats().join(" - "));
    }
}

void MainWindow::setRecentFile(const QString &fileName)
{

    QStringList files = settings.value("recentFileList").toStringList();
    fullPathInRecentFilesList = settings.value("fullPathInRecentFilesList").toBool();

    files.removeAll(fileName);
    if (!fileName.isEmpty()) {
      files.prepend(fileName);
    }

    while (files.size() > maxRecentFiles) {
        files.removeLast();
    }

    settings.setValue("recentFileList", files);

    int numRecentFiles = qMin(files.size(), (int)maxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString absPath = QFileInfo(files[i]).absoluteFilePath();
        QString text = QString("%1 %2").arg(i + 1).arg(fullPathInRecentFilesList ? absPath : QFileInfo(files[i]).fileName());
        recentFileActions[i]->setText(text);
        recentFileActions[i]->setData(absPath);
        recentFileActions[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < maxRecentFiles; ++j) {
        recentFileActions[j]->setVisible(false);
    }

    recentFileSeparator->setVisible(numRecentFiles > 0);
}

void MainWindow::insertText()
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }

    QString text = ((QAction *)sender())->iconText(); // iconText is the menu text without hotkey char
    getTextEdit()->insertPlainText(text.section("//",0,0)); // strip comments
}

void MainWindow::preferences()
{

    PreferencesDialog pd(this);
    pd.exec();
    readSettings();

    QString styleSheetFile = "file:///"+guiStylesheet;
    if(guiStylesheet.isEmpty()) styleSheetFile = "";
    qApp->setStyleSheet(styleSheetFile);
    // restart timer signal after setting preference
    engine->updateRefreshRate();

    setRecentFile("");

    if (tabBar->currentIndex() != -1) {
      getTextEdit()->setStyleSheet("* {" + editorStylesheet + "}");
      getTextEdit()->setTheme(editorTheme);
      getTextEdit()->highlightCurrentLine();
    }

    initTools();
    loopCameraPathAction->setChecked(loopCameraPath);
}

void MainWindow::getBufferSize(int w, int h, int &bufferSizeX, int &bufferSizeY, bool &fitWindow)
{
    if (bufferXSpinBox == nullptr || bufferYSpinBox == nullptr) {
        return;
    }

    fitWindow = lockedToWindowSize;

    if (engine != nullptr && engine->getState() == DisplayWidget::Tiled) {
        bufferSizeX = bufferXSpinBox->value();
        bufferSizeY = bufferYSpinBox->value();
        return;
    }

    if (lockedToWindowSize) {
        // Locked to the window size
        bufferSizeX = w;
        bufferSizeY = h;
    } else if (!lockedToWindowSize) {
        bufferSizeX = bufferXSpinBox->value();
        bufferSizeY = bufferYSpinBox->value();
    }

    if(lockedAspect) {
        double fw = (double)bufferSizeX/(double)w;
        double fh = (double)bufferSizeY/(double)h;
        // adjust X
        if (h > bufferSizeY/fw) {
            bufferSizeX = w;
            if(currentAspect > 1)
                bufferSizeY = MIN(w, round(w / currentAspect));
            else bufferSizeY = MAX(w, round(w / currentAspect));
        }
        else // adjust y
        if (w > bufferSizeX/fh) {
            if(currentAspect > 1)
                bufferSizeX = MAX(h, round(h * currentAspect));
            else bufferSizeX = MIN(h, round(h * currentAspect));
            bufferSizeY = h;
        }
    } else currentAspect = (double)bufferSizeX/(double)bufferSizeY;

    // update the XY spinboxes
    bufferXSpinBox->blockSignals(true);
    bufferYSpinBox->blockSignals(true);
    bufferXSpinBox->setValue(bufferSizeX);
    bufferYSpinBox->setValue(bufferSizeY);
    bufferXSpinBox->blockSignals(false);
    bufferYSpinBox->blockSignals(false);
}

void MainWindow::indent()
{

    if (tabBar->currentIndex() == -1) {
        WARNING(tr("No open tab"));
        return;
    }

    TextEdit *te = getTextEdit();
    int hValue =  te->horizontalScrollBar()->value();
    int vValue =  te->verticalScrollBar()->value();
    int cPos = te->textCursor().position();
    QStringList l = te->toPlainText().split("\n");
    QStringList out;
    int indent = 0;
    foreach (QString s, l) {
        int offset = s.trimmed().startsWith("}") ? -1 : 0;
        QString newString = s.trimmed();
        for (int i = 0; i < indent + offset; i++) {
            newString.push_front("\t");
        }
        out.append(newString);
        indent = indent + s.count("{")+s.count("(") - s.count("}") -s.count(")");
    }
    te->setPlainText(out.join("\n"));

    te->horizontalScrollBar()->setValue(hValue);
    te->verticalScrollBar()->setValue(vValue);
    QTextCursor tc = te->textCursor();
    tc.setPosition(cPos);
    te->setTextCursor(tc);

}

void MainWindow::setFPS(float fps)
{

    if (fps>0) {
        fpsLabel->setText("FPS: " + QString::number(fps, 'f', 1) + " (" + QString::number(1.0 / fps, 'g', 1) + "s)");
    } else {
        fpsLabel->setText("FPS: n.a.");
    }
}

QString MainWindow::getCameraSettings()
{

    QString settings = variableEditor->cameraSettings();
    QStringList l = settings.split("\n");
    QStringList r;
    // added " =" to Eye because Axolotl has Eyes!
    QString camId = engine->getCameraControl()->getID();
    if (camId == "3D") {
        r << l.filter("FOV") << l.filter("Eye =") << l.filter("Target") << l.filter("Up");
    } else if (camId == "2D") {
      r << l.filter("Center") << l.filter("Zoom");
    }
    return r.join("\n");
}

void MainWindow::setCameraSettings(glm::dvec3 e, glm::dvec3 t, glm::dvec3 u)
{

    QString r = QString("Eye = %1,%2,%3\nTarget = %4,%5,%6\nUp = %7,%8,%9\n")
                .arg(e.x).arg(e.y).arg(e.z)
                .arg(t.x).arg(t.y).arg(t.z)
                .arg(u.x).arg(u.y).arg(u.z);
    variableEditor->blockSignals(true);
    if(engine->getFragmentSource()->autoFocus) { // widget detected
        BoolWidget *btest = dynamic_cast<BoolWidget *>(variableEditor->getWidgetFromName("AutoFocus"));
        if (btest != nullptr) {
            if(btest->isChecked()) {
                double d = distance(e, t);
                r += QString("FocalPlane = %1\n").arg(d);
            }
        }
    }
    engine->setCameraPathLoop(loopCameraPath);
    variableEditor->setSettings(r);
    variableEditor->blockSignals(false);
}

QString MainWindow::getPresetNames(bool keyframesORpresets)
{

    int c = variableEditor->getPresetCount(); // total preset count including keyframes
    QStringList k, p;
    if (c > 0) {
        for(int i =0; i<c; i++) {
            QString presetname = variableEditor->getPresetName(i);
            if(!presetname.isEmpty()) {
                if (presetname.contains("KeyFrame", Qt::CaseInsensitive)) { // found a keyframe
                    k << presetname;
                } else {
                    p << presetname;
                }
            }
        }
    }

    return keyframesORpresets ? k.join(";") : p.join(";");
}

void MainWindow::initKeyFrameControl()
{

    if (engine->eyeSpline != nullptr || engine->targetSpline != nullptr || engine->upSpline != nullptr) {
        clearKeyFrameControl();
    }

    int c = variableEditor->getPresetCount();
    int k = variableEditor->getKeyFrameCount();

    if (k > 0) {
        engine->setHasKeyFrames(true);
    }

    if(k>1) {
        variableEditor->setKeyFramesEnabled(true);
        timeSlider->setTickInterval( timeSlider->maximum()/(k-1));
        timeSlider->setTickPosition(QSlider::TicksBelow);
        /// more than 1? try to spline
        /// setup splines for Eye Target and Up vectors
        if(c>1) {
            for(int i =0; i<c; i++) {
                QString presetname = variableEditor->getPresetName(i);
                QRegExp rx = QRegExp("(KeyFrame\\.[0-9]+)");
                if (rx.indexIn(presetname) != -1) { /// found a keyframe, add to keyframeMap
                    QStringList p;
                    p << presetname << variableEditor->getPresetByName( presetname );
                    addKeyFrame( p );
                }
            }

            // after keyframeMap is complete create camera path splines in the engine
            engine->createSplines(k,getFrameMax());
        }
    }
}

void MainWindow::clearKeyFrameControl()
{

    variableEditor->setKeyFramesEnabled(false);
    if (engine->eyeSpline != nullptr || engine->targetSpline != nullptr || engine->upSpline != nullptr) {
        engine->clearControlPoints();
        engine->eyeSpline = nullptr;
        engine->targetSpline = nullptr;
        engine->upSpline = nullptr;
        engine->setHasKeyFrames(false);
        timeSlider->setTickPosition(QSlider::NoTicks);
    }
    keyframeMap.clear();
}

void MainWindow::addKeyFrame(QStringList kfps)
{

    /// for each key frame add ctrl point to the list
    if(engine->cameraID() == "3D" ) {
        KeyFrameInfo *kf = new KeyFrameInfo(kfps);
        keyframeMap.insert(kf->index, kf);
        // TODO: control points in the engine need to be an indexed array
        // in case they are not generated in order ie: editing a point in the middle
        engine->addControlPoint(kf->eye,kf->target,kf->up);
    }

}

void MainWindow::selectPreset()
{

    QString pName;

    pName = QString("#preset %1").arg(variableEditor->getPresetName());

    TextEdit* te = getTextEdit();

    QTextCursor tc= te->textCursor();
    tc.setPosition(0);

    te->setTextCursor(tc);

    bool found = te->find( pName, QTextDocument::FindWholeWords );
    if(found ) {
      tc = te->textCursor();
      tc.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
      found = te->find("#endpreset");
      if(found) {
        tc.setPosition(te->textCursor().position()+1, QTextCursor::KeepAnchor);
        te->setTextCursor(tc);
        } else {
            statusBar()->showMessage(tr("#endpreset not found!"));
        }
    } else {
        statusBar()->showMessage(tr(QString(pName + " not found!").toStdString().c_str()));
    }
}

void MainWindow::processGuiEvents()
{

  // Immediately dispatches all queued events
  qApp->sendPostedEvents();
  // Processes all pending events until there are no more events to process
  qApp->processEvents();

}

void MainWindow::dumpShaderAsm()
{

    if (engine->hasShader()) {
        AsmBrowser::showPage(engine->shaderAsm(true), "Rendershader Program");
    }
    if (engine->hasBufferShader()) {
        AsmBrowser::showPage(engine->shaderAsm(false), "Buffershader Program");
    }
}

void MainWindow::saveCmdScript()
{

    QTextEdit *e = sender()->parent()->findChild<QTextEdit *>("cmdScriptEditor", Qt::FindChildrenRecursively);
    scriptText = e->toPlainText();

    QString filter = tr("Cmd Script (*.fqs);;All Files (*.*)");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), QString(), filter);
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fragmentarium"),
                             tr("Cannot write CmdScript %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    out << scriptText;
    file.close();
    INFO(tr("Cmd Script saved to file:")+fileName);
}

void MainWindow::loadCmdScript()
{

    QString filter = tr("Cmd Script (*.fqs);;All Files (*.*)");
    QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(), filter);
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Fragmentarium"),
                                 tr("Cannot read file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            settings.setValue("cmdscriptfilename", 0);
            return;
        }

        QTextStream in(&file);
        scriptText = in.readAll();
        file.close();
        INFO(tr("Cmd Script loaded from file: ") + fileName);
        settings.setValue("cmdscriptfilename", fileName);
        // is the editor open? overwrite current script.
        QTextEdit *e = sender()->parent()->findChild<QTextEdit *>("cmdScriptEditor", Qt::FindChildrenRecursively);
        if (e != nullptr) {
            e->setPlainText(scriptText);
        }
    }
}

void MainWindow::editScript()
{

    if (scriptText.isEmpty()) {
        loadCmdScript();
    }

    // we need a dialog
    QDialog *d;
    d = new QDialog();
    // with a text editor
    QTextEdit *t;
    t = new QTextEdit();
    // name these objects
    d->setObjectName(QString::fromUtf8("cmdScriptDialog"));
    t->setObjectName(QString::fromUtf8("cmdScriptEditor"));
    // setup some buttons
    QPushButton *saveButton = new QPushButton(tr("&Save"));
    QPushButton *loadButton = new QPushButton(tr("&Load"));
    QPushButton *executeButton = new QPushButton(tr("&Execute"));
    QPushButton *stopButton = new QPushButton(tr("&Stop"));
    QPushButton *closeButton = new QPushButton(tr("&Close"));
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(saveButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(loadButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(executeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(stopButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    // setup the main layout with text editor
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(t);
    mainLayout->addLayout(buttonLayout);
    d->setLayout(mainLayout);
    // give the buttons something to do
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveCmdScript()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(loadCmdScript()));
    connect(executeButton, SIGNAL(clicked()), this, SLOT(executeScript()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopScript()));
    connect(closeButton, SIGNAL(clicked()), d, SLOT(close()));
    // display the script text
    t->setText(scriptText);
    // cmdScriptLineNumber != 0 indicates an error from the last execute cycle
    // so move the cursor and highlight line
    if(cmdScriptLineNumber != 0) {
        QTextCursor tc = t->textCursor();
        tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        tc.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, cmdScriptLineNumber);
        tc.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor,1);
        t->setTextCursor(tc);
    }
    // start with this size
    d->resize(640,480);
    // open the dialog and take control
    d->exec();

    scriptText = t->toPlainText();

    // closed the window so...
    runningScript=false;
}

void MainWindow::executeScript()
{
    QTextEdit *e = nullptr;

    QString name = settings.value("filename").toString();
    QString scriptname = settings.value("cmdscriptfilename").toString();

    if (sender() != nullptr) {
        e = sender()->parent()->findChild<QTextEdit *>("cmdScriptEditor", Qt::FindChildrenRecursively);
        scriptText = e->toPlainText();
    }

    runningScript=true;

    QScriptValue result = scriptEngine.evaluate( scriptText, scriptname );

    runningScript=false;

    if (result.isError()) {
        QString err = result.toString();
        cmdScriptLineNumber = scriptEngine.uncaughtExceptionLineNumber();
        QString msg = tr("Error %1 at line %2").arg(err).arg(cmdScriptLineNumber);
        INFO(msg);
        // highlight the error line
        if (cmdScriptLineNumber != 0 && e != nullptr) {
            QTextCursor tc = e->textCursor();
            tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            tc.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, cmdScriptLineNumber - 1);
            tc.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor,1);
            e->setTextCursor(tc);
        }
        if (sender() != nullptr) {
            cmdScriptDebugger->attachTo(&scriptEngine);
        }
    } else {
        if (sender() != nullptr) {
            cmdScriptDebugger->action(QScriptEngineDebugger::ClearConsoleAction)->trigger();
            cmdScriptDebugger->action(QScriptEngineDebugger::ClearDebugOutputAction)->trigger();
            cmdScriptDebugger->action(QScriptEngineDebugger::ClearErrorLogAction)->trigger();
            cmdScriptDebugger->detach();
        }
        cmdScriptLineNumber = 0;
    }

    // can't edit the script in the debugger
    // only for examining status, messages and vars
    // only used when called from GUI

    if (runningScript) {
        settings.setValue("filename", name);
    }
    // finished script
}

void MainWindow::setupScriptEngine()
{

    runningScript=false;
    // expose these widgets to the script
    appContext = scriptEngine.newQObject(this);
    scriptEngine.globalObject().setProperty("app", appContext);
    // create a debugger for QScript
    if (cmdScriptDebugger == nullptr) {
        cmdScriptDebugger = new QScriptEngineDebugger(this);
        cmdScriptDebugger->standardWindow()->setWindowModality(Qt::ApplicationModal);
        cmdScriptDebugger->standardWindow()->resize(1280, 704);
    }
}

// this function is connected to a signal from listwidget in logger
// if the file is already loaded then jump to line else load the file and jump to line
void MainWindow::loadErrorSourceFile(QString fileName, int LineNumber)
{
    QStringList openFiles;
    if (!tabInfo.isEmpty()) {
        for (auto &i : tabInfo) {
            openFiles << i.filename;
            if(fileName == i.filename) {
                tabBar->setCurrentIndex( i.tabIndex );
            }
        }
    }

    if(!openFiles.contains(fileName)) {

        loadFragFile(fileName);
    }
    // jump to error line in text editor
    TextEdit *te = getTextEdit();
    QTextCursor cursor(te->textCursor());
    cursor.setPosition(0); // line numbers are indexed from 0
    cursor.movePosition(QTextCursor::Down,QTextCursor::MoveAnchor,LineNumber-1);
    te->setTextCursor( cursor );
    te->centerCursor();
}

/* Slot handler of F6
 * */
void MainWindow::slotShortcutF6()
{
    QString scriptname = settings.value("cmdscriptfilename").toString();

    bool loadingSucceded = false;

    if(!scriptname.isEmpty()) {


        QApplication::setOverrideCursor(Qt::WaitCursor);

        QFile file(scriptname);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            WARNING(tr("Cannot read file %1:\n%2.").arg(scriptname).arg(file.errorString()));
        } else {
            QTextStream in(&file);

            scriptText = in.readAll();

            INFO(tr("Loaded file: %1").arg(scriptname));
            loadingSucceded = true;
        }
    }

    if(loadingSucceded) {

        runningScript = true;

        QScriptValue result = scriptEngine.evaluate( scriptText, scriptname );

        runningScript=false;

        QApplication::restoreOverrideCursor();

        if (result.isError()) {
            QString err = result.toString();
            cmdScriptLineNumber = scriptEngine.uncaughtExceptionLineNumber();
            WARNING(tr("Error %1 at line %2").arg(err).arg(cmdScriptLineNumber));
        } else {
            cmdScriptLineNumber = 0;
        }

    } else runningScript = false;
}

/* Slot handler of Shift+F6
 *
 * Bind an fqScript to F6 hotkey
 * */
void MainWindow::slotShortcutShiftF6()
{
    static QString lastBoundCmdScript = "";

    QString filter = tr("CMD Script (*.fqs);;All Files (*.*)");
    QString fileName =
        QFileDialog::getOpenFileName(this, tr("Bind CMD script to F6 key"), lastBoundCmdScript, filter);
    if (!fileName.isEmpty()) {
        lastBoundCmdScript = fileName;
        INFO("Bound " + fileName + " to F6");
    } else fileName = lastBoundCmdScript;

    settings.setValue("cmdscriptfilename", fileName);
}

} // namespace GUI
} // namespace Fragmentarium
