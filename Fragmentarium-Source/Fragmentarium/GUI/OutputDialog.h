#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QtGui>

#include "ui_OutputDialog.h"

namespace Fragmentarium
{
namespace GUI
{

class OutputDialog : public QDialog
{
    Q_OBJECT
public:
    OutputDialog(QWidget* parent);
    ~OutputDialog();
    int getTiles();
    double getPadding();
    int getSubFrames()
    {
        return m_ui.subFrameSpinBox->value();
    }
    int getFPS()
    {
        return m_ui.fpsSpinBox->value();
    }
    void setMaxTime(double f);
    double getMaxTime()
    {
        return m_ui.animCheckBox->isChecked() ? m_ui.endTimeSpinBox->value() : 0;
    }
    bool preview()
    {
        return m_ui.previewFrameCheckBox->isChecked();
    }
    void setTileWidth ( int w )
    {
        tileWidth = w;
    };
    void setTileHeight ( int h )
    {
        tileHeight = h;
    };
    int getTileWidth()
    {
        return tileWidth;
    };
    int getTileHeight()
    {
        return tileHeight;
    };
    void setAspectLock(bool l) {
        m_ui.lockAspectCheckBox->setChecked(l);
    };
    bool getAspectLock() {
        return m_ui.lockAspectCheckBox->isChecked();
    };

public slots:
    QString getFragmentFileName();
    bool doSaveFragment()
    {
        return m_ui.autoSaveCheckBox->isChecked();
    }
    bool doAnimation()
    {
        return m_ui.animCheckBox->isChecked();
    }
    bool doReleaseFiles(){
        return m_ui.releaseFilesCheckBox->isChecked();
    };
    
    QString getFileName();
    QString getFolderName();
    void updateFileName ( const QString &fn );
    int startAtFrame()
    {
        return m_ui.startFrameSpinBox->value();
    }
    int endAtFrame()
    {
        return m_ui.endFrameSpinBox->value();
    }
    void readOutputSettings();
    void tileXSizeChanged ( int value );
    void tileYSizeChanged ( int value );
    void lockAspect(bool l);
private slots:
    void animationChanged();
    void chooseFile();
    void updateTotalTiles ( int value );
    void tilesChanged ( int value );
    void updateFileName();
    void saveOutputSettings();
    void frameRangeSliderChanged(QPair<int,int>fR);

private:
    Ui::OutputDialog m_ui;

    QString uniqueFileName;
    int tileWidth;
    int tileHeight;
    QStringList extensions;
    QString fragmentFileName;
    bool lockedAspect;
    double currentAspect;
};

} // namespace GUI
} // namespace Fragmentarium
