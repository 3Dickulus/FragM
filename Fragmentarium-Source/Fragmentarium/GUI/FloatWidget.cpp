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
#include "FloatWidget.h"

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

/// FloatVariable constructor.
FloatWidget::FloatWidget(QWidget *parent, QWidget *variableEditor, QString name, double defaultValue, double min, double max, bool logarithmic)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)
{
    auto *l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    comboSlider1 = new ComboSlider(parent, variableEditor, defaultValue, min, max, logarithmic);
    comboSlider1->setObjectName(QString("%1%2").arg(name).arg("1"));
    l->addWidget(comboSlider1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(comboSlider1, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));
}

void FloatWidget::setValue(double f)
{
    comboSlider1->setValue(f);
}

QString FloatWidget::toString()
{
    double f = comboSlider1->getValue();
    return QString::number(f,'g',(isDouble() ? DDEC : FDEC));
}

bool FloatWidget::fromString(QString str)
{
    double f;
    MiniParser(str).getDouble(f);
    if ( f == getValue() ) {
        return false;
    }
    setValue(f);
    return isLocked();
}

void FloatWidget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(comboSlider1->getValue()));
    }
}

//// ----- Float2Widget -----------------------------------------------

Float2Widget::Float2Widget(QWidget *parent, QWidget *variableEditor, QString name, glm::dvec2 defaultValue, glm::dvec2 min, glm::dvec2 max, bool logarithmic)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)
{
    auto *m = new QGridLayout(widget);
    m->setSpacing(2);
    m->setContentsMargins (0,0,0,0);

    comboSlider1 = new ComboSlider(parent, variableEditor, defaultValue.x,
                                   min.x, max.x, logarithmic);
    comboSlider1->setObjectName( QString("%1%2").arg(name).arg("1") );
    m->addWidget(comboSlider1,0,1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(comboSlider1, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

    comboSlider2 = new ComboSlider(parent, variableEditor, defaultValue.y,
                                   min.y, max.y, logarithmic);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(comboSlider2, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));
}

QString Float2Widget::toString()
{
    int p = FDEC;
    if (isDouble()) {
        p = DDEC;
    }
    return QString("%1,%2")
           .arg(QString::number(comboSlider1->getValue(),'g',p))
           .arg(QString::number(comboSlider2->getValue(),'g',p));
}

void Float2Widget::setValue(glm::dvec3 v)
{
    comboSlider1->setValue(v.x);
    comboSlider2->setValue(v.y);
}

bool Float2Widget::fromString(QString str)
{
    double f1,f2;
    MiniParser(str).getDouble(f1).getDouble(f2);
    glm::dvec2 f(f1,f2);

    if ( f == getValue() ) {
        return false;
    }

    comboSlider1->setValue(f1);
    comboSlider2->setValue(f2);
    return isLocked();
}

void Float2Widget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(comboSlider1->getValue()), (float)(comboSlider2->getValue()));
    }
}

//// ----- Float3Widget -----------------------------------------------

Float3Widget::Float3Widget(QWidget *parent, QWidget *variableEditor, QString name, glm::dvec3 defaultValue, glm::dvec3 min, glm::dvec3 max, bool logarithmic)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)
{

    normalize = false;

    if (min==max) {
        min = glm::dvec3(-1,-1,-1);
        max = glm::dvec3(1,1,1);
        if (name != "Up") {
            normalize = true;
        } // normalizing Up at the widget level interferes with spline path animating
    }

    auto *m = new QGridLayout(widget);
    m->setSpacing(2);
    m->setContentsMargins (0,0,0,0);

    comboSlider1 = new ComboSlider(parent, variableEditor, defaultValue[0], min[0], max[0], logarithmic);
    comboSlider1->setObjectName( QString("%1%2").arg(name).arg("1") );
    m->addWidget(comboSlider1,0,1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(n1Changed()));
    connect(comboSlider1, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

    comboSlider2 = new ComboSlider(parent, variableEditor, defaultValue[1], min[1], max[1], logarithmic);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(n2Changed()));
    connect(comboSlider2, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

    comboSlider3 = new ComboSlider(parent, variableEditor, defaultValue[2], min[2], max[2], logarithmic);
    comboSlider3->setObjectName( QString("%1%2").arg(name).arg("3") );
    m->addWidget(comboSlider3,2,1);
    connect(comboSlider3, SIGNAL(changed()), this, SLOT(n3Changed()));
    connect(this, SIGNAL(doneChanges()), this, SLOT(valueChanged()));
    connect(comboSlider3, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

}

void Float3Widget::setValue(glm::dvec3 v)
{
    comboSlider1->setValue(v.x);
    comboSlider2->setValue(v.y);
    comboSlider3->setValue(v.z);
}

void Float3Widget::n1Changed()
{
    if (normalize) {
        comboSlider2->blockSignals(true);
        comboSlider3->blockSignals(true);
        double x = comboSlider1->getValue();
        double y = comboSlider2->getValue();
        double z = comboSlider3->getValue();
        double a = sqrt((1.0-x*x)/(y*y+z*z));
        if (z*z+y*y == 0) {
            a = 0;
            comboSlider1->blockSignals(true);
            comboSlider1->setValue((x>0) ? 1.0 : -1.0);
            comboSlider1->blockSignals(false);
        }
        comboSlider2->setValue(y*a);
        comboSlider3->setValue(z*a);
        comboSlider2->blockSignals(false);
        comboSlider3->blockSignals(false);
    }
    emit(doneChanges());
}

void Float3Widget::n2Changed()
{
    if (normalize) {
        comboSlider1->blockSignals(true);
        comboSlider3->blockSignals(true);
        double x = comboSlider1->getValue();
        double y = comboSlider2->getValue();
        double z = comboSlider3->getValue();
        double a = sqrt((1.0-y*y)/(z*z+x*x));
        if (z*z+x*x == 0) {
            a = 0;
            comboSlider2->blockSignals(true);
            comboSlider2->setValue((y>0) ? 1.0 : -1.0);
            comboSlider2->blockSignals(false);
        }
        comboSlider1->setValue(x*a);
        comboSlider3->setValue(z*a);
        comboSlider1->blockSignals(false);
        comboSlider3->blockSignals(false);
    }
    emit(doneChanges());
}

void Float3Widget::n3Changed()
{
    if (normalize) {
        comboSlider1->blockSignals(true);
        comboSlider2->blockSignals(true);
        double x = comboSlider1->getValue();
        double y = comboSlider2->getValue();
        double z = comboSlider3->getValue();
        double a = sqrt((1.0-z*z)/(y*y+x*x));
        if (y*y+x*x == 0) {
//         if (fabs(y*y+x*x)<1E-8) {
            a = 0;
            comboSlider3->blockSignals(true);
            comboSlider3->setValue((z>0) ? 1.0 : -1.0);
            comboSlider3->blockSignals(false);
        }
        comboSlider1->setValue(x*a);
        comboSlider2->setValue(y*a);
        comboSlider1->blockSignals(false);
        comboSlider2->blockSignals(false);
    }
    emit(doneChanges());
}

QString Float3Widget::toString()
{
    int p = FDEC;
    if (isDouble()) {
        p = DDEC;
    }
    return QString("%1,%2,%3")
           .arg(QString::number(comboSlider1->getValue(),'g',p))
           .arg(QString::number(comboSlider2->getValue(),'g',p))
           .arg(QString::number(comboSlider3->getValue(),'g',p));
}

bool Float3Widget::fromString(QString str)
{
    double f1,f2,f3;
    MiniParser(str).getDouble(f1).getDouble(f2).getDouble(f3);
    glm::dvec3 f(f1,f2,f3);

    if ( f == getValue() ) {
        return false;
    }
    comboSlider1->setValue(f1);
    comboSlider2->setValue(f2);
    comboSlider3->setValue(f3);
    return isLocked();
}

void Float3Widget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(comboSlider1->getValue()),
                                       (float)(comboSlider2->getValue()),
                                       (float)(comboSlider3->getValue()));
    }
}

