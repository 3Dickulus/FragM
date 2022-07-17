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
// BoolWidget -------------------------------------------------

BoolWidget::BoolWidget(QWidget *parent, QWidget *variableEditor, QString name, bool defaultValue)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue)
{

    auto *l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    checkBox = new QCheckBox(widget);
    checkBox->setChecked(defaultValue);
    checkBox->setObjectName( QString("%1").arg(name) );
    connect(checkBox, SIGNAL(clicked()),  this, SLOT(valueChanged()));
    l->addWidget(checkBox,1,Qt::AlignLeft | Qt::AlignVCenter);
    connect(this, SIGNAL(sliderBoundsChanged(QString)), this, SLOT(comboSliderBoundsChanged(QString)));
}

QString BoolWidget::toString()
{
    return (checkBox->isChecked()?"true":"false");
}

bool BoolWidget::fromString(QString str)
{
    if (toString().toLower().trimmed() == str.toLower().trimmed()) {
        return false;
    }
    bool v = false;
    if (str.toLower().trimmed() == "true") {
        v = true;
    }
    checkBox->setChecked(v);
    return isLocked();
}

void BoolWidget::setUserUniform(QOpenGLShaderProgram *shaderProgram)
{
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (checkBox->isChecked() ? 1 : 0));
    }
}

} // namespace GUI
} // namespace Fragmentarium
