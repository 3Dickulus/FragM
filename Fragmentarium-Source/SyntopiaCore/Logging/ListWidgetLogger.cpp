#include "ListWidgetLogger.h"
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QTextStream>

const QString logFilePath = "debug.log";
bool loggingToFile = false;
int maxSize = 125000;

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

        void customMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
            QHash<QtMsgType, QString> msgLevelHash({{QtDebugMsg, "Debug"}, {QtInfoMsg, "Info"}, {QtWarningMsg, "Warning"}, {QtCriticalMsg, "Critical"}, {QtFatalMsg, "Fatal"}});
            QByteArray localMsg = msg.toLocal8Bit();
            QTime time = QTime::currentTime();
            QString formattedTime = time.toString("hh:mm:ss.zzz");
            QByteArray formattedTimeMsg = formattedTime.toLocal8Bit();
            QString logLevelName = msgLevelHash[type];
            QByteArray logLevelMsg = logLevelName.toLocal8Bit();

            if (loggingToFile) {
                QString txt = QString("%1 %2: %3 (%4)").arg(formattedTime, logLevelName, msg,  context.file);
                QFile outFile(logFilePath);
                outFile.open(QIODevice::WriteOnly | QIODevice::Append);
                QTextStream ts(&outFile);
                 ts.seek(0);
                 ts << txt << endl;
                 //ts.flush(); not necessary since endl adds end of line plus flushes
                outFile.resize( maxSize );
                outFile.close();
            }
        }  //sabine

        void ListWidgetLogger::log(QString message, LogLevel priority) {

            loggingToFile = QSettings().value("logToFile", false).toBool();//sabine
            if (loggingToFile) qInstallMessageHandler(customMessageOutput);//sabine

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
