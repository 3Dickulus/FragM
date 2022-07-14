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

class SubclassOfQStyledItemDelegate : public QStyledItemDelegate {

    virtual void paint(QPainter * painter_, const QStyleOptionViewItem & option_, const QModelIndex & index_) const
    {
//         QStyleOptionViewItem & refToNonConstOption = const_cast<QStyleOptionViewItem &>(option_);
//         refToNonConstOption.showDecorationSelected = false;
//         refToNonConstOption.state &= ~QStyle::State_Selected;

        QStyledItemDelegate::paint(painter_, option_, index_);
    }
};

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
        setMaximumSize(2048,20);
        setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

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
        slider->setMinimumSize(160,20);
        slider->setMaximumSize(2048,20);
        slider->setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
        l->addWidget(slider);

        spinner = new ExpSpinBox(this);
        spinner->setDecimals(FDEC);
        spinner->setMaximum(maximum);
        spinner->setMinimum(minimum);
        spinner->setValue(defaultValue);
        spinner->setKeyboardTracking(false);
        spinner->setGroupSeparatorShown(false);
        // Scientific
        spinner->setScientificFormat(false);
        spinner->setMinimumSize(FDEC*20,20);
        spinner->setMaximumSize(2048,20);
        spinner->setSizePolicy (QSizePolicy ( QSizePolicy::Minimum, QSizePolicy::Minimum ) );
       
        l->addWidget(spinner);
        spinner->update();

        connect ( spinner, SIGNAL ( valueChanged ( double ) ), this, SLOT ( spinnerChanged ( double ) ) );
        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));

        m_anim = new QPropertyAnimation(this,"value");
        m_loops = 1;
        m_pong = 0;
        m_framestart = 0;
        m_framefin = 0;
    }
    void setScientificFormat ( bool f )
    {
        spinner->setScientificFormat(f);
    }
    void setDecimals ( int d )
    {
        spinner->setDecimals ( d );
    }
    void contextMenuEvent ( QContextMenuEvent *ev)
    {

        QMenu contextMenu;
        QAction findInSource ( tr ( "Select source" ), &contextMenu );
        QAction editScaleAction ( tr ( "Edit scale" ), &contextMenu );
        QAction scientificFormatAction ( tr ( "Scientific" ), &contextMenu );
        QAction standardFormatAction ( tr ( "Standard" ), &contextMenu );
        QAction upperBoundAction ( tr ( "Upper Bound" ), &contextMenu );
        QAction lowerBoundAction ( tr ( "Lower Bound" ), &contextMenu );

        contextMenu.addAction ( &findInSource );
        contextMenu.addAction ( &editScaleAction );
        contextMenu.addAction ( &scientificFormatAction );
        contextMenu.addAction ( &standardFormatAction );
        contextMenu.addAction ( &upperBoundAction );
        contextMenu.addAction ( &lowerBoundAction );

        QAction *choice = contextMenu.exec ( ev->globalPos() );

        if ( choice == &editScaleAction ) {
            bool ok;
            int i = QInputDialog::getInt ( this, objectName(), tr ( "Slider Step Multiplier" ), slider->singleStep(), 1, 100000000, 1, &ok );
            if (ok) {
                slider->setSingleStep(i);
                slider->setPageStep(i*10);
            }

        } else if ( choice == &scientificFormatAction ) {
            spinner->setScientificFormat(true);
        } else if ( choice == &standardFormatAction ) {
            spinner->setScientificFormat(false);
        } else if ( choice == &upperBoundAction ) {
            bool ok = false;
            double i = QInputDialog::getDouble ( this, objectName(), tr ( "Slider Upper Bound" ), getMax(), -100000000, 100000000, 1, &ok );
            if (ok) {
                slider->setMaximum( (logarithmic ? std::log(std::abs(i)) : i)*scale );
                spinner->setMaximum(i);
                emit sliderBoundsChanged( objectName() + QString(" 2 %1").arg(i));
            }

        } else if ( choice == &lowerBoundAction ) {
            bool ok = false;
            double i = QInputDialog::getDouble ( this, objectName(), tr ( "Slider Lower Bound" ), getMin(), -100000000, 100000000, 1, &ok );
            if (ok) {
                slider->setMinimum( (logarithmic ? std::log(std::abs(i)) : i)*scale );
                spinner->setMinimum(i);
                emit sliderBoundsChanged( objectName() + QString(" 1 %1").arg(i) );
            }
            
        } else if ( choice == &findInSource ) {
            emit sliderBoundsChanged( objectName() + QString(" 0 0") );
        }

    }

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
    QPropertyAnimation* propertyAnimation()
    {
        return m_anim;
    }
    int getFrameStart()
    {
        return m_framestart;
    }
    int getFrameFin()
    {
        return m_framefin;
    }
    int getLoopDuration()
    {
        return m_framefin-m_framestart;
    }
    void setFrameStart( int s )
    {
        m_framestart=s;
    }
    void setFrameFin( int f )
    {
        m_framefin=f;
    }
    void setLoops( int l )
    {
        m_loops=l;
    }
    void setPong( int p )
    {
        m_pong = p;
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
    void sliderBoundsChanged(QString uniqueName);

protected slots:
    void spinnerChanged ( double d )
    {
        // our value comes from the spin box
        // the slider must be kept quiet while adjusting from spinner
        slider->blockSignals(true);

        if (logarithmic && ! ((d < 0 && minimum < 0 && maximum < 0 && defaultValue < 0) || (d > 0 && minimum > 0 && maximum > 0 && defaultValue > 0))) {
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
    ExpSpinBox* spinner;
    QWidget* variableEditor;
    double defaultValue;
    double minimum;
    double maximum;
    bool logarithmic;
    double scale;
    QPropertyAnimation* m_anim;
    int m_framestart, m_framefin; // firstframe lastframe
    int m_loops;
    int m_pong;

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


// A helper class (combined int slider+spinner)
class IntComboSlider : public QWidget
{
    Q_OBJECT
public:
    IntComboSlider(QWidget* parent, int defaultValue, int minimum, int maximum)
        : QWidget ( parent ), defaultValue ( defaultValue ), min ( minimum ), max ( maximum )
    {

        setMinimumSize(160,20);
        setMaximumSize(2048,20);
        setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

        QHBoxLayout* l = new QHBoxLayout(this);
        l->setSpacing(2);
        l->setContentsMargins(0,0,0,0);

        slider = new QSlider(Qt::Horizontal,this);
        slider->setRange(minimum,maximum);
        slider->setValue(defaultValue);
        slider->setMinimumSize(160,20);
        slider->setMaximumSize(2048,20);
        slider->setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
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
    void sliderBoundsChanged(QString uniqueName);
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


// A helper class (combined int menu+spinner)
class IntComboBox : public QWidget
{
    Q_OBJECT
public:
    IntComboBox(QWidget* parent, int defaultValue, QStringList texts)
        : QWidget ( parent ), defaultValue ( defaultValue ), texts ( texts )
    {

        setMinimumSize(160,20);
        setMaximumSize(2048,20);
        setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

        QHBoxLayout* l = new QHBoxLayout(this);
        l->setSpacing(2);
        l->setContentsMargins(0,0,0,0);

        comboBox = new QComboBox(this);
//         comboBox->setRange(minimum,maximum);
        comboBox->addItems(texts);
        comboBox->setCurrentIndex(defaultValue);

        comboBox->setMinimumSize(160,20);
        comboBox->setMaximumSize(2048,20);
        comboBox->setSizePolicy (QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
        l->addWidget(comboBox);

        spinner = new QSpinBox(this);
        spinner->setMaximum(texts.count()-1);
        spinner->setMinimum(0);
        spinner->setValue(defaultValue);
        spinner->setKeyboardTracking(false);
        l->addWidget(spinner);

        setSizePolicy ( QSizePolicy ( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

        connect (spinner, SIGNAL(valueChanged(int)), this, SLOT(spinnerChanged(int)));
        connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
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
    int getValue()
    {
        return spinner->value();
    }
public slots:
    void setValue ( int i )
    {
DBOUT << objectName() << i;
        spinner->setValue ( i );
    }

    void setValue ( QStringList list )
    {
        int index = comboBox->currentIndex();
        comboBox->clear();
        comboBox->addItems(list);
        comboBoxChanged ( index );
    }

signals:
    void changed();
    void sliderBoundsChanged(QString uniqueName);

protected slots:
    void spinnerChanged ( int val )
    {
        comboBox->blockSignals(true);
        comboBox->setCurrentIndex(val);
        comboBox->blockSignals(false);
        emit changed();
    }

    void comboBoxChanged ( int val )
    {
        spinner->blockSignals(true);
        spinner->setValue(val);
        spinner->blockSignals(false);
        emit changed();
    }

private:

    QComboBox* comboBox;
    QSpinBox* spinner;
    int defaultValue;
    QStringList texts;
};

class CameraControl;

/// Widget editor base class.
class VariableWidget : public QWidget
{

    Q_OBJECT
public:
    VariableWidget(QWidget* parent, QWidget* variableEditor, QString name);
    virtual void updateTexture ( FragmentSource * /*fs*/, FileManager * /*fileManager*/ ) {}
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
    void setLabelStyle(bool prov=false)
    {
       if (prov) {
            QColor c = label->palette().color(QPalette::Dark);
            label->setStyleSheet("* {border-style: outset; border-width: 1px; border-color: " + c.name() + "; border-radius: 2px;}");
        } else {
            label->setStyleSheet("* {border: none;}");
        }
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
    virtual void setSliderType(SliderType ) = 0;

public slots:
    void locked(bool l);
    void valueChanged();
    void comboSliderBoundsChanged( QString name );

signals:
    void changed(bool lockedChanged);

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

    QLabel* label;
    QPushButton* lockButton;
    QString name;
    QString group;
    bool updated;
    bool systemVariable;
    bool wantDouble=false;
    QWidget* widget;
    QWidget* variableEditor;
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
    virtual void updateTexture(FragmentSource* fs, FileManager* fileManager);
    virtual void setLockType ( LockType /*lt*/ )
    {
        lockType = AlwaysLocked;
    } // cannot change this
    
    QString getValue() ;
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3:%4" ).arg ( group ).arg ( getName() ).arg(defaultValue).arg(defaultChannelValue.join(";"));
    }
    void reset()
    {
        // updates channel combo box list from file, showing them (if EXR)
        comboBox->setEditText ( defaultValue );
        for (int channel = 0; channel < 4; ++channel)
        {
            // this test should always be true for EXR
            if(channel < defaultChannelValue.size() && ! defaultChannelValue[channel].isEmpty()) {
                channelComboBox[channel]->setCurrentText(defaultChannelValue[channel]);
            }
        }
    }
    QString getChannelValue(int channel)
    {
        if(!channelComboBox[channel]->isHidden())
            return channelComboBox[channel]->currentText();
        else return "";
    }
    QString getChannelValue()
    {
        QStringList l = QStringList();
        for (int channel = 0; channel < 4; ++channel) {
            QString c = getChannelValue(channel);
            if (! c.isEmpty()) {
                l += c;
            }
        }
        return l.join(";");
    }
    
    int hasChannel(int channel, QString chan);
    
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
    void setSliderType(SliderType ){};
    
    int texID;
    QStringList channelList;

public slots:

signals:
    void changed();

protected slots:

    void textChanged(const QString& text);
    void channelChanged(const QString& text);

    void buttonClicked();

private:

    QComboBox* comboBox;
    QComboBox* channelComboBox[4];
    QToolButton* toolButton;
    FileManager* fileManager;
    QString defaultValue;
    QStringList defaultChannelValue;
};

class iSamplerWidget : public SamplerWidget
{
    Q_OBJECT
public:
    iSamplerWidget ( FileManager *fileManager, QWidget *parent,
                    QWidget *variableEditor, QString name, QString defaultValue, QString defaultChannelValue="" );

signals:

protected slots:

private:

};

class uSamplerWidget : public SamplerWidget
{
    Q_OBJECT
public:
    uSamplerWidget ( FileManager *fileManager, QWidget *parent,
                    QWidget *variableEditor, QString name, QString defaultValue, QString defaultChannelValue="" );

signals:

protected slots:

private:

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
        return QString ( "%1:%2:%3:%4:%5" ).arg ( group ).arg ( getName() ).arg ( min ).arg (defaultValue).arg ( max );
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

    void setSliderType ( SliderType st ){
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
        QString d = QString("[%1 %2]").arg(defaultValue.x).arg(defaultValue.y);
        QString t = QString("[%1 %2]").arg(max.x).arg(max.y);
        return QString("%1:%2:%3:%4:%5").arg(group).arg(getName()).arg(f).arg(d).arg(t);
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

    void setSliderType ( SliderType st ){
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
    virtual QString getUniqueName(){
        QString f = QString("[%1 %2 %3]").arg(min.x).arg(min.y).arg(min.z);
        QString d = QString("[%1 %2 %3]").arg(defaultValue.x).arg(defaultValue.y).arg(defaultValue.z);
        QString t = QString("[%1 %2 %3]").arg(max.x).arg(max.y).arg(max.z);
        return QString("%1:%2:%3:%4:%5").arg(group).arg(getName()).arg(f).arg(d).arg(t);
    }
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

    void setSliderType ( SliderType st ){
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
    virtual QString getUniqueName(){
        QString f = QString("[%1 %2 %3 %4]").arg(min.x).arg(min.y).arg(min.z).arg(min.w);
        QString d = QString("[%1 %2 %3 %4]").arg(defaultValue.x).arg(defaultValue.y).arg(defaultValue.z).arg(defaultValue.w);
        QString t = QString("[%1 %2 %3 %4]").arg(max.x).arg(max.y).arg(max.z).arg(max.w);
        return QString("%1:%2:%3:%4:%5").arg(group).arg(getName()).arg(f).arg(d).arg(t);
    }
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

    void setSliderType ( SliderType st ){
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

class IntWidget : public VariableWidget
{
    Q_OBJECT
public:
    /// IntVariable constructor.
    IntWidget ( QWidget *parent, QWidget *variableEditor, QString name,
                int defaultValue, int min, int max );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3:%4:%5" ).arg ( group ).arg ( getName() ).arg ( min ).arg ( defaultValue ).arg ( max );
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
    void setSliderType(SliderType ){};

signals:
    void sliderBoundsChanged(QString uniqueName);

private:
    IntComboSlider* comboSlider;
    int defaultValue;
    int min;
    int max;
};

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


class IntMenuWidget : public VariableWidget
{
    Q_OBJECT
public:
    /// Variable constructor.
    IntMenuWidget ( QWidget *parent, QWidget *variableEditor, QString name, int defaultValue, QStringList texts );
    virtual QString getUniqueName()
    {
        return QString ( "%1:%2:%3" ).arg ( group ).arg ( getName() ).arg ( defaultValue );
    }
    virtual QString getValueAsText()
    {
        return QString::number ( comboBox->getValue() );
    };
    virtual QString toString();
    virtual bool fromString(QString string);
    int getValue()
    {
        return comboBox->getValue();
    }
    void setValue ( int i )
    {
DBOUT << objectName() << i;
        comboBox->setValue ( i );
    }
    virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
    void reset()
    {
        comboBox->setValue ( defaultValue );
    }
    QString getLockedSubstitution()
    {
        return "const int " + name + " = " +
               QString::number ( comboBox->getValue() ) + ";";
    };
    QString getLockedSubstitution2()
    {
        return "#define " + name + " " + QString::number ( comboBox->getValue() ) +
               "";
    };

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
    IntComboBox* comboBox;
    int defaultValue;
    int min;
    int max;
    QStringList texts;
};


}
} // namespace Fragmentarium