//// ----- Float4Widget -----------------------------------------------

Float4Widget::Float4Widget(QWidget *parent, QWidget *variableEditor, QString name, glm::dvec4 defaultValue, glm::dvec4 min, glm::dvec4 max, bool logarithmic)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue),
      min(min), max(max)
{

    auto *m = new QGridLayout(widget);

    m->setSpacing(2);
    m->setContentsMargins (0,0,0,0);

    normalize = false;

    if (min==max) {
        min = glm::dvec4(-1,-1,-1,-1);
        max = glm::dvec4(1,1,1,1);
        normalize = true;
    }

    comboSlider1 = new ComboSlider(parent, variableEditor, defaultValue[0], min[0], max[0], logarithmic);
    comboSlider1->setObjectName( QString("%1%2").arg(name).arg("1") );
    m->addWidget(comboSlider1,0,1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(comboSlider1, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

    comboSlider2 = new ComboSlider(parent, variableEditor, defaultValue[1], min[1], max[1], logarithmic);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(comboSlider2, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

    comboSlider3 = new ComboSlider(parent, variableEditor, defaultValue[2], min[2], max[2], logarithmic);
    comboSlider3->setObjectName( QString("%1%2").arg(name).arg("3") );
    m->addWidget(comboSlider3,2,1);
    connect(comboSlider3, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(comboSlider3, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

    comboSlider4 = new ComboSlider(parent, variableEditor, defaultValue[3], min[3], max[3], logarithmic);
    comboSlider4->setObjectName( QString("%1%2").arg(name).arg("4") );
    m->addWidget(comboSlider4,3,1);
    connect(comboSlider4, SIGNAL(changed()), this, SLOT(valueChanged()));
    connect(comboSlider4, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

}

void Float4Widget::setValue(glm::dvec4 v)
{
    comboSlider1->setValue(v.x);
    comboSlider2->setValue(v.y);
    comboSlider3->setValue(v.z);
    comboSlider4->setValue(v.w);
}

QString Float4Widget::toString()
{
    int p = FDEC;
    if (isDouble()) {
        p = DDEC;
    }
    return QString("%1,%2,%3,%4")
           .arg(QString::number(comboSlider1->getValue(),'g',p))
           .arg(QString::number(comboSlider2->getValue(),'g',p))
           .arg(QString::number(comboSlider3->getValue(),'g',p))
           .arg(QString::number(comboSlider4->getValue(),'g',p));
}

bool Float4Widget::fromString(QString str)
{
    double f1,f2,f3,f4;
    MiniParser(str).getDouble(f1).getDouble(f2).getDouble(f3).getDouble(f4);
    glm::dvec4 f(f1,f2,f3,f4);

    if ( f == getValue() ) {
        return false;
    }
    comboSlider1->setValue(f1);
    comboSlider2->setValue(f2);
    comboSlider3->setValue(f3);
    comboSlider4->setValue(f4);
    return isLocked();
}

void Float4Widget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue( l,
                                        (float)(comboSlider1->getValue()),
                                        (float)(comboSlider2->getValue()),
                                        (float)(comboSlider3->getValue()),
                                        (float)(comboSlider4->getValue()));
    }
}

} // namespace GUI
} // namespace Fragmentarium
