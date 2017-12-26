#include "VariableWidget.h"

#include <QString>
#include <QPushButton>
#include <QSizeGrip>
#include <QToolButton>
#include <QFileDialog>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QScrollArea>
#include <QClipboard>
#include "../../SyntopiaCore/Logging/ListWidgetLogger.h"
#include "../../SyntopiaCore/Misc/MiniParser.h"
#include "MainWindow.h"

using namespace SyntopiaCore::Logging;

namespace Fragmentarium {
namespace GUI {

using namespace SyntopiaCore::Misc;
using namespace Fragmentarium::Parser;

VariableWidget::VariableWidget(QWidget* parent, QWidget* variableEditor, QString name) : QWidget(parent), name(name),  updated(false), systemVariable(false), variableEditor(variableEditor)  {

    QHBoxLayout* vl = new QHBoxLayout(this);
    vl->setSpacing(2);
    vl->setContentsMargins (0,0,0,0);
    setObjectName(name);
    lockButton = new QPushButton(this);
    lockButton->setFlat(true);
    lockButton->setStyleSheet("QPushButton {border: none; outline: none;}");
    lockButton->setIcon(QIcon(":/Icons/padlockb.png"));
    lockButton->setFixedSize(22,16);
    lockButton->setCheckable(true);
    vl->addWidget(lockButton,0,Qt::AlignLeft | Qt::AlignTop);
    connect(lockButton, SIGNAL(toggled(bool)), this, SLOT(locked(bool)));
    
    QLabel* label = new QLabel(name,this);
    vl->addWidget(label,0,Qt::AlignLeft | Qt::AlignTop);
    
    widget = new QWidget(this);
    vl->addWidget(widget);
    connect(this, SIGNAL(changed(bool)), variableEditor, SLOT(childChanged(bool)));

}

bool VariableWidget::isLocked() {
    return (lockType == Locked || lockType == AlwaysLocked);
}

void VariableWidget::valueChanged() {

    if (lockType == Locked || lockType == AlwaysLocked ) {
        QPalette pal = palette();
        pal.setColor(backgroundRole(), Qt::yellow);
        setPalette(pal);
        setAutoFillBackground(true);
        emit(changed(true));
    } else {
        emit(changed(false));
    }
}


void VariableWidget::locked(bool l) {
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

void VariableWidget::setDefaultLockType(LockType lt) {
    defaultLockType = lt;
    setLockType(lt);
}

void VariableWidget::setLockType(LockType lt) {
    if (lt == Locked) {
        locked(true);
    } else {
        locked(false);
    }
}

QString VariableWidget::toSettingsString() {
    QString l;
    if (lockType != defaultLockType) {
        l = " " + lockType.toString();
    }
    return toString() + l;
}

bool VariableWidget::fromSettingsString(QString string) {

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

    fromString(string);

    if (requiresRecompile) {
        locked( lockType == Locked );
    }

    return requiresRecompile;
}

int VariableWidget::uniformLocation(QOpenGLShaderProgram* shaderProgram) {
    if (lockType == Locked) return -1;
    return shaderProgram->uniformLocation(name);
}

/// FloatVariable constructor.
FloatWidget::FloatWidget(QWidget* parent, QWidget* variableEditor, QString name, double defaultValue, double min, double max)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)  {
    QHBoxLayout* l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);

    comboSlider1 = new ComboSlider(parent, variableEditor, defaultValue, min, max);
    comboSlider1->setObjectName(QString("%1%2").arg(name).arg("1"));
    l->addWidget(comboSlider1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(valueChanged()));
};

void FloatWidget::setValue(double f) {
    comboSlider1->setValue(f);
}

QString FloatWidget::toString() {
    double f = comboSlider1->getValue();
    return QString::number(f,'g',(isDouble() ? DDEC : FDEC));
}

void FloatWidget::fromString(QString string) {
    double f;
    MiniParser(string).getDouble(f);
    if ( f == getValue() ) {
        return;
    }
    setValue(f);
}

void FloatWidget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(comboSlider1->getValue()));
    }
}

//// ----- Float2Widget -----------------------------------------------

Float2Widget::Float2Widget(QWidget* parent, QWidget* variableEditor, QString name, QVector2D defaultValue, QVector2D min, QVector2D max)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)  {
    QGridLayout* m = new QGridLayout(widget);
    m->setSpacing(2);
    m->setContentsMargins (0,0,0,0);

    comboSlider1 = new ComboSlider(parent, variableEditor, defaultValue.x(), min.x(), max.x());
    comboSlider1->setObjectName( QString("%1%2").arg(name).arg("1") );
    m->addWidget(comboSlider1,0,1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(valueChanged()));

    comboSlider2 = new ComboSlider(parent,  variableEditor, defaultValue.y(), min.y(), max.y());
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(valueChanged()));
}


