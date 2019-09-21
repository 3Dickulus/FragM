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

#include "../Parser/Preprocessor.h"
#include "DisplayWidget.h"
#include "SyntopiaCore/Logging/Logging.h"
#include <QOpenGLShaderProgram>

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

// A helper class (combined float slider+spinner)
class ComboSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(double value READ getValue WRITE setValue)
public:
    ComboSlider ( QWidget *parent, QWidget *variableEditor, double defaultValue, double minimum, double maximum, bool logarithmic = false)
        : QWidget ( parent ), variableEditor ( variableEditor ), defaultValue ( defaultValue ), minimum ( minimum ), maximum ( maximum ), logarithmic ( logarithmic )
    {

        setMinimumSize(160,20);
        setMaximumSize(1024,20);

        QHBoxLayout* l = new QHBoxLayout(this);
        l->setSpacing(2);
        l->setContentsMargins(0,0,0,0);

        double rangemin, rangemax;
        if (logarithmic) {
            rangemin = std::log(std::min(std::abs(minimum), std::abs(maximum)));
            rangemax = std::log(std::max(std::abs(minimum), std::abs(maximum)));
            if (maximum < 0) {
              double tmp = rangemin; rangemin = rangemax; rangemax = tmp;
            }
        } else {
            rangemin = minimum;
            rangemax = maximum;
        }
        // 4294967295
        scale = (1.0/(rangemax-rangemin))*100000000;

        slider = new QSlider(Qt::Horizontal,this);
        slider->setRange(rangemin*scale,rangemax*scale+1);
        slider->setValue((logarithmic ? std::log(std::abs(defaultValue)) : defaultValue)*scale);
        slider->setSingleStep(scale/1000);
        slider->setPageStep(scale/100);
        slider->setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
        l->addWidget(slider);

        spinner = new QDoubleSpinBox(this);
        spinner->setDecimals(FDEC);
        spinner->setMaximum(maximum);
        spinner->setMinimum(minimum);
        spinner->setValue(defaultValue);
        spinner->setKeyboardTracking(false);
        l->addWidget(spinner);

        setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

        connect ( spinner, SIGNAL ( valueChanged ( double ) ), this, SLOT ( spinnerChanged ( double ) ) );
        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));

        m_anim = new QPropertyAnimation(this,"value");
        m_loops = 1;
        m_pong = 0;
        m_framestart = 0;
        m_framefin = 0;
    }
    void setDecimals ( int d )
    {
        spinner->setDecimals ( d );
    }
    void contextMenuEvent ( QContextMenuEvent * )
    {

        bool ok;
        int i =
            QInputDialog::getInt ( this, objectName(), tr ( "Slider Step Multiplier" ), slider->singleStep(), 1, 100000000, 1, &ok );
        if (ok) {
            slider->setSingleStep(i);
            slider->setPageStep(i*10);
        }
    }

    QPropertyAnimation *m_anim;
    int m_framestart, m_framefin; // firstframe lastframe
    int m_loops;
    int m_pong;

    double getMin()
    {
        return minimum;
    }
    double getMax()
    {
        return maximum;
    }
    double getSpan()
    {
        return maximum - minimum;
    }
    double getValue()
    {
        return spinner->value();
    }

    int getLoops()
    {
        return m_loops;
    }
    int getPong()
    {
        return m_pong;
    }

