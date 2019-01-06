#pragma once

#include <QtGui>
#include <QDialog>
#include <QDialogButtonBox>

#include "ui_OutputDialog.h"

namespace Fragmentarium {
    namespace GUI {

        class OutputDialog : public QDialog
        {
            Q_OBJECT
        public:
            OutputDialog(QWidget* parent);
            ~OutputDialog();
            int getTiles();
            double getPadding();
            int getSubFrames() { return m_ui.subFrameSpinBox->value(); }
            int getFPS() {return m_ui.fpsSpinBox->value(); }
            void setMaxTime(double f);
            int getMaxTime() {return m_ui.animCheckBox->isChecked() ? m_ui.endTimeSpinBox->value() : 0; }
            bool preview() { return m_ui.previewFrameCheckBox->isChecked(); }
            void setTileWidth(int w) {tileWidth = w;};
            void setTileHeight(int h) {tileHeight = h;};
            int getTileWidth() { return tileWidth;};
            int getTileHeight() { return tileHeight;};
        public slots:
            QString getFragmentFileName();
            bool doSaveFragment() { return m_ui.autoSaveCheckBox->isChecked(); }
            bool doAnimation() { return m_ui.animCheckBox->isChecked(); }
            QString getFileName();
            QString getFolderName();
            void updateFileName(const QString &);
            int startAtFrame() { return m_ui.startFrameSpinBox->value(); }
            int endAtFrame() { return m_ui.endFrameSpinBox->value(); }
            void readOutputSettings();
            void tileSizeChanged(int);
	private slots:
            void animationChanged();
            void chooseFile();
            void updateTotalTiles(int);
            void tilesChanged(int);
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
        };

    }
}
