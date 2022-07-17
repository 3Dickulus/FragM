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
#include "IntWidget.h"

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

/// ------------ IntWidget ---------------------

IntWidget::IntWidget(QWidget *parent, QWidget *variableEditor, QString name, int defaultValue, int min, int max)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)
{

    auto *l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    comboSlider = new IntComboSlider(parent, defaultValue, min, max);
    comboSlider->setObjectName(name);
    l->addWidget(comboSlider);
    connect(comboSlider, SIGNAL(changed()),  this, SLOT(valueChanged()));
    connect(comboSlider, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

}

QString IntWidget::toString()
{
    return QString("%1").arg(comboSlider->getValue());
}

bool IntWidget::fromString(QString str)
{
    if (comboSlider->getValue() == str.toInt()) {
        return false;
    }
    int i;
    MiniParser(str).getInt(i);
    if (i == getValue()) {
        return false;
    }

    comboSlider->setValue(i);
    return isLocked();
}

void IntWidget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (int)(comboSlider->getValue()));
    }
}

/// ------------ MenuWidget ---------------------
IntMenuWidget::IntMenuWidget(QWidget *parent, QWidget *variableEditor, QString name, int defaultValue, QStringList texts)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), texts ( texts )
{

    auto *l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    comboBox = new IntComboBox(parent, defaultValue, texts);
    comboBox->setObjectName(name);
    l->addWidget(comboBox);
    connect(comboBox, SIGNAL(changed()),  this, SLOT(valueChanged()));
    connect(this, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));

}

QString IntMenuWidget::toString()
{
    return QString("%1").arg(comboBox->getValue());
}

bool IntMenuWidget::fromString(QString str)
{
    if (comboBox->getValue() == str.toInt()) {
        return false;
    }
    int i;
    MiniParser(str).getInt(i);
    if (i == getValue()) {
        return false;
    }

    comboBox->setValue(i);
    
    return isLocked();
}

void IntMenuWidget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (int)(comboBox->getValue()));
    }
}

} // namespace GUI
} // namespace Fragmentarium