public slots:
    void setValue ( double d )
    {
        spinner->setValue(d);
        bool clamping = (d < minimum || d > maximum);
        if(clamping) {
            QString msg = QString ( tr ( "Clamping" ) + " " + objectName() + " " + tr ( "to min/max range!" ) );
            WARNING( msg );
        }
        if (logarithmic && ! ((d < 0 && minimum < 0 && maximum < 0 && defaultValue < 0) || (d > 0 && minimum > 0 && maximum > 0 && defaultValue > 0))) {
            // this warns on every change, but WARNING() in the constructor is invisible
            WARNING("Logarithmic slider " + objectName() + " range contains 0!");
        }
    }

    void setSliderType(SliderType st) {

        logarithmic = (st.toString() == QString("Logarithmic"));

        double rangemin, rangemax;
        if (logarithmic) {
            rangemin = std::log(std::min(std::abs(minimum), std::abs(maximum)));
            rangemax = std::log(std::max(std::abs(minimum), std::abs(maximum)));
            if (maximum < 0) {
              double tmp = rangemin; rangemin = rangemax; rangemax = tmp;
            }
        } else {
            rangemin = minimum;
            rangemax = maximum;
        }
        // 4294967295
        scale = (1.0/(rangemax-rangemin))*100000000;

        slider->setRange(rangemin*scale,rangemax*scale+1);
        slider->setValue((logarithmic ? std::log(std::abs(spinner->value())) : spinner->value())*scale);
        slider->setSingleStep(scale/1000);
        slider->setPageStep(scale/100);
    }

signals:
    void changed();

protected slots:
    void spinnerChanged ( double d )
    {
        // our value comes from the spin box
        // the slider must be kept quiet while adjusting from spinner
        slider->blockSignals(true);

        if (logarithmic && ! ((d < 0 && minimum < 0 && maximum < 0 && defaultValue < 0) || (d > 0 && minimum > 0 && maximum > 0 && defaultValue > 0))) {
            // this warns on every change, but WARNING() in the constructor is invisible
            WARNING("Logarithmic slider " + objectName() + " range contains 0!");
        }
        slider->setValue((logarithmic ? std::log(std::abs(d)) : d)*scale);

        slider->blockSignals(false);
        // let the main gui thread know something happened
        emit changed();

    }
    void sliderChanged ( int i )
    {
        // our value is calculated from the slider position
        // the spinner must be kept quiet while adjusting from slider
        spinner->blockSignals(true);

        spinner->setValue(logarithmic ? (maximum < 0 ? -1 : +1) * std::exp(i/scale) : i/scale);

        spinner->blockSignals(false);
        // let the main gui thread know something happened
        emit changed();
    }

private:
    QSlider* slider;
    SliderType sliderType;
    QDoubleSpinBox* spinner;
    QWidget* variableEditor;
    double defaultValue;
    double minimum;
    double maximum;
    bool logarithmic;
    double scale;
};

class ColorChooser : public QFrame
{
    Q_OBJECT
public:
    ColorChooser(QWidget* parent, glm::dvec3 defaultValue)
        : QFrame ( parent ), defaultValue ( defaultValue ), value ( defaultValue )
    {
        setLayout(new QHBoxLayout(this));
        setColor(defaultValue);
        setFrameStyle(QFrame::Panel | QFrame::Plain);
        setLineWidth(1);
    }

public slots:
    void setColor ( glm::dvec3 v )
    {
        QPalette p = palette();
        QColor c;
        c.setRgbF(v.x,v.y,v.z);
        p.setColor(backgroundRole(), c);
        setAutoFillBackground( true );
        setPalette(p);
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

private:
    QColorDialog *qcd;
    glm::dvec3 defaultValue;
    glm::dvec3 value;
};


// A helper class (combined int slider+spinner)
class IntComboSlider : public QWidget
{
    Q_OBJECT
public:
    IntComboSlider(QWidget* parent, int defaultValue, int minimum, int maximum)
        : QWidget ( parent ), defaultValue ( defaultValue ), min ( minimum ), max ( maximum )
    {

        QHBoxLayout* l = new QHBoxLayout(this);
        l->setSpacing(2);
        l->setContentsMargins(0,0,0,0);

        slider = new QSlider(Qt::Horizontal,this);
        slider->setRange(minimum,maximum);
        slider->setValue(defaultValue);
        l->addWidget(slider);

        spinner = new QSpinBox(this);
        spinner->setMaximum(maximum);
        spinner->setMinimum(minimum);
        spinner->setValue(defaultValue);
        spinner->setKeyboardTracking(false);
        l->addWidget(spinner);

        setSizePolicy ( QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

        connect ( spinner, SIGNAL ( valueChanged ( int ) ), this, SLOT ( spinnerChanged ( int ) ) );
        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    }

    int getValue()
    {
        return spinner->value();
    }
public slots:
    void setValue ( int i )
    {
        spinner->setValue ( i < min ? min : i > max ? max : i );
    }

signals:
    void changed();

protected slots:
    void spinnerChanged ( int val )
    {
        slider->blockSignals(true);
        slider->setValue(val);
        slider->blockSignals(false);
        emit changed();
    }

    void sliderChanged ( int val )
    {
        spinner->blockSignals(true);
        spinner->setValue(val);
        spinner->blockSignals(false);
        emit changed();
    }

private:

    QSlider* slider;
    QSpinBox* spinner;
    int defaultValue;
    int min;
    int max;
};


class CameraControl;

/// Widget editor base class.
class VariableWidget : public QWidget
{

