#include "../../SyntopiaCore/Misc/Misc.h"
#include "MainWindow.h"
#include "OutputDialog.h"
#include <QDir>
#include <QFileInfo>
#include <QImageWriter>
#include <QLayout>
#include <QSettings>

namespace Fragmentarium
{
namespace GUI
{

OutputDialog::OutputDialog(QWidget *parent) : QDialog(parent)
{

	m_ui.setupUi(this);

    readOutputSettings();

    connect(m_ui.subFrameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTotalTiles(int)));
    connect(m_ui.animCheckBox, SIGNAL(clicked()), this, SLOT(animationChanged()));
    connect(m_ui.startFrameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTotalTiles(int)));
    connect(m_ui.endFrameSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTotalTiles(int)));
//     connect(m_ui.fpsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateTotalTiles(int)));
    connect(m_ui.tilesSlider, SIGNAL(valueChanged(int)), this, SLOT(tilesChanged(int)));
    connect(m_ui.paddingSlider, SIGNAL(valueChanged(int)), this, SLOT(tilesChanged(int)));
    connect(m_ui.uniqueCheckBox, SIGNAL(clicked()), this, SLOT(updateFileName()));
    connect(m_ui.autoSaveCheckBox, SIGNAL(clicked()), this, SLOT(updateFileName()));
    connect(m_ui.fileButton, SIGNAL(clicked()), this, SLOT(chooseFile()));
    connect(m_ui.filenameEdit, SIGNAL(textChanged(const QString &)), this, SLOT(updateFileName(const QString &)));
    connect(m_ui.tileWidthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(tileSizeChanged(int)));
    connect(m_ui.tileHeightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(tileSizeChanged(int)));

    QList<QByteArray> a = QImageWriter::supportedImageFormats();
#ifdef USE_OPEN_EXR
    a.append("exr");
#endif
    foreach(QByteArray s, a) {
            extensions.append(QString(s));
    }

    tileSizeChanged(0);
    updateTotalTiles(0);
    animationChanged();
    updateFileName();

    connect(m_ui.frameRangeSlider, SIGNAL(rangeChanged(QPair<int, int>)), this, SLOT(frameRangeSliderChanged(QPair<int, int>)));
}

OutputDialog::~OutputDialog()
{
    saveOutputSettings();
}

void OutputDialog::readOutputSettings()
{
    QSettings settings;
    m_ui.uniqueCheckBox->setChecked(settings.value("unique", true).toBool());
    m_ui.autoSaveCheckBox->setChecked(settings.value("autosave", true).toBool());
    m_ui.filenameEdit->setText(settings.value("filename", "out.png").toString());
    m_ui.fpsSpinBox->setValue(settings.value("fps", 25).toInt());
    m_ui.startFrameSpinBox->setValue(settings.value("startframe", 1).toInt());
    m_ui.endFrameSpinBox->setValue(settings.value("endframe", 100000).toInt());
    m_ui.subFrameSpinBox->setValue(settings.value("subframes", 1).toInt());
    m_ui.tilesSlider->setValue(settings.value("tiles", 3).toInt());
    m_ui.paddingSlider->setValue(settings.value("padding", 0).toInt());
    m_ui.animCheckBox->setChecked(settings.value("animation", false).toBool());
    m_ui.previewFrameCheckBox->setChecked(settings.value("preview", false).toBool());
    m_ui.tileWidthSpinBox->setValue(settings.value("tilewidth", 16).toInt());
    m_ui.tileHeightSpinBox->setValue(settings.value("tileheight", 9).toInt());
}

