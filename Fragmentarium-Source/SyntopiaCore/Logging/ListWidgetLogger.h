#pragma once

#include <QString>
#include <QVector>
#include <QListWidget>
#include <QContextMenuEvent>
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QApplication>


#include "Logging.h"

namespace SyntopiaCore {
    namespace Logging {

      class ListWidget : public QListWidget {
        Q_OBJECT
      public:
        ListWidget(QWidget* parent) : QListWidget(parent) {
        }
        
        void contextMenuEvent(QContextMenuEvent* ev) {
          QMenu contextMenu;
          QAction copyAction(tr("Copy to Clipboard"), &contextMenu);
          QAction clearAction(tr("Clear"), &contextMenu);
          contextMenu.addAction(&copyAction);
          contextMenu.addAction(&clearAction);
          QAction* choice = contextMenu.exec(ev->globalPos());
          if (choice == &copyAction) {
            QClipboard *clipboard = QApplication::clipboard();
            QList<QListWidgetItem*> items = selectedItems();
            QStringList l;
            foreach (QListWidgetItem* i, items) {
              l.append(i->text());
            }
            INFO(tr("Copied %1 lines to clipboard").arg(l.count()));
            clipboard->setText(l.join("\n"));
            
          } else if (choice == &clearAction) {
            clear();
          }
        }
      };
      
      class ListWidgetLogger : public Logger {
      public:
            ListWidgetLogger(QWidget* parent);

            virtual ~ListWidgetLogger();

            /// This method all loggers must implement
            void log(QString message, LogLevel priority);

            QListWidget* getListWidget() { return listWidget; }

            void setLogToFile();

      private:
            QListWidget* listWidget;
            QWidget* parent;
        };

    }
}