    Q_OBJECT
public:
    VariableWidget(QWidget* parent, QWidget* variableEditor, QString name);
    virtual void updateTextures ( FragmentSource * /*fs*/, FileManager * /*fileManager*/ ) {}
    virtual QString getValueAsText()
    {
        return "";
    }
    QString getName() const
    {
        return name;
    }
    virtual void reset() = 0;
    void setGroup ( QString group )
    {
        this->group = group;
    }
    QString getGroup()
    {
        return group;
    }
    void setProvenance(Provenance p)
    {
        provenance = p;
    }
    void addProvenance(Provenance p)
    {
        provenance = Provenance(provenance | p);
    }
    Provenance getProvenance() const
    {
        return provenance;
    }
    bool isUpdated() const
    {
        return updated;
    }
    void setUpdated ( bool value )
    {
        updated = value;
    }
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram) = 0;
    QString toSettingsString();
    bool fromSettingsString(QString string);
    virtual QString toString() = 0;
    virtual bool fromString(QString string) = 0;
    virtual QString getUniqueName() = 0;
    void setSystemVariable ( bool v )
    {
        systemVariable = v;
    }
    bool isSystemVariable()
    {
        return systemVariable;
    }
    virtual void setIsDouble(bool v) = 0;
    bool isDouble()
    {
        return wantDouble;
    }
    int uniformLocation(QOpenGLShaderProgram* shaderProgram);
    bool isLocked();
    LockType getDefaultLockType()
    {
        return defaultLockType;
    }
    void setDefaultLockType(LockType lt);
    LockType getLockType()
    {
        return lockType;
    }
    virtual void setLockType(LockType lt) ;
    virtual QString getLockedSubstitution() = 0;
    virtual QString getLockedSubstitution2() = 0;
    virtual void setSliderType(SliderType ){};

public slots:
    void locked(bool l);
    void valueChanged();

signals:
    void changed(bool lockedChanged, Provenance provenance);

protected:
    QString toGLSL ( double d )
    {
        QString n = QString::number(d,'g',isDouble()?DDEC:FDEC);
        // GLSL requires a dot in floats.
        if ( n.contains ( "." ) || n.contains ( "e" ) ) {
            return n;
        }
        return n+".0";
    }

    LockType defaultLockType;
    LockType lockType;
    LockType oldLockType;
    SliderType defaultSliderType;
    SliderType sliderType;

    QPushButton* lockButton;
    QString name;
    QString group;
    bool updated;
    bool systemVariable;
    bool wantDouble=false;
    QWidget* widget;
    QWidget* variableEditor;
    Provenance provenance;
};

class SamplerWidget : public VariableWidget
{
    Q_OBJECT
public:
    SamplerWidget ( FileManager *fileManager, QWidget *parent,
                    QWidget *variableEditor, QString name, QString defaultValue, QString defaultChannelValue="" );
    virtual QString toString();
    virtual bool fromString(QString string);
    virtual void setUserUniform ( QOpenGLShaderProgram* shaderProgram );
    virtual void updateTextures(FragmentSource* fs, FileManager* fileManager);
    virtual void setLockType ( LockType /*lt*/ )
    {
        lockType = AlwaysLocked;
    } // cannot change this
    
