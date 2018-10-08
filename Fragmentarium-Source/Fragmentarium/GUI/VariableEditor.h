#pragma once

#include <QString>
#include <QVector>
#include <QWidget>
#include <QMap>
#include <QSlider>
#include <QTabWidget>
#include <QComboBox>
#include <QFrame>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QOpenGLShaderProgram>
#include "../Parser/Preprocessor.h"
#include "SyntopiaCore/Logging/Logging.h"
#include "DisplayWidget.h"
#include "VariableWidget.h"

/// The editor window for GUI variables (uniforms)
namespace Fragmentarium {
namespace GUI {

using namespace SyntopiaCore::Logging;
using namespace Fragmentarium::Parser;

class MainWindow;  // forward
class ComboSlider; // forward

/// The Variable Editor window.
class VariableEditor : public QWidget {
    Q_OBJECT
public:
    VariableEditor(QWidget* parent, MainWindow* mainWin);

    void updateFromFragmentSource(FragmentSource* fs, bool* showGUI);
    void updateCamera(CameraControl* c);
    void setUserUniforms(QOpenGLShaderProgram* shaderProgram);
    QVector<VariableWidget*> getUserUniforms(){return variables;};
    QString getSettings();
    bool setSettings(QString text);
    void createGroup(QString g);
    VariableWidget* getWidgetFromName(QString name);
    void setPresets(QMap<QString, QString> presets);
    ComboSlider* getCurrentComboSlider() {
        return currentComboSlider;
    }
    void setCurrentComboSlider(ComboSlider* cs) {
        focusChanged(currentComboSlider,cs);
        currentComboSlider = cs;
    }

    bool setDefault();
    void substituteLockedVariables(FragmentSource* fs);
    void updateTextures(FragmentSource* fs, FileManager* fileManager);

    void setEasingCurve();
    void setEasingCurves( QString ecset );
    int addEasingCurve(QString c);
    int getKeyFrameCount();
    void setKeyFramesEnabled(bool k) {
        keyFramesEnabled = k;
    };
    bool hasKeyFrames() {
      return keyFramesEnabled;
    }
    void locksUseDefines(bool f) {
      useDefines = f;
    }
    void setEasingEnabled(bool e) {
        easingEnabled = e;
    };
    // saves easing curve settings in presets
    void setSaveEasing(bool e) {
        saveEasing = e;
    };
    bool hasEasing() {
        return easingEnabled;
    }
    QString getPresetName(int i) {
        return presetComboBox->itemText(i);
    }
    
    QString getPresetName() {
      return presetComboBox->currentText();
    }
    
    QStringList getPresetByName(QString name);
    int getCurrentKeyFrame();
    int getPresetCount() {
        return presetComboBox->count();
    }
    void setPreset(QString p);
    
    QStringList getWidgetNames() {
      QStringList varnames;
      for (int i = 0; i < variables.count(); i++) {
        varnames << variables[i]->getName();
      }
      return varnames;
    }
    
signals:
    void changed(bool lockedChanged);

public slots:
    void sliderDestroyed(QObject* o);
    void focusChanged(QWidget* oldWidget,QWidget* newWidget);
    bool applyPreset();
    void resetUniforms(bool clear = true);
    void resetGroup();
    void lockGroup();
    void unlockGroup();
    void copy();
    void copyGroup();
    void paste();
    void childChanged(bool lockedChanged);
    void presetSelected(QString presetName);
    void dockChanged(bool t){
        if(width() > height()*2) t = true;
        tabWidget->setTabPosition( t ? (QTabWidget::North) : (QTabWidget::East)); // 05/22/17 Sabine ;)
    }
    
protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    
private:
    QMap<QString, QString> presets;
    MainWindow* mainWindow;
    QSpacerItem* spacer;
    QVector<VariableWidget*> variables;
    QVBoxLayout* layout;
    QComboBox* presetComboBox;
    QWidget* currentWidget;

    QMap<QString, QWidget*> tabs;
    QMap<QWidget*, QWidget*> spacers;
    QTabWidget* tabWidget;
    ComboSlider* currentComboSlider;
    bool keyFramesEnabled;
    bool easingEnabled;
    bool saveEasing;
    bool useDefines;
};
}
}

