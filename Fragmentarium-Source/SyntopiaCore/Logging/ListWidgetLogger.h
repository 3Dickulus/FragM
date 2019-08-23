#pragma once

#include <QObject>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QListWidget>
#include <QMenu>
#include <QString>
#include <QVector>
#include <QRegExp>

#include "Logging.h"

namespace SyntopiaCore
{
namespace Logging
{

class ListWidget : public QListWidget
{
        Q_OBJECT
public:
    ListWidget ( QWidget *parent ) : QListWidget ( parent ) {}

    void contextMenuEvent ( QContextMenuEvent *ev )
    {
        QMenu contextMenu;
        QAction openFileAction ( tr ( "Open file" ), &contextMenu );
        QAction copyAction ( tr ( "Copy to Clipboard" ), &contextMenu );
        QAction clearAction ( tr ( "Clear" ), &contextMenu );

        // check to see if we should add an Open File menu item
        QList<QListWidgetItem *> items = selectedItems();
        QRegExp test ( "^(.*[.frag])\\s.([0-9]+)[)]" );
        foreach ( QListWidgetItem *i, items ) {
            if (test.indexIn(i->text()) != -1) {
                contextMenu.addAction ( &openFileAction );
                break;
            }
        }
        // always add these menu items
        contextMenu.addAction ( &copyAction );
        contextMenu.addAction ( &clearAction );

        QAction *choice = contextMenu.exec ( ev->globalPos() );

        if ( choice == &openFileAction ) {
            foreach ( QListWidgetItem *i, items ) {
                if (test.indexIn(i->text()) != -1) {
                    // send signal to mainWindow to open this file and jump to line
                    emit(loadSourceFile(test.cap(1), test.cap(2).toInt()));
                }
            }

        } else if ( choice == &copyAction ) {
            QClipboard *clipboard = QApplication::clipboard();
            QList<QListWidgetItem *> items = selectedItems();
            QStringList l;
            foreach ( QListWidgetItem *i, items ) {
                l.append ( i->text() );
            }
            INFO ( tr ( "Copied %1 lines to clipboard" ).arg ( l.count() ) );
            clipboard->setText ( l.join ( "\n" ) );

        } else if ( choice == &clearAction ) {
            clear();
        }
    }

signals:
    void loadSourceFile(QString fileName, int lineNumber);
};

class ListWidgetLogger : public Logger
{
public:
    ListWidgetLogger ( QWidget *parent );

    virtual ~ListWidgetLogger();

    /// This method all loggers must implement
    void log ( QString message, LogLevel priority );

    QListWidget *getListWidget()
    {
        return listWidget;
    }

    void setLogToFile();

private:
    QListWidget *listWidget;
    QWidget *parent;
};

} // namespace Logging
} // namespace SyntopiaCore
