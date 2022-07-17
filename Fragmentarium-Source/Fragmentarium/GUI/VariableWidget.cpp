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

VariableWidget::VariableWidget(QWidget *parent, QWidget *variableEditor, QString name)
    : QWidget(parent), name(name), updated(false), systemVariable(false), variableEditor(variableEditor)
{

    auto *vl = new QHBoxLayout(this);
    vl->setSpacing(2);
    vl->setContentsMargins (0,0,0,0);
    setObjectName(name);
    lockButton = new QPushButton(this);
    lockButton->setObjectName("lockbutton");
    lockButton->setFlat(true);
    lockButton->setStyleSheet("* {background: none; border: none; outline: none;}");
    lockButton->setIcon(QIcon(":/Icons/padlockb.png"));
    lockButton->setFixedSize(12,18);
    lockButton->setCheckable(true);
    lockButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    vl->addWidget(lockButton,0,Qt::AlignLeft | Qt::AlignVCenter);
    connect(lockButton, SIGNAL(toggled(bool)), this, SLOT(locked(bool)));

    label = new QLabel(name,this);
    label->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
    vl->addWidget(label,0,Qt::AlignRight | Qt::AlignVCenter);

    widget = new QWidget(this);
    vl->addWidget(widget);
    connect(this, SIGNAL(changed(bool)), variableEditor, SLOT(childChanged(bool)));
}

void VariableWidget::comboSliderBoundsChanged( QString name ) {

    auto *ve = dynamic_cast<VariableEditor *>(variableEditor);
    auto *te = dynamic_cast<TextEdit *>(ve->getMainWindow()->getTextEdit());

    QStringList sliderBounds = name.split(" ");

    int bound = 0;
    bound = sliderBounds[1].toInt();

    int whichSlider = QString(sliderBounds[0][sliderBounds[0].size()-1]).toInt();

    if(whichSlider != 0) sliderBounds[0].chop(1);

    // move cursor to beginning of document in preparation for search
    QTextCursor cursor(te->textCursor());
    cursor.setPosition(0);
    te->setTextCursor(cursor);

    if(te->find( QRegExp( "^.*[ ]{1,}" + sliderBounds[0] + "[ ]{0,};[ ]{0,}.*$" ))) {
        if(bound != 0) {
            QTextCursor cursor(te->textCursor());
            te->setTextCursor( cursor );
            QString comboSliderLine = cursor.block().text();
            
            QString type;
            QString name;
            QString lower;
            // QString default;
            QString upper;
            int pos=0;

            if (intSlider.indexIn(comboSliderLine) != -1) {
                name = intSlider.cap(1);
                lower = intSlider.cap(2); // default = intSlider.cap(3);
                upper = intSlider.cap(4);
                pos = bound==1 ? intSlider.pos(2) : bound==2 ? intSlider.pos(4):0;
                
            } else
            if (float1Slider.indexIn(comboSliderLine) != -1) {
                type = float1Slider.cap(1);
                name = float1Slider.cap(2);
                lower = float1Slider.cap(3); // default = float1Slider.cap(4);
                upper = float1Slider.cap(5);
                pos = bound==1 ? float1Slider.pos(3) : bound==2 ? float1Slider.pos(5):0;
                
            } else
            if (float2Slider.indexIn(comboSliderLine) != -1) {
                type = float2Slider.cap(1);
                name = float2Slider.cap(2);
                switch(whichSlider) {
                    case 1: {
                        lower = float2Slider.cap(3); // default = float2Slider.cap(5);
                        upper = float2Slider.cap(7);
                        pos = bound==1 ? float2Slider.pos(3) : bound==2 ? float2Slider.pos(7):0;
                    break;
                    }
                    case 2: {
                        lower = float2Slider.cap(4); // default = float2Slider.cap(6);
                        upper = float2Slider.cap(8);
                        pos = bound==1 ? float2Slider.pos(4) : bound==2 ? float2Slider.pos(8):0;
                    break;
                    }
                }
            } else
            if (float3Slider.indexIn(comboSliderLine) != -1) {
                type = float3Slider.cap(1);
                name = float3Slider.cap(2);
                switch(whichSlider) {
                    case 1: {
                        lower = float3Slider.cap(3); // default = float3Slider.cap(6);
                        upper = float3Slider.cap(9);
                        pos = bound==1 ? float3Slider.pos(3) : bound==2 ? float3Slider.pos(9):0;
                    break;
                    }
                    case 2: {
                        lower = float3Slider.cap(4); // default = float3Slider.cap(7);
                        upper = float3Slider.cap(10);
                        pos = bound==1 ? float3Slider.pos(4) : bound==2 ? float3Slider.pos(10):0;
                    break;
                    }
                    case 3: {
                        lower = float3Slider.cap(5); // default = float3Slider.cap(8);
                        upper = float3Slider.cap(11);
                        pos = bound==1 ? float3Slider.pos(5) : bound==2 ? float3Slider.pos(11):0;
                    break;
                    }
                }
            } else
            if (float4Slider.indexIn(comboSliderLine) != -1) {
                type = float4Slider.cap(1);
                name = float4Slider.cap(2);
                switch(whichSlider) {
                    case 1: {
                        lower = float4Slider.cap(3); // default = float4Slider.cap(7);
                        upper = float4Slider.cap(11);
                        pos = bound==1 ? float4Slider.pos(3) : bound==2 ? float4Slider.pos(11):0;
                    break;
                    }
                    case 2: {
                        lower = float4Slider.cap(4); // default = float4Slider.cap(8);
                        upper = float4Slider.cap(12);
                        pos = bound==1 ? float4Slider.pos(4) : bound==2 ? float4Slider.pos(12):0;
                    break;
                    }
                    case 3: {
                        lower = float4Slider.cap(5); // default = float4Slider.cap(9);
                        upper = float4Slider.cap(13);
                        pos = bound==1 ? float4Slider.pos(5) : bound==2 ? float4Slider.pos(13):0;
                    break;
                    }
                    case 4: {
                        lower = float4Slider.cap(6); // default = float4Slider.cap(10);
                        upper = float4Slider.cap(14);
                        pos = bound==1 ? float4Slider.pos(6) : bound==2 ? float4Slider.pos(14):0;
                    break;
                    }
                }
            }

            int len = bound==1 ? lower.length() : bound==2 ? upper.length():0;
            QString newComboSliderLine = comboSliderLine.replace(pos, len, sliderBounds[2]);

            cursor.removeSelectedText();
            cursor.insertText(newComboSliderLine);
        }
    } else {
        if(!te->find( QRegExp( "^.*[ ]{1,}" + sliderBounds[0] + ";[ ]{0,}.*$" ))) {
            ve->getMainWindow()->getLogger()->log(QString("Uniform variable named: " + sliderBounds[0] + " Not Found in user frag!"),ScriptInfoLevel);
            ve->getMainWindow()->getLogger()->log(QString("Declared in one of the following dependencies..."),ScriptInfoLevel);
            
            while(te->find( QRegExp( "#include[ ]{1,}.*.frag.*$" ))) {
                QTextCursor cursor(te->textCursor());
                te->setTextCursor( cursor );
                QRegExp includeFile = QRegExp( "#include[ ]{1,}.(.*.frag)" );
                if (includeFile.indexIn(cursor.block().text())!=-1) {
                    QString s = includeFile.cap(1);
                    ve->getMainWindow()->getLogger()->log(s,InfoLevel);
                }
            }
        }
    }
}

