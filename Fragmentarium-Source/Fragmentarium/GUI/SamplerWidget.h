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
/// E.g. the line: uniform samplerCube skybox; file[cubemap.png]
/// or uniform sampler2D tex; file[texture.jpg]
///	will make a sampler widget.

namespace Fragmentarium
{
namespace GUI
{

using namespace SyntopiaCore::Logging;
using namespace Fragmentarium::Parser;

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

}
} // namespace Fragmentarium