QString Float2Widget::toString() {
    int p = FDEC;
    if(isDouble()) p = DDEC;
    return QString("%1,%2")
           .arg(QString::number(comboSlider1->getValue(),'g',p))
           .arg(QString::number(comboSlider2->getValue(),'g',p));
}

void Float2Widget::setValue(QVector3D v) {
    comboSlider1->setValue(v.x());
    comboSlider2->setValue(v.y());
}

void Float2Widget::fromString(QString string) {
    double f1,f2;
    MiniParser(string).getDouble(f1).getDouble(f2);
    setValue(QVector2D(f1,f2));
}

void Float2Widget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(comboSlider1->getValue()),(float)(comboSlider2->getValue()));
    }
}

//// ----- Float3Widget -----------------------------------------------

Float3Widget::Float3Widget(QWidget* parent, QWidget* variableEditor, QString name, QVector3D defaultValue, QVector3D min, QVector3D max)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)  {

    normalize = false;

    if (min==max) {
        min = QVector3D(-1,-1,-1);
        max = QVector3D(1,1,1);
        if(name != "Up") normalize = true; // normalizing Up at the widget level interferes with spline path animating
    }

    QGridLayout* m = new QGridLayout(widget);
    m->setSpacing(2);
    m->setContentsMargins (0,0,0,0);

    comboSlider1 = new ComboSlider(parent,  variableEditor, defaultValue[0], min[0], max[0]);
    comboSlider1->setObjectName( QString("%1%2").arg(name).arg("1") );
    m->addWidget(comboSlider1,0,1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(n1Changed()));

    comboSlider2 = new ComboSlider(parent,  variableEditor, defaultValue[1], min[1], max[1]);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(n2Changed()));

    comboSlider3 = new ComboSlider(parent,  variableEditor, defaultValue[2], min[2], max[2]);
    comboSlider3->setObjectName( QString("%1%2").arg(name).arg("3") );
    m->addWidget(comboSlider3,2,1);
    connect(comboSlider3, SIGNAL(changed()), this, SLOT(n3Changed()));
    connect(this, SIGNAL(doneChanges()), this, SLOT(valueChanged()));

}

void Float3Widget::setValue(QVector3D v) {
    comboSlider1->setValue(v.x());
    comboSlider2->setValue(v.y());
    comboSlider3->setValue(v.z());
}


void Float3Widget::n1Changed() {
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

void Float3Widget::n2Changed() {
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

void Float3Widget::n3Changed() {
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

QString Float3Widget::getUniqueName() {
    if (normalize) {
        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg("[0 0 0]").arg("[0 0 0]");
    } else {

        QString f = QString("[%1 %2 %3]").arg(min.x()).arg(min.y()).arg(min.z());
        QString t = QString("[%1 %2 %3]").arg(max.x()).arg(max.y()).arg(max.z());

        return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
    }
}

QString Float3Widget::toString() {
    int p = FDEC;
    if(isDouble()) p = DDEC;
    return QString("%1,%2,%3")
           .arg(QString::number(comboSlider1->getValue(),'g',p))
           .arg(QString::number(comboSlider2->getValue(),'g',p))
           .arg(QString::number(comboSlider3->getValue(),'g',p));
}

void Float3Widget::fromString(QString string) {
    double f1,f2,f3;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3);
    setValue(QVector3D(f1,f2,f3));
}

void Float3Widget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(comboSlider1->getValue()),(float)(comboSlider2->getValue()),(float)(comboSlider3->getValue()));
    }
}

//// ----- Float4Widget -----------------------------------------------

Float4Widget::Float4Widget(QWidget* parent, QWidget* variableEditor, QString name, QVector4D defaultValue, QVector4D min, QVector4D max)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)  {
      
    QGridLayout* m = new QGridLayout(widget);

    m->setSpacing(2);
    m->setContentsMargins (0,0,0,0);

    normalize = false;

    if (min==max) {
      min = QVector4D(-1,-1,-1,-1);
      max = QVector4D(1,1,1,1);
      normalize = true;
    }
    
    comboSlider1 = new ComboSlider(parent,  variableEditor, defaultValue[0], min[0], max[0]);
    comboSlider1->setObjectName( QString("%1%2").arg(name).arg("1") );
    m->addWidget(comboSlider1,0,1);
    connect(comboSlider1, SIGNAL(changed()), this, SLOT(valueChanged()));

    comboSlider2 = new ComboSlider(parent,  variableEditor, defaultValue[1], min[1], max[1]);
    comboSlider2->setObjectName( QString("%1%2").arg(name).arg("2") );
    m->addWidget(comboSlider2,1,1);
    connect(comboSlider2, SIGNAL(changed()), this, SLOT(valueChanged()));

    comboSlider3 = new ComboSlider(parent,  variableEditor, defaultValue[2], min[2], max[2]);
    comboSlider3->setObjectName( QString("%1%2").arg(name).arg("3") );
    m->addWidget(comboSlider3,2,1);
    connect(comboSlider3, SIGNAL(changed()), this, SLOT(valueChanged()));

    comboSlider4 = new ComboSlider(parent,  variableEditor, defaultValue[3], min[3], max[3]);
    comboSlider4->setObjectName( QString("%1%2").arg(name).arg("4") );
    m->addWidget(comboSlider4,3,1);
    connect(comboSlider4, SIGNAL(changed()), this, SLOT(valueChanged()));
    
}

