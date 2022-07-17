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
/// E.g. the line: uniform int i; slider[0,1,2]
///	will make a simple editor widget appear.

namespace Fragmentarium
{
namespace GUI
{

using namespace SyntopiaCore::Logging;
using namespace Fragmentarium::Parser;

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
        QAction upperBoundAction ( tr ( "Upper Bound" ), &contextMenu );
        QAction lowerBoundAction ( tr ( "Lower Bound" ), &contextMenu );
        contextMenu.addAction ( &findInSource );
        contextMenu.addAction ( &upperBoundAction );
        contextMenu.addAction ( &lowerBoundAction );

        QAction *choice = contextMenu.exec ( ev->globalPos() );
        
         if ( choice == &upperBoundAction ) {
            bool ok = false;
            int i = QInputDialog::getInt ( this, objectName(), tr ( "Slider Upper Bound" ), getMax(), -100000000, 100000000, 1, &ok );
            if (ok) {
                slider->setMaximum(i);
                spinner->setMaximum(i);
                emit sliderBoundsChanged( objectName() + QString(" 2 %1").arg(i));
            }

        } else if ( choice == &lowerBoundAction ) {
            bool ok = false;
            int i = QInputDialog::getInt ( this, objectName(), tr ( "Slider Lower Bound" ), getMin(), -100000000, 100000000, 1, &ok );
            if (ok) {
                slider->setMinimum(i);
                spinner->setMinimum(i);
                emit sliderBoundsChanged( objectName() + QString(" 1 %1").arg(i) );
            }
            
        } else if ( choice == &findInSource ) {
            emit sliderBoundsChanged( objectName() + QString(" 0 0") );
        }
        
    }
    int getMin()
    {
        return min;
    }
    int getMax()
    {
        return max;
    }
    int getSpan()
    {
        return max - min;
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