bool VariableWidget::isLocked()
{
    return (lockType == Locked || lockType == AlwaysLocked);
}

void VariableWidget::valueChanged()
{

    if(qApp->styleSheet().isEmpty()) {

        if (isLocked() || oldLockType != lockType) {
            QPalette pal = palette();
            pal.setColor(backgroundRole(), Qt::yellow);
            setPalette(pal);
            setAutoFillBackground(true);
            emit(changed(true));
        } else {
            emit(changed(false));
        }

    } else {

        if (isLocked() || oldLockType != lockType) {
            label->setStyleSheet("* {border: none; color: black; background: QRadialGradient(cx:0.5, cy:0.5, radius: 0.6, fx:0.5, fy:0.5, stop:0 rgb(255,255,0,100%), stop:1 rgb(255,255,0,0%));}");
            emit(changed(true));
        } else {
            label->setStyleSheet("* {;}");
            emit(changed(false));
        }
    }
    oldLockType = lockType;
}

void VariableWidget::locked(bool l)
{

    oldLockType = lockType;

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
    
    oldLockType = lt;
}

QString VariableWidget::toSettingsString()
{
    QString l = "";
    if (lockType != defaultLockType) {
        l = " " + lockType.toString();
    }
    QString s = "";
    if (sliderType != defaultSliderType) {
        s = " " + sliderType.toString();
    }
    return toString() + s + l;
}

bool VariableWidget::fromSettingsString(QString str)
{
    bool requiresRecompile = false;

    const QRegExp lockTypeString("(AlwaysLocked|Locked|NotLocked|NotLockable)\\s*.?$");
    if (lockTypeString.indexIn(str)!=-1) {
        QString s = lockTypeString.cap(1);
        str.remove(s);
        LockType before = lockType;
        lockType.fromString(s);
        if (before!=lockType) {
            requiresRecompile = true;
        }
    }

    if (requiresRecompile) {
        locked( lockType == Locked || lockType == AlwaysLocked);
    }

    const QRegExp sliderTypeString("(Logarithmic)\\s*.?$");
    if (sliderTypeString.indexIn(str)!=-1) {
        QString s = sliderTypeString.cap(1);
        setSliderType(Logarithmic);
        str.remove(s);

    } else setSliderType(Linear);

    requiresRecompile |= fromString(str.trimmed());

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

} // namespace GUI
} // namespace Fragmentarium