    QString getValue() ;
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3:%4" ).arg ( group ).arg ( getName() );
    }
    void reset()
    {
        comboBox->setEditText ( defaultValue );
        if(!defaultChannelValue.isEmpty()) {
            channelComboBox->show();
            int i = channelComboBox->findText(defaultChannelValue);
            if(i != -1) {
                channelComboBox->setCurrentIndex(i);
            }
        } else channelComboBox->hide();
    }
    QString getChannelValue()
    {
        if(!channelComboBox->isHidden())
        return channelComboBox->currentText();
        else return "";
    }
    int hasChannel(QString chan)
    {
        if(!channelComboBox->isHidden())
        return channelComboBox->findText(chan);
        else return -1;
    }
    QString getLockedSubstitution()
    {
        return QString();
    }
    QString getLockedSubstitution2()
    {
        return QString();
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
    };
    int texID;

signals:
    void changed();

protected slots:

    void textChanged(const QString& text);
    void channelChanged(const QString& text);

    void buttonClicked();

private:

    QComboBox* comboBox;
    QComboBox* channelComboBox;
    QPushButton* pushButton;
    FileManager* fileManager;
    QString defaultValue;
    QString defaultChannelValue;
};


/// A widget editor for a float variable.
class FloatWidget : public VariableWidget
{
public:
    /// FloatVariable constructor.
    FloatWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                  double defaultValue, double min, double max, bool logarithmic );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3:%4" ).arg ( group ).arg ( getName() ).arg ( min ).arg ( max );
    }
    virtual QString getValueAsText()
    {
        int p = FDEC;
        if ( isDouble() ) {
            p = DDEC;
        }
        return QString::number(comboSlider1->getValue(),'g',p);
    }
    virtual QString toString();
    virtual bool fromString(QString string);
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
    double getValue()
    {
        return comboSlider1->getValue();
    }
    void setValue(double f);
    void reset()
    {
        setValue ( defaultValue );
    }
    QString getLockedSubstitution()
    {
        QString type = isDouble() ? "double " : "float ";
        return "const " + type + name + " = " + toGLSL(getValue()) +";";
    }
    QString getLockedSubstitution2()
    {
        return "#define " + name + " " + toGLSL ( getValue() );
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
        comboSlider1->setDecimals(wd ? DDEC : FDEC);
    };

    virtual void setSliderType ( SliderType st ){
        comboSlider1->setSliderType(st);
        sliderType = st;
    };

private:
    ComboSlider* comboSlider1;
    double defaultValue;
    double min;
    double max;
};

/// A widget editor for a float2 variable.
class Float2Widget : public VariableWidget
{
public:
    Float2Widget ( QWidget *parent, QWidget *variableEditor, QString name,
                   glm::dvec2 defaultValue, glm::dvec2 min, glm::dvec2 max, bool logarithmic );

    virtual QString getUniqueName()
    {
        QString f = QString("[%1 %2]").arg(min.x).arg(min.y);
        QString t = QString("[%1 %2]").arg(max.x).arg(max.y);
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
    }

    virtual QString getValueAsText()
    {
        int p = FDEC;
        if ( isDouble() ) {
            p = DDEC;
        }
        return QString::number ( comboSlider1->getValue(), 'g', p ) + "," +
               QString::number ( comboSlider2->getValue(), 'g', p );
    }
    virtual QString toString();
    virtual bool fromString(QString string);
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);

    // Third component unused for these
    glm::dvec2 getValue()
    {
        return glm::dvec2 ( comboSlider1->getValue(), comboSlider2->getValue() );
    }
    void setValue(glm::dvec3 v);
    void reset()
    {
        setValue ( glm::dvec3(defaultValue.x,defaultValue.y,0.0) );
    }
    QString getLockedSubstitution()
    {
        QString type = isDouble() ? "dvec2" : "vec2";
        return "const " + type + " " + name + " = " + type + "(" +
               toGLSL ( getValue().x ) + "," + toGLSL ( getValue().y ) + ");";
    }
    QString getLockedSubstitution2()
    {
        QString type = isDouble() ? " dvec2(" : " vec2(";
        return "#define " + name + type + toGLSL ( getValue().x ) + "," +
               toGLSL ( getValue().y ) + ")";
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
        comboSlider1->setDecimals(wd ? DDEC : FDEC);
        comboSlider2->setDecimals(wd ? DDEC : FDEC);
    };

    virtual void setSliderType ( SliderType st ){
        comboSlider1->setSliderType(st);
        comboSlider2->setSliderType(st);
        sliderType = st;
    };

