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

using namespace SyntopiaCore::Logging;

namespace Fragmentarium
{
namespace GUI
{

using namespace SyntopiaCore::Misc;
using namespace Fragmentarium::Parser;

VariableWidget::VariableWidget(QWidget *parent, QWidget *variableEditor, QString name)
    : QWidget(parent), name(name), updated(false), systemVariable(false), variableEditor(variableEditor), provenance(FromUnknown)
{

    auto *vl = new QHBoxLayout(this);
    vl->setSpacing(2);
    vl->setContentsMargins (0,0,0,0);
    setObjectName(name);
    lockButton = new QPushButton(this);
    lockButton->setFlat(true);
    lockButton->setStyleSheet("QPushButton {border: none; outline: none;}");
    lockButton->setIcon(QIcon(":/Icons/padlockb.png"));
    lockButton->setFixedSize(12,18);
    lockButton->setCheckable(true);
    lockButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    vl->addWidget(lockButton,0,Qt::AlignLeft | Qt::AlignVCenter);
    connect(lockButton, SIGNAL(toggled(bool)), this, SLOT(locked(bool)));

    QLabel* label = new QLabel(name,this);
    label->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
    vl->addWidget(label,0,Qt::AlignRight | Qt::AlignVCenter);

    widget = new QWidget(this);
    vl->addWidget(widget);
    connect(this, SIGNAL(changed(bool,Provenance)), variableEditor, SLOT(childChanged(bool,Provenance)));
}

bool VariableWidget::isLocked()
{
    return (lockType == Locked || lockType == AlwaysLocked);
}

void VariableWidget::valueChanged()
{
    if (lockType == Locked || lockType == AlwaysLocked ) {
        QPalette pal = palette();
        pal.setColor(backgroundRole(), Qt::yellow);
        setPalette(pal);
        setAutoFillBackground(true);
        emit(changed(true, getProvenance()));
    } else {
        emit(changed(false, getProvenance()));
    }
}

void VariableWidget::locked(bool l)
{
    if (defaultLockType == NotLockable) {
        lockButton->setIcon(QIcon());
        lockType = NotLockable;
        return;
    }

    if (l) {
        lockButton->setIcon(QIcon(":/Icons/padlocka.png"));
        lockType = Locked;
    } else {
        lockButton->setIcon(QIcon(":/Icons/padlockb.png"));
        lockType =  NotLocked;
    }

    valueChanged();
}

void VariableWidget::setDefaultLockType(LockType lt)
{
    defaultLockType = lt;
    setLockType(lt);
}

void VariableWidget::setLockType(LockType lt)
{
    if (lt == Locked || lt == AlwaysLocked) {
        locked(true);
    } else {
        locked(false);
    }
}

QString VariableWidget::toSettingsString()
{
    QString l;
    if (lockType != defaultLockType) {
        l = " " + lockType.toString();
    }
    return toString() + l;
}

bool VariableWidget::fromSettingsString(QString string)
{

    bool requiresRecompile = false;

    const QRegExp lockTypeString("(AlwaysLocked|Locked|NotLocked|NotLockable)\\s*.?$");
    if (lockTypeString.indexIn(string)!=-1) {
        QString s = lockTypeString.cap(1);
        string.remove(s);
        LockType before = lockType;
        lockType.fromString(s);
        if (before!=lockType) {
            requiresRecompile = true;
        }
    }

    requiresRecompile |= fromString(string.trimmed());

    if (requiresRecompile) {
        locked( lockType == Locked || lockType == AlwaysLocked);
    }

    return requiresRecompile;
}

int VariableWidget::uniformLocation(QOpenGLShaderProgram *shaderProgram)
{
    if (lockType == Locked) {
        return -1;
    }
    int l =  shaderProgram->uniformLocation(name);
    return l;
}

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

bool FloatWidget::fromString(QString string)
{
    double f;
    MiniParser(string).getDouble(f);
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

    comboSlider2 = new ComboSlider(parent, variableEditor, defaultValue.y,
                                   min.y, max.y, logarithmic);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(valueChanged()));
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

bool Float2Widget::fromString(QString string)
{
    double f1,f2;
    MiniParser(string).getDouble(f1).getDouble(f2);
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

    comboSlider2 = new ComboSlider(parent, variableEditor, defaultValue[1], min[1], max[1], logarithmic);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(n2Changed()));

    comboSlider3 = new ComboSlider(parent, variableEditor, defaultValue[2], min[2], max[2], logarithmic);
    comboSlider3->setObjectName( QString("%1%2").arg(name).arg("3") );
    m->addWidget(comboSlider3,2,1);
    connect(comboSlider3, SIGNAL(changed()), this, SLOT(n3Changed()));
    connect(this, SIGNAL(doneChanges()), this, SLOT(valueChanged()));

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

QString Float3Widget::getUniqueName()
{
    if (normalize) {
        return QString("%1:%2:%3:%4")
               .arg(group)
               .arg(getName())
               .arg("[0 0 0]")
               .arg("[0 0 0]");
    }

    QString f = QString("[%1 %2 %3]").arg(min.x).arg(min.y).arg(min.z);
    QString t = QString("[%1 %2 %3]").arg(max.x).arg(max.y).arg(max.z);

    return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
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

bool Float3Widget::fromString(QString string)
{
    double f1,f2,f3;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3);
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

    comboSlider2 = new ComboSlider(parent, variableEditor, defaultValue[1], min[1], max[1], logarithmic);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(valueChanged()));