void Float4Widget::setValue(QVector4D v) {
    comboSlider1->setValue(v.x());
    comboSlider2->setValue(v.y());
    comboSlider3->setValue(v.z());
    comboSlider4->setValue(v.w());
}

QString Float4Widget::getUniqueName() {
                QString f = QString("[%1 %2 %3 %4]").arg(min.x()).arg(min.y()).arg(min.z()).arg(min.w());
                QString t = QString("[%1 %2 %3 %4]").arg(max.x()).arg(max.y()).arg(max.z()).arg(max.w());
                return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
}

QString Float4Widget::toString() {
    int p = FDEC;
    if(isDouble()) p = DDEC;
    return QString("%1,%2,%3,%4")
           .arg(QString::number(comboSlider1->getValue(),'g',p))
           .arg(QString::number(comboSlider2->getValue(),'g',p))
           .arg(QString::number(comboSlider3->getValue(),'g',p))
           .arg(QString::number(comboSlider4->getValue(),'g',p));
}

void Float4Widget::fromString(QString string) {
    double f1,f2,f3,f4;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3).getDouble(f4);
    setValue(QVector4D(f1,f2,f3,f4));
}

void Float4Widget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(comboSlider1->getValue()),(float)(comboSlider2->getValue()),(float)(comboSlider3->getValue()),(float)(comboSlider4->getValue()));
    }
}


/// ------------ ColorWidget ---------------------------------------

ColorWidget::ColorWidget(QWidget* parent, QWidget* variableEditor, QString name, QVector3D defaultValue)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue)  {
      
    QHBoxLayout* l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);
    
    colorChooser = new ColorChooser(parent, defaultValue);
    colorChooser->setObjectName( QString("%1").arg(name) );
    l->addWidget(colorChooser);
    connect(colorChooser, SIGNAL(changed()),  this, SLOT(valueChanged()));
    QApplication::postEvent(widget, new QEvent(QEvent::LayoutRequest));
}

QString ColorWidget::toString() {
    int p = FDEC;
    if(isDouble()) p = DDEC;
    return QString("%1,%2,%3")
           .arg(QString::number(colorChooser->getValue()[0],'g',p))
           .arg(QString::number(colorChooser->getValue()[1],'g',p))
           .arg(QString::number(colorChooser->getValue()[2],'g',p));
}

void ColorWidget::fromString(QString string) {
    double f1,f2,f3;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3);
    QVector3D c(f1,f2,f3);
    colorChooser->setColor(c);
}

void ColorWidget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(colorChooser->getValue()[0])
                                       ,(float)(colorChooser->getValue()[1]),(float)(colorChooser->getValue()[2]));
    }
}


/// FloatColorWidget constructor.
FloatColorWidget::FloatColorWidget(QWidget* parent, QWidget* variableEditor, QString name, double defaultValue, double min, double max, QVector3D defaultColorValue)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max), defaultColorValue(defaultColorValue)  {

    QHBoxLayout* l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);
    
    comboSlider = new ComboSlider(parent,  variableEditor, defaultValue, min, max);
    l->addWidget(comboSlider);
    connect(comboSlider, SIGNAL(changed()), this, SLOT(valueChanged()));
    comboSlider->setObjectName( QString("%1Slider").arg(name) );

    colorChooser = new ColorChooser(parent, defaultColorValue);
    colorChooser->setMinimumHeight(5);
    colorChooser->setMinimumWidth(30);
    colorChooser->setObjectName( QString("%1Chooser").arg(name) );
    l->addWidget(colorChooser);
    connect(colorChooser, SIGNAL(changed()),  this, SLOT(valueChanged()));
    QApplication::postEvent(widget, new QEvent(QEvent::LayoutRequest));
}


