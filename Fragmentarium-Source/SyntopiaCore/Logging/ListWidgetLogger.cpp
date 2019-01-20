#include "ListWidgetLogger.h"
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QSettings>

namespace SyntopiaCore {
    namespace Logging {

        ListWidgetLogger::ListWidgetLogger(QWidget* parent) : parent(parent) {
            listWidget = new ListWidget(parent);
            listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
            listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        }

        ListWidgetLogger::~ListWidgetLogger() {
            listWidget->clear();
            delete [] listWidget;
            listWidget = 0;
        }
//sabine
        namespace {
            void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
                QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
                QTime time = QTime::currentTime();
                QString formattedTime = time.toString("hh:mm:ss.zzz");
                QString logLevelName = msgLevelHash[type];
                // are we in verbose mode ?
                QString txt = QSettings().value("verbose", false).toBool() ?
                              QString("%1 %2: %3 (%4)\n").arg(formattedTime).arg(logLevelName).arg(msg).arg(context.file) :
                              QString(msg);
                // setup our file
                QFile outFile(QSettings().value("logFilePath", "fragm.log").toString());
                // QIODevice::Text should ensure handling of CRLF issues
                outFile.open(QIODevice::ReadWrite | QIODevice::Text);
                // it should exist unless something went wrong
                if(outFile.exists()) {
                    //read in the file
                    QString temp( outFile.readAll() );
                    
                    int fSize = QSettings().value("maxLogFileSize", 125).toInt() * 1024;
                    if(outFile.size()+txt.size() > fSize)
                        temp.remove( 0, txt.size() );
                    temp.append( txt );
                    // set to overwrite
                    outFile.seek(0);
                    outFile.write( temp.toLocal8Bit() );
                    // finalize
                    outFile.flush();
                }
                outFile.close();
            }
        }

        void ListWidgetLogger::setLogToFile() {
            qInstallMessageHandler( customMessageOutput );
        };
//sabine      

        void ListWidgetLogger::log(QString message, LogLevel priority) {

            QListWidgetItem* i = new QListWidgetItem(message, listWidget);

            // Levels: NoneLevel, DebugLevel, TimingLevel, InfoLevel, WarningLevel, CriticalLevel, AllLevel

            if ( priority == InfoLevel ) {
                i->setBackgroundColor(QColor(255,255,255));
            } else if ( priority == ScriptInfoLevel ) {
                i->setBackgroundColor(QColor(50,50,50));
                i->setForeground(QBrush(QColor(255,255,255)));
                QFont f = i->font();
                f.setBold(true);
                i->setFont(f);
            } else if ( priority == WarningLevel ) {
                parent->show();
                i->setBackgroundColor(QColor(255,243,73));
            } else if ( priority == CriticalLevel ) {
                parent->show();
                i->setBackgroundColor(QColor(255,2,0));
            } else if ( priority == TimingLevel ) {
                parent->show();
                i->setBackgroundColor(QColor(25,255,0));
            } else {
                i->setBackgroundColor(QColor(220,220,220));
            }
/// this causes segfault in qtextengine
//             listWidget->scrollToItem(i);
/// so we use this
            listWidget->scrollToBottom();
        }

    }
}