private:

    ComboSlider* comboSlider1;
    ComboSlider* comboSlider2;
    glm::dvec2 defaultValue;
    glm::dvec2 min;
    glm::dvec2 max;
};


/// A widget editor for a float3 variable.
class Float3Widget : public VariableWidget
{
    Q_OBJECT
public:
    /// FloatVariable constructor.
    Float3Widget ( QWidget *parent, QWidget *variableEditor, QString name,
                   glm::dvec3 defaultValue, glm::dvec3 min, glm::dvec3 max, bool logarithmic );
    virtual QString getUniqueName();
    virtual QString getValueAsText()
    {
        int p = FDEC;
        if ( isDouble() ) {
            p = DDEC;
        }
        return QString::number ( comboSlider1->getValue(), 'g', p ) + "," +
               QString::number ( comboSlider2->getValue(), 'g', p ) + "," +
               QString::number ( comboSlider3->getValue(), 'g', p );
    }
    virtual QString toString();
    virtual bool fromString(QString string);
    glm::dvec3 getValue()
    {
        return glm::dvec3 ( comboSlider1->getValue(), comboSlider2->getValue(),
                           comboSlider3->getValue() );
    }
    void setValue(glm::dvec3 v);
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
    void reset()
    {
        setValue ( defaultValue );
    }
    QString getLockedSubstitution()
    {
        QString type = isDouble() ? "dvec3" : "vec3";
        return "const " + type + " " + name + " = " + type + "(" +
               toGLSL ( getValue().x ) + "," + toGLSL ( getValue().y ) + "," +
               toGLSL ( getValue().z ) + ");";
    }
    QString getLockedSubstitution2()
    {
        QString type = isDouble() ? " dvec3(" : " vec3(";
        return "#define " + name + type + toGLSL ( getValue().x ) + "," +
               toGLSL ( getValue().y ) + "," + toGLSL ( getValue().z ) + ")";
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
        comboSlider1->setDecimals(wd?DDEC:FDEC);
        comboSlider2->setDecimals(wd?DDEC:FDEC);
        comboSlider3->setDecimals(wd?DDEC:FDEC);
    };

    virtual void setSliderType ( SliderType st ){
        comboSlider1->setSliderType(st);
        comboSlider2->setSliderType(st);
        comboSlider3->setSliderType(st);
        sliderType = st;
    };

public slots:
    void n1Changed();
    void n2Changed();
    void n3Changed();
signals:
    void doneChanges();
private:
    bool normalize;
    ComboSlider* comboSlider1;
    ComboSlider* comboSlider2;
    ComboSlider* comboSlider3;
    glm::dvec3 defaultValue;
    glm::dvec3 min;
    glm::dvec3 max;
};

