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

#include "FloatWidget.h" // for float combo helper

#define DBOUT qDebug() << QString(__FILE__).split(QDir::separator()).last() << __LINE__ << __FUNCTION__

/// Classes for the GUI Editor for the preprocessor constant variables.
/// E.g. the line: uniform float Angle; slider[45.00,0,360]
///	will make a simple editor widget appear.
namespace Fragmentarium
{
namespace GUI
{

// decimal places for float and double
#define FDEC 9
#define DDEC 18

using namespace SyntopiaCore::Logging;
using namespace Fragmentarium::Parser;

class ColorChooser : public QFrame
{
	Q_OBJECT
public:
	ColorChooser(QWidget* parent, glm::dvec3 defaultValue)
	: QFrame ( parent ), defaultValue ( defaultValue ), value ( defaultValue )
	{
		setLayout(new QHBoxLayout(this));
		setColor(defaultValue);
		setAutoFillBackground( true );
		setFrameStyle(QFrame::Panel | QFrame::Plain);
		setLineWidth(1);
	}
	
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
public slots:
	// BUG palette gets inherited when using an application level stylesheet loaded
	// via the cmdline option --stylesheet=filename.qss this widget no longer responds
	// to setting the palette backgroundrole color
	// FIX: use stylesheet internally/manually to set colors instead of palette
	void setColor ( glm::dvec3 v )
	{
		QPalette p = palette();
		QColor c;
		
		c.setRgbF(v.x,v.y,v.z);
		setStyleSheet(QString(
			"* {background: %1;"
			"border-color: %2;"
			"border-width: 1px;"
			"border-style: solid;"
			"border-radius: 3px;}"
		).arg(c.name(QColor::HexRgb)).arg(p.color(QPalette::Dark).name(QColor::HexRgb))
		);
		value = v;
	}
	
	void setColor ( QColor c )
	{
		if (c.isValid()) {
			value = glm::dvec3(c.redF(), c.greenF(), c.blueF());
			setColor(value);
			emit changed();
		}
	}
	
	glm::dvec3 getValue()
	{
		return value;
	}
	
private slots:
	void mouseReleaseEvent ( QMouseEvent * )
	{
		QColor initial;
		initial.setRgbF( value.x, value.y, value.z );
		
		qcd = new QColorDialog(initial);
		connect ( qcd, SIGNAL ( currentColorChanged ( QColor ) ), this, SLOT ( setColor ( QColor ) ) );
		if ( !qcd->exec() ) {
			setColor ( initial );
		}
	}
	
signals:
	void changed();
	void sliderBoundsChanged(QString uniqueName);
	
private:
	QColorDialog *qcd;
	glm::dvec3 defaultValue;
	glm::dvec3 value;
};

//////////////////////////////////////////////////////////
class ColorWidget : public VariableWidget
{
    Q_OBJECT
public:
    /// FloatVariable constructor.
    ColorWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                  glm::dvec3 defaultValue );
    virtual QString getUniqueName()
    {
        QString d = QString("[%1 %2 %3]").arg(defaultValue.x).arg(defaultValue.y).arg(defaultValue.z);
        return QString ( "%1:%2:%3" ).arg ( group ).arg ( getName() ).arg(d);
    }
    virtual QString getValueAsText()
    {
        glm::dvec3 t = colorChooser->getValue();
        int p = FDEC;
        if ( isDouble() ) {
            p = DDEC;
        }
        return QString::number ( t.x, 'g', p ) + "," +
               QString::number ( t.y, 'g', p ) + "," +
               QString::number ( t.z, 'g', p );
    }

    glm::dvec3 getValue()
    {
        return colorChooser->getValue();
    }
    void setValue(glm::dvec3 v);
    virtual QString toString();
    virtual bool fromString(QString string);
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);

    void reset()
    {
        colorChooser->setColor ( defaultValue );
    }
    QString getLockedSubstitution()
    {
        QString type = isDouble() ? "dvec3" : "vec3";
        return "const " + type + " " + name + " = " + type + "(" +
               toGLSL ( colorChooser->getValue().x ) + "," +
               toGLSL ( colorChooser->getValue().y ) + "," +
               toGLSL ( colorChooser->getValue().z ) + ");";
    }
    QString getLockedSubstitution2()
    {
        QString type = isDouble() ? " dvec3(" : " vec3(";
        return "#define " + name + type + toGLSL ( colorChooser->getValue().x ) +
               "," + toGLSL ( colorChooser->getValue().y ) + "," +
               toGLSL ( colorChooser->getValue().z ) + ")";
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
    };
    void setSliderType(SliderType ){};

private:
    ColorChooser* colorChooser;
    glm::dvec3 defaultValue;
};

class FloatColorWidget : public VariableWidget
{
    Q_OBJECT
public:
    /// FloatVariable constructor.
    FloatColorWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                       double defaultValue, double min, double max,
                       glm::dvec3 defaultColorValue );
    virtual QString getUniqueName()
    {
        QString d = QString("[%1 %2 %3]").arg(defaultColorValue.x).arg(defaultColorValue.y).arg(defaultColorValue.z);
        return QString ( "%1:%2:%3:%4:%5:%6" ).arg ( group ).arg ( getName() ).arg ( min ).arg ( defaultValue ).arg ( max ).arg ( d );
    }
    virtual QString getValueAsText()
    {
        glm::dvec3 t = colorChooser->getValue();
        int p = FDEC;
        if ( isDouble() ) {
            p = DDEC;
        }
        return QString::number ( t.x, 'g', p ) + "," +
               QString::number ( t.y, 'g', p ) + "," +
               QString::number ( t.z, 'g', p ) + "," +
               QString::number ( comboSlider->getValue(), 'g', p );
    }
    glm::dvec4 getValue()
    {
        return glm::dvec4 ( colorChooser->getValue(), comboSlider->getValue() );
    }
    void setValue(glm::dvec4 v);
    virtual QString toString();
    virtual bool fromString(QString string);
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
    void reset()
    {
        comboSlider->setValue ( defaultValue );
        colorChooser->setColor ( defaultColorValue );
    }
    QString getLockedSubstitution()
    {
        QString type = isDouble() ? "dvec4" : "vec4";
        return "const " + type + " " + name + " = " + type + "(" +
               toGLSL ( colorChooser->getValue().x ) + "," +
               toGLSL ( colorChooser->getValue().y ) + "," +
               toGLSL ( colorChooser->getValue().z ) + "," +
               toGLSL ( comboSlider->getValue() ) + ");";
    }
    QString getLockedSubstitution2()
    {
        QString type = isDouble() ? " dvec4(" : " vec4(";
        return "#define " + name + type + toGLSL ( colorChooser->getValue().x ) +
               "," + toGLSL ( colorChooser->getValue().y ) + "," +
               toGLSL ( colorChooser->getValue().z ) + "," +
               toGLSL ( comboSlider->getValue() ) + ")";
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
        comboSlider->setDecimals(wd ? DDEC : FDEC);
    };
    void setSliderType(SliderType ){};

private:
    ComboSlider* comboSlider;
    ColorChooser* colorChooser;
    double defaultValue;
    double min;
    double max;
    glm::dvec3 defaultColorValue;
};

}
} // namespace Fragmentarium
