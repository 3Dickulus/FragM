#include <QApplication>
#include <QClipboard>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QSizeGrip>
#include <QSlider>
#include <QString>
#include <QToolButton>

#include "VariableWidget.h"
#include "ColorWidget.h"

#include "../../SyntopiaCore/Logging/ListWidgetLogger.h"
#include "../../SyntopiaCore/Misc/MiniParser.h"
#include "MainWindow.h"
#include "VariableEditor.h"
#include "TextEdit.h"
#include "Preprocessor.h"

using namespace SyntopiaCore::Logging;

namespace Fragmentarium
{
namespace GUI
{

using namespace SyntopiaCore::Misc;
using namespace Fragmentarium::Parser;

//////////////////////////////////////////////////////////
/// ------------ ColorWidget ---------------------------------------

ColorWidget::ColorWidget(QWidget *parent, QWidget *variableEditor, QString name, glm::dvec3 defaultValue)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue)
{

    auto *l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    colorChooser = new ColorChooser(parent, defaultValue);
    colorChooser->setObjectName( QString("%1").arg(name) );
    colorChooser->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));

    l->addWidget(colorChooser);
    connect(colorChooser, SIGNAL(changed()),  this, SLOT(valueChanged()));
    QApplication::postEvent(widget, new QEvent(QEvent::LayoutRequest));
    connect(colorChooser, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

}

QString ColorWidget::toString()
{
    int p = FDEC;
    if (isDouble()) {
        p = DDEC;
    }

    glm::dvec3 c = colorChooser->getValue();

    return QString("%1,%2,%3")
           .arg(QString::number(c.x,'g',p))
           .arg(QString::number(c.y,'g',p))
           .arg(QString::number(c.z,'g',p));
}

bool ColorWidget::fromString(QString str)
{
    double f1,f2,f3;
    MiniParser(str).getDouble(f1).getDouble(f2).getDouble(f3);
    glm::dvec3 c(f1,f2,f3);
    if ( c == getValue() ) {
        return false;
    }
    colorChooser->setColor(c);
    return isLocked();
}

void ColorWidget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        glm::dvec3 c = colorChooser->getValue();
        shaderProgram->setUniformValue(l, c.x, c.y, c.z);
    }
}


/// FloatColorWidget constructor.
FloatColorWidget::FloatColorWidget(QWidget *parent, QWidget *variableEditor, QString name, double defaultValue, double min, double max, glm::dvec3 defaultColorValue)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max), defaultColorValue(defaultColorValue)
{
    auto *l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    comboSlider = new ComboSlider(parent,  variableEditor, defaultValue, min, max);
    l->addWidget(comboSlider);
    connect(comboSlider, SIGNAL(changed()), this, SLOT(valueChanged()));
    comboSlider->setObjectName( QString("%1Slider").arg(name) );

    colorChooser = new ColorChooser(parent, defaultColorValue);
    colorChooser->setFixedSize(60,20);
    colorChooser->setObjectName( QString("%1Chooser").arg(name) );
    l->addWidget(colorChooser);
    connect(colorChooser, SIGNAL(changed()),  this, SLOT(valueChanged()));
    QApplication::postEvent(widget, new QEvent(QEvent::LayoutRequest));
    connect(comboSlider, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));
}

QString FloatColorWidget::toString()
{
    int p = FDEC;
    if (isDouble()) {
        p = DDEC;
    }
    glm::dvec3 c = colorChooser->getValue();
    double v = comboSlider->getValue();
    return QString("%1,%2,%3,%4")
           .arg(QString::number(c.x,'g',p))
           .arg(QString::number(c.y,'g',p))
           .arg(QString::number(c.z,'g',p))
           .arg(QString::number( v,'g',p));

}

bool FloatColorWidget::fromString(QString str)
{
    if (toString() == str) {
        return false;
    }
    double f,f1,f2,f3;
    MiniParser(str).getDouble(f1).getDouble(f2).getDouble(f3).getDouble(f);
    glm::dvec3 c(f1,f2,f3);
    if ( glm::dvec4(c,f) == getValue() ) {
        return false;
    }
    colorChooser->setColor(c);
    comboSlider->setDecimals(isDouble() ? DDEC : FDEC);
    comboSlider->setValue(f);
    return isLocked();
}

void FloatColorWidget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        glm::dvec3 c = colorChooser->getValue();
        double v = comboSlider->getValue();
        shaderProgram->setUniformValue(l,  c.x, c.y, c.z, (float)v );
    }
}

} // namespace GUI
} // namespace Fragmentarium