void OutputDialog::saveOutputSettings()
{
    QSettings settings;
    settings.setValue("unique", m_ui.uniqueCheckBox->isChecked());
    settings.setValue("autosave", m_ui.autoSaveCheckBox->isChecked());
    settings.setValue("filename", m_ui.filenameEdit->text());
    settings.setValue("fps", m_ui.fpsSpinBox->value());
    settings.setValue("startframe", m_ui.startFrameSpinBox->value());
    settings.setValue("endframe", m_ui.endFrameSpinBox->value());
    settings.setValue("subframes", m_ui.subFrameSpinBox->value());
    settings.setValue("tiles", m_ui.tilesSlider->value());
    settings.setValue("padding", m_ui.paddingSlider->value());
    settings.setValue("animation", m_ui.animCheckBox->isChecked());
    settings.setValue("preview", m_ui.previewFrameCheckBox->isChecked());
    settings.setValue("tilewidth",m_ui.tileWidthSpinBox->value());
    settings.setValue("tileheight",m_ui.tileHeightSpinBox->value());
    settings.sync();
}

void OutputDialog::setMaxTime(double f)
{
    m_ui.endTimeSpinBox->setValue(f);
    m_ui.endFrameSpinBox->setMaximum(f*m_ui.fpsSpinBox->value());
    m_ui.frameRangeSlider->setCutoffRange(QPair<int, int>( 0, (f * m_ui.fpsSpinBox->value())-1));
    m_ui.frameRangeSlider->setRange(QPair<int, int>( m_ui.startFrameSpinBox->value()-1, m_ui.endFrameSpinBox->value()-1));
}

void OutputDialog::animationChanged()
{
    m_ui.previewFrameCheckBox->setEnabled(!m_ui.animCheckBox->isChecked());
    if (m_ui.animCheckBox->isChecked()) {
        m_ui.previewFrameCheckBox->setChecked(false);
    }
    updateTotalTiles(0);
    m_ui.fpsSpinBox->setEnabled(false);
}

void OutputDialog::frameRangeSliderChanged(QPair<int, int> fR)
{
    m_ui.startFrameSpinBox->setValue(fR.first+1);
    m_ui.endFrameSpinBox->setValue(fR.second+1);
}

void OutputDialog::chooseFile()
{
    QString filename = SyntopiaCore::Misc::GetImageFileName(this, tr("Save image as:"));
    if (!filename.isEmpty()) {
        m_ui.filenameEdit->setText(filename);
    }
    updateFileName("");
}

void OutputDialog::updateFileName()
{
    updateFileName("");
}

QString OutputDialog::getFileName()
{
    if (m_ui.uniqueCheckBox->isChecked()) {
        return uniqueFileName;
    }
    return m_ui.filenameEdit->text();
}

QString OutputDialog::getFolderName()
{
    return QFileInfo(getFileName()).fileName().section(".",0,-2) + "_Files";
}

QString OutputDialog::getFragmentFileName()
{
    return fragmentFileName;
}

void OutputDialog::updateFileName(const QString &fn)
{

    Q_UNUSED(fn)

    uniqueFileName = "";
    QString uname = "";
    QString file = m_ui.filenameEdit->text();
    QFileInfo fi(file);
    bool error = false;
    if (!fi.absoluteDir().exists()) {
        uname = tr("dir does not exist");
        error = true;
    }

    QString extension = m_ui.filenameEdit->text().section(".",-1,-1);

    if (!extensions.contains(extension, Qt::CaseInsensitive)) {
        uname = tr("not a valid image extension");
        error = true;
    }


    if (error) {
        QPalette p = m_ui.filenameEdit->palette();
        p.setColor(QPalette::Base, QColor(255,70,70));
        m_ui.filenameEdit->setPalette(p);
    } else {
        m_ui.filenameEdit->setPalette(QApplication::palette());
    }

    m_ui.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!error);

    if (m_ui.uniqueCheckBox->isChecked()) {
        if (!error) {

            QString stripped = m_ui.filenameEdit->text().section(".", 0, -2); // find everything until extension.
            QString lastNumber = QFileInfo(stripped).fileName().section("-", -1, -1);
            bool succes = false;
            int number = lastNumber.toInt(&succes);

            if (!succes) {
                number = 1;
            }
            if (succes) {
                // The filename already had a number extension.
                stripped = stripped.section("-", 0, -2);
            }

            QString testName = stripped + "-" + QString::number(number);
            // m_ui.filenameEdit->text();
            while (QFile(testName + "." + extension).exists() || QFile(testName + "_Files").exists()) {
                testName = stripped + "-" + QString::number(number++);
            }
            uname = testName + "." + extension;
            uniqueFileName = uname;
        }

        m_ui.uniqueCheckBox->setText(
            tr("Add unique ID to filename (%1)").arg(uname));
        fragmentFileName = QFileInfo(uname.section(".", 0, -2) + ".frag").fileName();
    } else {
        m_ui.uniqueCheckBox->setText(tr("Add unique ID to filename"));
        fragmentFileName = QFileInfo(m_ui.filenameEdit->text().section(".", 0, -2) + ".frag").fileName();
    }
    if (m_ui.autoSaveCheckBox != nullptr) {
        m_ui.autoSaveCheckBox->setText(tr("Autosave fragments and settings (in directory: '%1)").arg(getFolderName()));
    }
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // //
}

