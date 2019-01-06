#pragma once

#include <QtGui>
#include <QDialog>
#include <QDebug>
#include <QDialogButtonBox>
#include <QMessageBox>
// #include "MainWindow.h"
#include "ui_PreferencesDialog.h"

namespace Fragmentarium {
namespace GUI {

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    PreferencesDialog(QWidget*) {
        m_ui.setupUi(this);
        readSettings();
    };
    ~PreferencesDialog() {
    };

public slots:

private slots:

    void accept()
    {
        saveSettings();
        QDialog::accept();
    }

    void readSettings() {
        QSettings settings;
        m_ui.moveMainCheckBox->setChecked(settings.value("moveMain", true).toBool());
        m_ui.lineNumbersCheckBox->setChecked(settings.value("lineNumbers", true).toBool());
        m_ui.loopPlayCheckBox->setChecked(settings.value("loopPlay", false).toBool());
        m_ui.drawGLPathsCheckBox->setChecked(settings.value("drawGLPaths", true).toBool());
        m_ui.splineOccCheckBox->setChecked(settings.value("splineOcc", true).toBool());
        m_ui.autoRunCheckBox->setChecked(settings.value("autorun", true).toBool());
        m_ui.autoLoadCheckBox->setChecked(settings.value("autoload", false).toBool());
        m_ui.saveEasingCheckBox->setChecked(settings.value("saveEasing", true).toBool());
        m_ui.useDefinesCheckBox->setChecked(settings.value("useDefines", false).toBool());
        m_ui.refreshRateSpinBox->setValue(settings.value("refreshRate", 20).toInt());
        m_ui.fpsSpinBox->setValue(settings.value("fps", 25).toInt());
        m_ui.maxRecentFilesSpinBox->setValue(settings.value("maxRecentFiles", 5).toInt());
        m_ui.includePathsLineEdit->setText(settings.value("includePaths", "Examples/Include;").toString());
#ifdef USE_OPEN_EXR
        m_ui.exrBinPathsLineEdit->setText(settings.value("exrBinPaths", "bin;/usr/bin;").toString());
#endif // USE_OPEN_EXR
        m_ui.stylesheetLineEdit->setText(settings.value("editorStylesheet", "font: 9pt Courier;").toString());
    }

    void saveSettings()
    {
        QSettings settings;
        settings.setValue("moveMain", m_ui.moveMainCheckBox->isChecked());
        settings.setValue("lineNumbers", m_ui.lineNumbersCheckBox->isChecked());
        settings.setValue("loopPlay", m_ui.loopPlayCheckBox->isChecked());
        settings.setValue("drawGLPaths", m_ui.drawGLPathsCheckBox->isChecked());
        settings.setValue("splineOcc", m_ui.splineOccCheckBox->isChecked());
        settings.setValue("autorun", m_ui.autoRunCheckBox->isChecked());
        settings.setValue("autoload", m_ui.autoLoadCheckBox->isChecked());
        settings.setValue("saveEasing", m_ui.saveEasingCheckBox->isChecked());
        settings.setValue("useDefines", m_ui.useDefinesCheckBox->isChecked());
        settings.setValue("refreshRate",  m_ui.refreshRateSpinBox->value());
        settings.setValue("fps", m_ui.fpsSpinBox->value());
        settings.setValue("maxRecentFiles", m_ui.maxRecentFilesSpinBox->value());
        settings.setValue("includePaths", m_ui.includePathsLineEdit->text());
#ifdef USE_OPEN_EXR
        settings.setValue("exrBinPaths", m_ui.exrBinPathsLineEdit->text());
#endif // USE_OPEN_EXR
        settings.setValue("editorStylesheet", m_ui.stylesheetLineEdit->text());
        settings.sync();
    }

private:
    Ui::PreferencesDialog m_ui;
};
}
}
