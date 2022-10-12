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
#include "SamplerWidget.h"

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

// SamplerWidget
// ------------------------------------------------------------------

SamplerWidget::SamplerWidget(FileManager *fileManager, QWidget *parent, QWidget *variableEditor, QString name, QString defaultValue, QString defaultChannelValueString)
    : VariableWidget(parent, variableEditor, name), fileManager(fileManager), defaultValue(defaultValue)
{
    defaultChannelValue = defaultChannelValueString.split(";");

    // hard coded for 4 channels so no adjustments required just setup the buttons here

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

    for (int channel = 0; channel < 4; ++channel) {
        channelComboBox[channel] = new QComboBox(parent);
        channelComboBox[channel]->setEditable(false);
        channelComboBox[channel]->setObjectName(name+"Channel"+QString(channel));
        channelComboBox[channel]->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        l->addWidget(channelComboBox[channel]);
        channelComboBox[channel]->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));
        channelComboBox[channel]->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // necessarily!
        channelComboBox[channel]->view()->setCornerWidget(new QSizeGrip(channelComboBox[channel]));
        connect(channelComboBox[channel], SIGNAL(currentTextChanged(const QString &)), this, SLOT(channelChanged(const QString &)));
    }

    toolButton = new QToolButton(parent);
    toolButton->setText("...");
    l->addWidget(toolButton);
    connect(toolButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    connect(comboBox, SIGNAL(editTextChanged(const QString &)), this, SLOT(textChanged(const QString &)));

    // new widget has no ID until texture file is loaded re: SamplerWidget::setUserUniform()
    // internal texture ID 0 is reserved for the backbuffer
    texID=0;

    // updates channel combo boxes list of items (and visibility) from EXR file contents
    // the use of connect() causes a call to textChanged() as soon as the filename gets set
    // will adjust the default channel list and setting if defaultChannelValue is empty
    // this all happens before the file is loaded and bound to a texture
   textChanged(defaultValue);
}

int SamplerWidget::hasChannel(int channel, QString chan)
{
    int ci=-1;
    if(!channelComboBox[channel]->isHidden()) {
        ci=channelComboBox[channel]->findText(chan);
    }
    
    if(ci == -1) {
        QPalette pal = channelComboBox[channel]->palette();
        pal.setColor(channelComboBox[channel]->backgroundRole(), Qt::red);
        channelComboBox[channel]->setPalette(pal);
        channelComboBox[channel]->setAutoFillBackground(true);
    } else {
        channelComboBox[channel]->setPalette(QApplication::palette(channelComboBox[channel]));
        channelComboBox[channel]->setAutoFillBackground(false);
    }

    return ci;
}

void SamplerWidget::channelChanged(const QString &text)
{
    
    if(getValue().endsWith(".exr") && !text.isEmpty()) {
        
        if(!channelList.contains(text)) {
            WARNING("Channel " + text + " not found!");
        } else {
            // DBOUT << channelComboBox->currentIndex();
            valueChanged();
        }
    }
}

void SamplerWidget::textChanged(const QString &text)
{
    if (!fileManager->fileExists(text)) {
        QPalette pal = comboBox->palette();
        pal.setColor(comboBox->backgroundRole(), Qt::red);
        comboBox->setPalette(pal);
        comboBox->setAutoFillBackground(true);
    } else {
        comboBox->setPalette(QApplication::palette(comboBox));
        comboBox->setAutoFillBackground(false);
    }

    QString fileName = "";
    try {
        fileName = fileManager->resolveName(text, false);
    } catch (SyntopiaCore::Exceptions::Exception &) {
        // ignore (an empty fileName is fine as it does not end in .exr)
    }
    // hide all channel boxes
    for (int channel = 0; channel < 4; ++channel) {
        channelComboBox[channel]->clear();
        channelComboBox[channel]->setHidden(true);
    }

    if(fileName.endsWith(".exr")) {
        try {
            InputFile file ( fileName.toLatin1().data() );
            // setup channelComboBox
            if ( file.isComplete() ) {
                channelList = QStringList();
                const ChannelList &channels = file.header().channels();
                for (ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i)
                {
                    channelList << i.name();
                }
            }
        } catch (...) {
            // maybe the file disappeared between resolving and opening?
            // ignore? no, emit warning and return
            WARNING(tr("Read channel list from %1 FAILED!").arg(fileName));
            return;
        }

        if(channelList.size() != 4) {            // file does not have 4 channels
            // this could also check for some "known" file layouts and set default channels accordingly
            // Y[U|V] RGB[A]|[Z|D|DEPTH]
            if(channelList.size() == 1) {
                channelList << "";
            }
            if(channelList.size() == 2) {
                channelList << "";
            }
            if(channelList.size() == 3) {
                channelList << "";
            }
            // now channelList.size() == 4 some are blanks
        }
        
        // reveal the channel boxes
        for (int channel = 0; channel < 4; ++channel) {
            channelComboBox[channel]->addItems(channelList);
            channelComboBox[channel]->setCurrentText(channelList[channel]);
            channelComboBox[channel]->setHidden(false);
        }
    }


    valueChanged();
}