    comboSlider3 = new ComboSlider(parent, variableEditor, defaultValue[2], min[2], max[2], logarithmic);
    comboSlider3->setObjectName( QString("%1%2").arg(name).arg("3") );
    m->addWidget(comboSlider3,2,1);
    connect(comboSlider3, SIGNAL(changed()), this, SLOT(valueChanged()));

    comboSlider4 = new ComboSlider(parent, variableEditor, defaultValue[3], min[3], max[3], logarithmic);
    comboSlider4->setObjectName( QString("%1%2").arg(name).arg("4") );
    m->addWidget(comboSlider4,3,1);
    connect(comboSlider4, SIGNAL(changed()), this, SLOT(valueChanged()));

}

void Float4Widget::setValue(glm::dvec4 v)
{
    comboSlider1->setValue(v.x);
    comboSlider2->setValue(v.y);
    comboSlider3->setValue(v.z);
    comboSlider4->setValue(v.w);
}

QString Float4Widget::getUniqueName()
{
    QString f = QString("[%1 %2 %3 %4]")
                .arg(min.x)
                .arg(min.y)
                .arg(min.z)
                .arg(min.w);
    QString t = QString("[%1 %2 %3 %4]")
                .arg(max.x)
                .arg(max.y)
                .arg(max.z)
                .arg(max.w);
    return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
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

bool Float4Widget::fromString(QString string)
{
    double f1,f2,f3,f4;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3).getDouble(f4);
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

bool ColorWidget::fromString(QString string)
{
    double f1,f2,f3;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3);
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

bool FloatColorWidget::fromString(QString string)
{
    if (toString() == string) {
        return false;
    }
    double f,f1,f2,f3;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3).getDouble(f);
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

}

QString IntWidget::toString()
{
    return QString("%1").arg(comboSlider->getValue());
}

bool IntWidget::fromString(QString string)
{
    if (comboSlider->getValue() == string.toInt()) {
        return false;
    }
    int i;
    MiniParser(string).getInt(i);
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

// SamplerWidget
// ------------------------------------------------------------------

SamplerWidget::SamplerWidget(FileManager *fileManager, QWidget *parent, QWidget *variableEditor, QString name, QString defaultValue)
    : VariableWidget(parent, variableEditor, name), fileManager(fileManager), defaultValue(defaultValue)
{

    auto *l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    comboBox = new QComboBox(parent);
    comboBox->setEditable(true);
    comboBox->addItems(fileManager->getImageFiles());
    comboBox->setEditText(defaultValue);
    comboBox->setObjectName(name);

    l->addWidget(comboBox);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    comboBox->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
    comboBox->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // necessarily!
    comboBox->view()->setCornerWidget(new QSizeGrip(comboBox));

    pushButton = new QPushButton("...", parent);
    l->addWidget(pushButton);
    pushButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    connect(comboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(pushButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    textChanged("");

    texID=0;
}

void SamplerWidget::textChanged(const QString &text)
{
    Q_UNUSED(text)
    if (!fileManager->fileExists(comboBox->currentText())) {
        QPalette pal = this->palette();
        pal.setColor(this->backgroundRole(), Qt::red);
        this->setPalette(pal);
        this->setAutoFillBackground(true);
    } else {
        this->setPalette(QApplication::palette(this));
        this->setAutoFillBackground(false);
    }
    //emit changed();
    valueChanged();
}

void SamplerWidget::buttonClicked()
{
    QStringList extensions;

    QList<QByteArray> a;
    a << "";
    a << "hdr";

#ifdef USE_OPEN_EXR
#ifdef Q_OS_WIN
    a << "exr";
#endif
#endif
    a << QImageReader::supportedImageFormats();
    foreach(QByteArray s, a) {
        extensions.append(QString(s));
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a Texture"), QString(), tr("Images (") + extensions.join(" *.") + tr(");;All (*.*)"));
    if (!fileName.isEmpty()) {
        comboBox->setEditText(fileName);
    }
}

QString SamplerWidget::toString()
{
    return comboBox->currentText();
}

QString SamplerWidget::getValue()
{
    return comboBox->currentText();
}

bool SamplerWidget::fromString(QString string)
{
//             INFO("'" + string + "'");
    if (toString() == string) {
        return false;
    }
    comboBox->setEditText(string.trimmed());
    return isLocked();
}

void SamplerWidget::setUserUniform(QOpenGLShaderProgram* shaderProgram)
{
    if (texID != 0) {
        int l = uniformLocation(shaderProgram);
        if(l != -1) {
            shaderProgram->setUniformValue(l, texID);
//  DBOUT << shaderProg->programId() << " TextureCache[" << texID << "] " << name;
        }
    }
}

void SamplerWidget::updateTextures(Parser::FragmentSource *fs,
                                   FileManager *fileManager)
{
    if (fs->textures.contains(name)) {
        QString fName;
        try {
            fName = fileManager->resolveName ( getValue() );
        } catch ( SyntopiaCore::Exceptions::Exception& e ) {
            CRITICAL ( e.getMessage() );
        }
        fs->textures[name] = fName;
        INFO ( tr("Setting texture to: ") + fs->textures[name] );
    } else {
        WARNING(tr("Weird, texture not found in fragment source: ") + name);
    }
}

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
}

QString BoolWidget::toString()
{
    return (checkBox->isChecked()?"true":"false");
}

bool BoolWidget::fromString(QString string)
{
    if (toString().toLower().trimmed() == string.toLower().trimmed()) {
        return false;
    }
    bool v = false;
    if (string.toLower().trimmed() == "true") {
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
