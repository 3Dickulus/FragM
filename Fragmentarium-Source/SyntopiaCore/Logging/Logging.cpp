#include "Logging.h"

#ifdef WIN32
#include "windows.h"
#endif

namespace SyntopiaCore {
    namespace Logging {
        QVector<Logger*> Logger::loggers;
        QStack<QTime> Logger::timeStack;
        QStack<QString> Logger::timeStringStack;

        void LOG(QString message, LogLevel priority) {
            // On Windows this allows us to see debug in the Output::Debug window while running.
#ifdef WIN32
#ifdef DEBUG
            OutputDebugString((LPCWSTR) (message+"\r\n").utf16());
#endif
#endif
            for (int i = 0; i < Logger::loggers.size(); i++) {
                Logger::loggers[i]->log(message, priority);
            }
        }

        /// Useful aliases
        void Debug(QString text) { LOG(text, DebugLevel); }
        void INFO(QString text) { LOG(text, InfoLevel); }
        void SCRIPTINFO(QString text) { LOG(text, ScriptInfoLevel); }
        void WARNING(QString text) { LOG(text, WarningLevel); }
        void CRITICAL(QString text) { LOG(text, CriticalLevel); }

        void TIME(QString text) {
            LOG(text, TimingLevel);

            Logger::timeStack.push(QTime::currentTime());
            Logger::timeStringStack.push(text);
        }

        void TIME(int repetitions) {
            QTime t = Logger::timeStack.pop();
            QString s = Logger::timeStringStack.pop();
            int secs = t.msecsTo(QTime::currentTime());
            if (repetitions == 0) {
                LOG(QString("Time: %1s for ").arg(secs/1000.0f) + s, TimingLevel);
            } else {
                LOG(QString("Time: %1s for %2. %3 repetitions, %4s per repetition.").arg(secs/1000.0f).arg(s)
                    .arg(repetitions).arg((secs/repetitions)/1000.0f), TimingLevel);
            }
        }
    }
}

