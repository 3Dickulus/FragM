// VideoDialog.cpp

#include "MainWindow.h"
#include "VideoDialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QString>

namespace Fragmentarium
{
namespace GUI
{

VideoDialog::VideoDialog(MainWindow *parent)
    : mainWin(parent), m_ui(new Ui::VideoDialog)
{
    m_ui->setupUi(this);

    readSettings();

    // Play button for output - initially disabled
    m_ui->playOutputButton->setEnabled(QFile::exists(m_ui->toLineEdit->text()));
    // Stop button for enc/play - initially disabled
    m_ui->stopButton->setEnabled(false);

    // Create transcoding processes
    mTranscodingProcess = new QProcess(this);
    mTranscodingProcess->setObjectName("TranscodingProcess");
    connect(mTranscodingProcess, SIGNAL(started()), this, SLOT(processStarted()));
    connect(mTranscodingProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(mTranscodingProcess, SIGNAL(finished(int)), this, SLOT(encodingFinished(int)));

    // Create playback processes
    mOutputPlayProcess = new QProcess(this);
    mOutputPlayProcess->setObjectName("PlayProcess");
    connect(mOutputPlayProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    m_ui->transcodingStatusLabel->setText("Encoding Status: Ready.");
}

VideoDialog::~VideoDialog()
{
    saveSettings();
    delete m_ui;
}

void VideoDialog::processStarted()
{
    m_ui->transcodingStatusLabel->setText(QString("Encoding Status: %1").arg("Running!"));
    m_ui->playOutputButton->setEnabled(false);
    m_ui->stopButton->setEnabled(true);
}

// conversion start
void VideoDialog::on_startButton_clicked()
{
    QString program = m_ui->encCmdLineEdit->text();

    QStringList arguments;
    QString input = m_ui->fromLineEdit->text();
    QString options = m_ui->optLineEdit->text();

    if (input.isEmpty()) {
        QMessageBox::information(this, tr("Image"), tr("Input files not specified"));
        return;
    }
    QString output = m_ui->toLineEdit->text();
    if (output.isEmpty()) {
        QMessageBox::information(this, tr("Video"), tr("Output file not specified"));
        return;
    }

    QString fileName = m_ui->toLineEdit->text();
    if (QFile::exists(fileName)) {
        if (QMessageBox::question(this, tr("Video"),
                                  tr("A file called %1 already exists in the current directory. Overwrite?")
                                  .arg(fileName),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::No) {
            return;
        }
        QFile::remove(fileName);
        if (QFile::exists(fileName)) {
            QMessageBox::warning(this, tr("Video"), tr("%1 still exists").arg(fileName));
        }
    }

    arguments.clear();

    QPixmap pm(input);

    QString filesuf = input.split(".").last();
    QString filepre = input.left(input.length() - (filesuf.length() + 7));

    input = QString("%1.*.%2").arg(filepre).arg(filesuf);
    // mencoder mf://$1*.png -mf w=$2:h=$3:fps=$4:type=png -ovc x264 -lavcopts vcodec=libx264 -x264encopts crf=25 -o $1.$2x$3.mp4
    // mencoder file input
    if (program.contains("mencoder", Qt::CaseInsensitive) && !pm.isNull()) {
        arguments << QString("mf://%1").arg(input);
        arguments << QString("-mf w=%1:h=%2:fps=%3:type=%4")
                  .arg(pm.width())
                  .arg(pm.height())
                  .arg(mainWin->renderFPS)
                  .arg(filesuf);
        arguments << options;
        arguments << QString("-o %1").arg(output);
    }

    // ffmpeg -f image2 -s $2 -i $1.%05d.png -r 25000/1001 -b:v 2400k -bt 3400k -vcodec libx264 -coder 0 -bf 0 -refs 1 -level 30 -bufsize 1835k -maxrate 30M $1.$2.mp4
    //ffmpeg file input
    if (program.contains("ffmpeg", Qt::CaseInsensitive) && !pm.isNull()) {
        arguments << QString("-f image2");
        arguments << QString("-s %1x%2").arg(pm.width()).arg(pm.height());
        arguments << QString("-i %1.%05d.%2").arg(filepre).arg(filesuf);
        arguments << QString("-r %1/1001").arg(mainWin->renderFPS * 1000);
        arguments << options;
        arguments << output;
    }

    mTranscodingProcess->setProcessChannelMode(QProcess::MergedChannels);
    if(!arguments.isEmpty() && (program.contains("ffmpeg", Qt::CaseInsensitive) || program.contains("mencoder", Qt::CaseInsensitive)) ) {
        mTranscodingProcess->setProgram(program);

        const QString newArgs = arguments.join(" ");
#ifdef Q_OS_WIN
        // this is fudge for windows
        mTranscodingProcess->setNativeArguments( newArgs );
#else
        // this is fudge for linux
        mTranscodingProcess->setArguments( newArgs.split(" ") );
#endif
        mTranscodingProcess->start();
    } else mTranscodingProcess->terminate();
}

void VideoDialog::readyReadStandardOutput()
{

    QString senderName = sender()->objectName();
    QString outtxt;

    if (senderName == QString("PlayProcess")) {
        outtxt = mOutputPlayProcess->readAllStandardOutput();
    }

    if (senderName == QString("TranscodingProcess")) {
        outtxt = mTranscodingProcess->readAllStandardOutput();
    }

    if (outtxt.contains("configuration: --prefix")) {
        outtxt = "";
    }

    if (outtxt.endsWith('\r')) {
        mOutputString.replace(mOutputString.lastIndexOf('\n') + 1, outtxt.length(), outtxt);
    } else {
        mOutputString.append(outtxt);
    }

    m_ui->textEdit->setText(mOutputString);
    m_ui->textEdit->update();
    // put the slider at the bottom
    m_ui->textEdit->verticalScrollBar()->setSliderPosition(
    m_ui->textEdit->verticalScrollBar()->maximum());
}

void VideoDialog::encodingFinished(int)
{
    // Set the encoding status by checking output file's existence
    QString fileName = m_ui->toLineEdit->text();

    bool encstat = (mTranscodingProcess->exitStatus() == 0);

    if (QFile::exists(fileName)) {
        m_ui->transcodingStatusLabel->setText(QString("Encoding Status: %1").arg(encstat ? "Ready." : "Failed!"));
    } else {
        m_ui->transcodingStatusLabel->setText("Encoding Status: Failed!");
        encstat = false;
    }

    m_ui->playOutputButton->setEnabled(encstat);
    m_ui->stopButton->setEnabled(false);
}

// ... button clicked - this is for input file
void VideoDialog::on_fileOpenButton_clicked()
{
    QStringList extensions;
    QList<QByteArray> a = QImageWriter::supportedImageFormats();
    foreach (QByteArray s, a) {
        extensions.append(QString("%1.%2").arg("*").arg(QString(s)));
    }
    QString ext = QString(tr("Images (")) + extensions.join(" ") + tr(")");

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", ext);
    if (!fileName.isEmpty()) {
        m_ui->fromLineEdit->setText(fileName);
    }
}

void VideoDialog::on_fileSaveButton_clicked()
{
    QStringList extensions;
    extensions << "*.mpeg";
    extensions << "*.mpg";
    extensions << "*.avi";
    extensions << "*.mp4";
    extensions << "*.ogg";
    extensions << "*.mkv";

    QString ext = QString(tr("Videos (")) + extensions.join(" ") + tr(")");

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", ext);
    if (!fileName.isEmpty()) {
        m_ui->toLineEdit->setText(fileName);
    }
}

void VideoDialog::playingFinished(int)
{
    on_stopButton_clicked();
    m_ui->transcodingStatusLabel->setText("Encoding Status: Ready.");
}

void VideoDialog::on_playOutputButton_clicked()
{
    QString program = m_ui->playCmdLineEdit->text().split(" ").first();
    QStringList arguments = m_ui->playCmdLineEdit->text().split(" ");
    arguments.removeFirst();
    QString output = m_ui->toLineEdit->text();
    arguments << output;
    mOutputPlayProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_ui->transcodingStatusLabel->setText("Encoding Status: Playing...");
    mOutputPlayProcess->setProgram(program);

    const QString newArgs = arguments.join(" ");
#ifdef Q_OS_WIN
        // this is fudge for windows
        mOutputPlayProcess->setNativeArguments( newArgs );
#else
        // this is fudge for linux
        mOutputPlayProcess->setArguments( newArgs.split(" ") );
#endif

    mOutputPlayProcess->start();
    connect(mOutputPlayProcess, SIGNAL(finished(int)), this, SLOT(playingFinished(int)));
    m_ui->stopButton->setEnabled(true);
}

void VideoDialog::on_encCmdButton_clicked()
{
    QString ext = QString(tr("All files ( * )"));

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", ext);
    if (!fileName.isEmpty()) {
        m_ui->encCmdLineEdit->setText(fileName);
    }
}

void VideoDialog::on_optButton_clicked()
{
    QString ext = QString(tr("Cmd options ( *.opt )"));

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), mainWin->getMiscDir(), ext);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Fragmentarium"), tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }

    if (!fileName.isEmpty()) {
        QTextStream in(&file);
        m_ui->optLineEdit->setText(in.readLine());
    }
}

void VideoDialog::on_playCmdButton_clicked()
{
    QString ext = QString(tr("All files ( * )"));

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", ext);
    if (!fileName.isEmpty()) {
        m_ui->playCmdLineEdit->setText(fileName);
    }
}

void VideoDialog::on_closeButton_clicked()
{
    on_stopButton_clicked();

    close();
}

void VideoDialog::on_stopButton_clicked()
{
    mOutputPlayProcess->close();
    mTranscodingProcess->close();
    mOutputPlayProcess->kill();
    mTranscodingProcess->kill();
    m_ui->stopButton->setEnabled(false);
    m_ui->transcodingStatusLabel->setText(QString("Encoding Status: %1").arg("Stopped!"));
    saveSettings();
}

void VideoDialog::readSettings()
{
    QSettings settings;
    m_ui->fromLineEdit->setText(settings.value("videofrom", "").toString());
    m_ui->toLineEdit->setText(settings.value("videoto", "").toString());
    m_ui->encCmdLineEdit->setText(settings.value("videoenccmd", "").toString());
    m_ui->optLineEdit->setText(settings.value("videoencopt", "").toString());
    m_ui->playCmdLineEdit->setText(settings.value("videoplayer", "").toString());
}

void VideoDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("videofrom", m_ui->fromLineEdit->text());
    settings.setValue("videoto", m_ui->toLineEdit->text());
    settings.setValue("videoenccmd", m_ui->encCmdLineEdit->text());
    settings.setValue("videoencopt", m_ui->optLineEdit->text());
    settings.setValue("videoplayer", m_ui->playCmdLineEdit->text());
}
} // namespace GUI
} // namespace Fragmentarium
