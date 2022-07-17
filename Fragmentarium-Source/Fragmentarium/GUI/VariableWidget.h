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


}
} // namespace Fragmentarium
