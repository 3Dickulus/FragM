#include "ListWidgetLogger.h"
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

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

        void ListWidgetLogger::log(QString message, LogLevel priority) {
            if (listWidget->count() > 120) {
                listWidget->setUpdatesEnabled(false);
                while (listWidget->count() > 130) {
                     listWidget->removeItemWidget(listWidget->item(0));
                }
                listWidget->setUpdatesEnabled(true);
            }

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