void SamplerWidget::buttonClicked()
{
    QStringList extensions;
    QList<QByteArray> imageFormats;
    imageFormats << "*"; // will show "*.*"
    imageFormats << "hdr"; // not supported by Qt, FragM supported
    imageFormats << QImageReader::supportedImageFormats();
    foreach(QByteArray s, imageFormats) {
        extensions.append(QString("*.%1").arg(QString(s)));
    }

    QStringList types;
    QList<QByteArray> mimeTypes;
    mimeTypes << "application/octet-stream"; // will show "All files (*)"
    mimeTypes << QImageReader::supportedMimeTypes();
    mimeTypes << "image/vnd.radiance"; //  supported by FragM not supported by Qt
    foreach(QByteArray s, mimeTypes) {
        types.append(QString(s));
    }

    QString fileName = "";

    QFileDialog dialog(this);
    QSettings settings;
    // use mime-types
    if(settings.value("useMimetypes").toBool()) {
        dialog.setMimeTypeFilters(types);
    } else {
        // use file extensions
        dialog.setNameFilters(extensions);
    }
    // show files with caps extensions on debian linux
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if(dialog.exec() == 1) { // test for accept
        fileName = dialog.selectedFiles().at(0);
    }

    if (!fileName.isEmpty()) {
        comboBox->setEditText(fileName);
    }
}

QString SamplerWidget::toString()
{
    QString returnValue = comboBox->currentText();
    // all the channel combo boxes should have the same isHidden state, pick one arbitrarily
    returnValue += (channelComboBox[0]->isHidden()) ? "" : " " + getChannelValue();
    return returnValue;
}

QString SamplerWidget::getValue()
{
    return comboBox->currentText();
}

bool SamplerWidget::fromString(QString str)
{
//             INFO("'" + string + "'");
    if (toString() == str) {
        return false;
    }
    QStringList test = str.split(" ");
    QString value = test.at(0);
    comboBox->setEditText(value.trimmed()); // also refreshes channel list from EXR file
    if(value != str) { // requested channel list is present
        if (value.endsWith(".exr")) {
            QStringList requestedChannels = test.at(1).split(";");
            for (int channel = 0; channel < 4; ++channel)
            {
                if (channel < requestedChannels.size()) {
                    channelComboBox[channel]->setCurrentText(requestedChannels[channel]);
                }
            }
        } else {
            for (int channel = 0; channel < 4; ++channel) {
                channelComboBox[channel]->setHidden(true);
            }
        }
    }
    return isLocked();
}

void SamplerWidget::setUserUniform(QOpenGLShaderProgram* shaderProgram)
{
    if (texID != 0) {
        int l = uniformLocation(shaderProgram);
        if( !(l < 0) ) {
            shaderProgram->setUniformValue(l, texID);
        }
    }
}

void SamplerWidget::updateTexture(Parser::FragmentSource *fs,
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

iSamplerWidget::iSamplerWidget(FileManager *fileManager, QWidget *parent, QWidget *variableEditor, QString name, QString defaultValue, QString defaultChannelValue)
    : SamplerWidget(fileManager, parent, variableEditor, name, defaultValue, defaultChannelValue)
{
}

uSamplerWidget::uSamplerWidget(FileManager *fileManager, QWidget *parent, QWidget *variableEditor, QString name, QString defaultValue, QString defaultChannelValue)
    : SamplerWidget(fileManager, parent, variableEditor, name, defaultValue, defaultChannelValue)
{
}

} // namespace GUI
} // namespace Fragmentarium