/// A widget editor for a float4 variable.
class Float4Widget : public VariableWidget
{
    Q_OBJECT
public:
    /// FloatVariable constructor.
    Float4Widget ( QWidget *parent, QWidget *variableEditor, QString name,
                   glm::dvec4 defaultValue, glm::dvec4 min, glm::dvec4 max, bool logarithmic );
    virtual QString getUniqueName();
    virtual QString getValueAsText()
    {
        int p = FDEC;
        if ( isDouble() ) {
            p = DDEC;
        }
        return QString::number ( comboSlider1->getValue(), 'g', p ) + "," +
               QString::number ( comboSlider2->getValue(), 'g', p ) + "," +
               QString::number ( comboSlider3->getValue(), 'g', p ) + "," +
               QString::number ( comboSlider4->getValue(), 'g', p );
    }
    virtual QString toString();
    virtual bool fromString(QString string);
    glm::dvec4 getValue()
    {
        return glm::dvec4 ( comboSlider1->getValue(), comboSlider2->getValue(),
                           comboSlider3->getValue(), comboSlider4->getValue() );
    }
    void setValue(glm::dvec4 v);
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
    void reset()
    {
        setValue ( defaultValue );
    }
    QString getLockedSubstitution()
    {
        QString type = isDouble() ? "dvec4" : "vec4";
        return "const " + type + " " + name + " = " + type + "(" +
               toGLSL ( getValue().x ) + "," + toGLSL ( getValue().y ) + "," +
               toGLSL ( getValue().z ) + "," + toGLSL ( getValue().w ) + ");";
    }
    QString getLockedSubstitution2()
    {
        QString type = isDouble() ? " dvec4(" : " vec4(";
        return "#define " + name + type + toGLSL ( getValue().x ) + "," +
               toGLSL ( getValue().y ) + "," + toGLSL ( getValue().z ) + "," +
               toGLSL ( getValue().w ) + ")";
    }
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
        comboSlider1->setDecimals(wd?DDEC:FDEC);
        comboSlider2->setDecimals(wd?DDEC:FDEC);
        comboSlider3->setDecimals(wd?DDEC:FDEC);
        comboSlider4->setDecimals(wd?DDEC:FDEC);
    };

    virtual void setSliderType ( SliderType st ){
        comboSlider1->setSliderType(st);
        comboSlider2->setSliderType(st);
        comboSlider3->setSliderType(st);
        comboSlider4->setSliderType(st);
        sliderType = st;
    };

signals:
    void doneChanges();
private:
    bool normalize;
    ComboSlider* comboSlider1;
    ComboSlider* comboSlider2;
    ComboSlider* comboSlider3;
    ComboSlider* comboSlider4;
    glm::dvec4 defaultValue;
    glm::dvec4 min;
    glm::dvec4 max;
};

class ColorWidget : public VariableWidget
{
public:
    /// FloatVariable constructor.
    ColorWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                  glm::dvec3 defaultValue );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2" ).arg ( group ).arg ( getName() );
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
private:
    ColorChooser* colorChooser;
    glm::dvec3 defaultValue;
};

class FloatColorWidget : public VariableWidget
{
public:
    /// FloatVariable constructor.
    FloatColorWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                       double defaultValue, double min, double max,
                       glm::dvec3 defaultColorValue );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3:%4" ).arg ( group ).arg ( getName() ).arg ( min ).arg ( max );
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
private:
    ComboSlider* comboSlider;
    ColorChooser* colorChooser;
    double defaultValue;
    double min;
    double max;
    glm::dvec3 defaultColorValue;
};

class IntWidget : public VariableWidget
{
public:
    /// IntVariable constructor.
    IntWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                int defaultValue, int min, int max );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3:%4" ).arg ( group ).arg ( getName() ).arg ( min ).arg ( max );
    }
    virtual QString getValueAsText()
    {
        return QString::number ( comboSlider->getValue() );
    };
    virtual QString toString();
    virtual bool fromString(QString string);
    int getValue()
    {
        return comboSlider->getValue();
    }
    void setValue ( int i )
    {
        comboSlider->setValue ( i );
    }
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
    void reset()
    {
        comboSlider->setValue ( defaultValue );
    }
    QString getLockedSubstitution()
    {
        return "const int " + name + " = " +
               QString::number ( comboSlider->getValue() ) + ";";
    };
    QString getLockedSubstitution2()
    {
        return "#define " + name + " " + QString::number ( comboSlider->getValue() ) +
               "";
    };
    void setIsDouble ( bool wd = false )
    {
        wantDouble = wd;
    };
private:
    IntComboSlider* comboSlider;
    int defaultValue;
    int min;
    int max;
};

class BoolWidget : public VariableWidget
{
public:
    /// BoolVariable constructor.
    BoolWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                 bool defaultValue );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2" ).arg ( group ).arg ( getName() );
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

private:
    QCheckBox* checkBox;
    bool defaultValue;
};


}
} // namespace Fragmentarium