void OutputDialog::tilesChanged(int value)
{

    Q_UNUSED(value)

    int t = m_ui.tilesSlider->value();
    auto mp = (double)((t * tileWidth * t * tileHeight) / (1024.0 * 1024.0));

    m_ui.tilesLabel->setText(
        tr("Render tiles: (%1x%2 - %3x%4 pixels - %5 MegaPixel):")
        .arg(t)
        .arg(t)
        .arg(t * tileWidth)
        .arg(t * tileHeight)
        .arg(mp,0,'f',1));

    double f = m_ui.paddingSlider->value()/100.0;
    double fd = 1.0/(1.0+f);

    m_ui.paddingLabel->setText(
        tr("Padding %1%: (resulting size: %2x%3 pixels - %4 MegaPixel):")
        .arg((float)(f * 100.), 0, 'f', 1)
        .arg((int)(fd * t * tileWidth))
        .arg((int)(fd * t * tileHeight))
        .arg(mp * fd * fd, 0, 'f', 1));
    updateTotalTiles(0);
}

void OutputDialog::updateTotalTiles(int value)
{
    Q_UNUSED(value)
    int t = m_ui.tilesSlider->value();
    t = t*t;
    int s = m_ui.subFrameSpinBox->value();
    if (m_ui.animCheckBox->isChecked()) {

        if (m_ui.startFrameSpinBox->value() > m_ui.endFrameSpinBox->value()) {
            m_ui.endFrameSpinBox->setValue(m_ui.startFrameSpinBox->value());
        }

        int fps = m_ui.fpsSpinBox->value();

        double length = (double)(m_ui.endFrameSpinBox->value() - m_ui.startFrameSpinBox->value()) / fps;

        m_ui.totalFrames->setText(tr("Total tiles: %1x%2x%3x%4 = %5")
                                  .arg(t)
                                  .arg(s)
                                  .arg(fps)
                                  .arg(length)
                                  .arg(fps * length * s * t));

        m_ui.frameRangeSlider->setCutoffRange(QPair<int, int>(0, (m_ui.endTimeSpinBox->value() * fps)-1));
        m_ui.frameRangeSlider->setRange(QPair<int, int>(m_ui.startFrameSpinBox->value()-1, m_ui.endFrameSpinBox->value()-1));

    } else {
        m_ui.totalFrames->setText(tr("Total tiles: %1x%2 = %3").arg(t).arg(s).arg(t * s));
    }
    // Tiles x subframes x animframes
}

void OutputDialog::tileSizeChanged(int value)
{
    Q_UNUSED(value)
    tileWidth = m_ui.tileWidthSpinBox->value();
    tileHeight = m_ui.tileHeightSpinBox->value();
    tilesChanged(getTiles());
}

int OutputDialog::getTiles()
{
    return m_ui.tilesSlider->value();
}

double OutputDialog::getPadding()
{
    return m_ui.paddingSlider->value()/100.0;
}
} // namespace GUI
} // namespace Fragmentarium