QString FloatColorWidget::toString() {
    int p = FDEC;
    if(isDouble()) p = DDEC;
    return QString("%1,%2,%3,%4")
           .arg(QString::number(colorChooser->getValue().x(),'g',p))
           .arg(QString::number(colorChooser->getValue().y(),'g',p))
           .arg(QString::number(colorChooser->getValue().z(),'g',p))
           .arg(QString::number(comboSlider->getValue(),'g',p));

}

void FloatColorWidget::fromString(QString string) {
    double f,f1,f2,f3;
    MiniParser(string).getDouble(f1).getDouble(f2).getDouble(f3).getDouble(f);
    QVector3D c(f1,f2,f3);
    colorChooser->setColor(c);
    comboSlider->setDecimals(isDouble() ? DDEC : FDEC);
    comboSlider->setValue(f);
}

void FloatColorWidget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (float)(colorChooser->getValue()[0]),
                                          (float)(colorChooser->getValue()[1]),
                                          (float)(colorChooser->getValue()[2]),
                                          (float)(comboSlider->getValue())
                                      );
    }
}

/// ------------ IntWidget ---------------------

IntWidget::IntWidget(QWidget* parent, QWidget* variableEditor, QString name, int defaultValue, int min, int max)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue), min(min), max(max)  {
      
    QHBoxLayout* l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);
    
    comboSlider = new IntComboSlider(parent, defaultValue, min, max);
    comboSlider->setObjectName(name);
    l->addWidget(comboSlider);
    connect(comboSlider, SIGNAL(changed()),  this, SLOT(valueChanged()));

}

QString IntWidget::toString() {
    return QString("%1").arg(comboSlider->getValue());
}

void IntWidget::fromString(QString string) {
    int i;
    MiniParser(string).getInt(i);
    comboSlider->setValue(i);
}

void IntWidget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (int)(comboSlider->getValue()));
    }
}

// SamplerWidget ------------------------------------------------------------------

SamplerWidget::SamplerWidget(FileManager* fileManager, QWidget* parent, QWidget* variableEditor,QString name, QString defaultValue)
    : VariableWidget(parent, variableEditor, name), fileManager(fileManager), defaultValue(defaultValue) {
      
    QHBoxLayout* l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);
    
    comboBox = new QComboBox(parent);
    comboBox->setEditable(true);
    comboBox->addItems(fileManager->getImageFiles());
    comboBox->setEditText(defaultValue);
    comboBox->setObjectName(name);

    l->addWidget(comboBox);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    comboBox->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum));
    comboBox->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // necessarily!
    comboBox->view()->setCornerWidget(new QSizeGrip(comboBox));
    
    pushButton = new QPushButton("...", parent);
    l->addWidget(pushButton);
    pushButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum));
    connect(comboBox, SIGNAL(editTextChanged(const QString& )), this, SLOT(textChanged(const QString& )));
    connect(pushButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    textChanged("");

}

void SamplerWidget::textChanged(const QString& ) {
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

void SamplerWidget::buttonClicked() {
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a Texture"),
                       QString(),
                       tr("Images (") + extensions.join(" *.") + tr(");;All (*.*)"));
    if (!fileName.isEmpty()) comboBox->setEditText(fileName);
}

QString SamplerWidget::toString() {
    return QString("%1").arg(comboBox->currentText());
}

QString SamplerWidget::getValue() {
    return comboBox->currentText();
}

void SamplerWidget::fromString(QString string) {
//             INFO("'" + string + "'");
    comboBox->setEditText(string.trimmed());
}

void SamplerWidget::setUserUniform(QOpenGLShaderProgram* /*shaderProgram*/) {

}

void SamplerWidget::updateTextures(Parser::FragmentSource* fs,  FileManager* fileManager) {
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

BoolWidget::BoolWidget(QWidget* parent, QWidget* variableEditor, QString name, bool defaultValue)
    : VariableWidget(parent, variableEditor, name), defaultValue(defaultValue)  {
      
    QHBoxLayout* l = new QHBoxLayout(widget);
    l->setSpacing(2);
    l->setContentsMargins (0,0,0,0);
    
    checkBox = new QCheckBox(widget);
    checkBox->setChecked(defaultValue);
    checkBox->setObjectName( QString("%1").arg(name) );
    connect(checkBox, SIGNAL(clicked()),  this, SLOT(valueChanged()));
    l->addWidget(checkBox);
}

QString BoolWidget::toString() {
    return (checkBox->isChecked()?"true":"false");
}

void BoolWidget::fromString(QString string) {
    bool v = false;
    if (string.toLower().trimmed() == "true") v = true;
    checkBox->setChecked(v);
}

void BoolWidget::setUserUniform(QOpenGLShaderProgram* shaderProgram) {
    int l = uniformLocation(shaderProgram);
    if (l != -1) {
        shaderProgram->setUniformValue(l, (bool)(checkBox->isChecked()));
    }
}

}
}

