#pragma once

#ifndef VIDEO_DIALOG_H
#define VIDEO_DIALOG_H

#include <QFile>
#include <QProcess>
#include <QtGui>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTextEdit>
#include "ui_VideoDialog.h"

class MainWindow;

namespace Fragmentarium
{
namespace GUI
{

class VideoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoDialog ( MainWindow *parent );
    ~VideoDialog();

public slots:

public:
private slots:
    void readSettings();
    void saveSettings();
    void readyReadStandardOutput();
    void processStarted();
    void encodingFinished(int);
    void playingFinished(int);
    void on_startButton_clicked();
    void on_fileOpenButton_clicked();
    void on_fileSaveButton_clicked();
    void on_playOutputButton_clicked();
    void on_encCmdButton_clicked();
    void on_optButton_clicked();
    void on_playCmdButton_clicked();
    void on_closeButton_clicked();
    void on_stopButton_clicked();

private:
    MainWindow *mainWin;
    Ui::VideoDialog *m_ui;
    QProcess *mTranscodingProcess;
    QProcess *mOutputPlayProcess;
    QString mOutputString;
};
} // namespace GUI
} // namespace Fragmentarium
#endif // DIALOG_H
