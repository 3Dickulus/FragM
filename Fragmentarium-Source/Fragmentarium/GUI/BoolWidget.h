#pragma once

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QMap>
#include <QSlider>
#include <QString>
#include <QTabWidget>
#include <QVector>
#include <QWidget>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QOpenGLShaderProgram>

#include "ExpSpinBox.h"
#include "../Parser/Preprocessor.h"
#include "DisplayWidget.h"
#include "SyntopiaCore/Logging/Logging.h"

#include "VariableWidget.h"

/// Classes for the GUI Editor for the preprocessor constant variables.
/// E.g. the line: uniform bool Alternate; checkbox[false]
///	will make a simple editor widget appear.

namespace Fragmentarium
{
namespace GUI
{

using namespace SyntopiaCore::Logging;
using namespace Fragmentarium::Parser;

class BoolWidget : public VariableWidget
{
    Q_OBJECT
public:
    /// BoolVariable constructor.
    BoolWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                 bool defaultValue );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3" ).arg ( group ).arg ( getName() ).arg ( defaultValue );
    }
    virtual QString getValueAsText()
    {
        return ( checkBox->isChecked() ? "true" : "false" );
    }
    virtual QString toString();
    virtual bool fromString(QString string);
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
    void reset()
    {
        checkBox->setChecked ( defaultValue );
    }
    QString getLockedSubstitution()
    {
        return "const bool " + name + " = " +
               ( checkBox->isChecked() ? "true" : "false" ) + ";";
    }
    QString getLockedSubstitution2()
    {
        return "#define " + name + " " +
               ( checkBox->isChecked() ? "true" : "false" ) + "";
    }
    bool isChecked()
    {
        return checkBox->isChecked();
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
    };
    void setSliderType(SliderType ){};
    
    void contextMenuEvent ( QContextMenuEvent *ev)
    {
        QMenu contextMenu;
        QAction findInSource ( tr ( "Select source" ), &contextMenu );
        contextMenu.addAction ( &findInSource );

        QAction *choice = contextMenu.exec ( ev->globalPos() );
        
        if ( choice == &findInSource ) {
            emit sliderBoundsChanged( objectName() + QString(" 0 0") );
        }
        
    }

signals:
    void sliderBoundsChanged(QString uniqueName);

private:
    QCheckBox* checkBox;
    bool defaultValue;
};




}
} // namespace Fragmentarium
