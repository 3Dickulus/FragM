#include <QAbstractScrollArea>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QSpinBox>
#include <QStackedWidget>
#include <QTabBar>
#include <QTextBlock>
#include <QTextEdit>

#include "TextEdit.h"

namespace Fragmentarium
{
namespace GUI
{

TextEdit::TextEdit ( MainWindow *parent ) : QPlainTextEdit ( parent ), mainWindow ( parent )
{
    lineNumberArea = new LineNumberArea ( this );

    connect ( this, SIGNAL ( blockCountChanged ( int ) ), this, SLOT ( updateLineNumberAreaWidth ( int ) ) );
    connect ( this, SIGNAL ( updateRequest ( QRect, int ) ), this, SLOT ( updateLineNumberArea ( QRect, int ) ) );
    connect ( this, SIGNAL ( cursorPositionChanged() ), this, SLOT ( highlightCurrentLine() ) );
}

void TextEdit::insertText()
{
    QString text = ( ( QAction* ) sender() )->text();
    insertPlainText ( text.section ( "//",0,0 ) ); // strip comments
}

void TextEdit::insertFromMimeData ( const QMimeData * source )
{
    insertPlainText ( source->text() );
}

void TextEdit::contextMenuEvent ( QContextMenuEvent *event )
{
    QMenu *menu = createStandardContextMenu();
    mainWindow->createCommandHelpMenu ( menu, this, mainWindow );
    menu->exec ( event->globalPos() );
    delete menu;

}

int TextEdit::lineNumberAreaWidth()
{
    if ( !mainWindow->wantLineNumbers ) return 0;

    int count = 1;
    int max = qMax ( 1, document()->blockCount() );
    while ( max >= 10 ) {
        max /= 10;
        ++count;
    }

    return 3 + ( fontMetrics().width ( QLatin1Char ( '0' ) ) * count );
}

void TextEdit::updateLineNumberAreaWidth ( int /* newBlockCount */ )
{
    setViewportMargins ( lineNumberAreaWidth(), 0, 0, 0 );
}

void TextEdit::updateLineNumberArea ( const QRect &rect, int dy )
{
    if ( dy != 0 ) {
        lineNumberArea->scroll ( 0, dy );
    } else {
        lineNumberArea->update ( 0, rect.y(), lineNumberArea->width(), rect.height() );
    }

    updateLineNumberAreaWidth ( 0 );
}

void TextEdit::resizeEvent ( QResizeEvent *ev )
{
    QPlainTextEdit::resizeEvent ( ev );

    QRect cr = contentsRect();
    lineNumberArea->setGeometry ( QRect ( cr.left(), cr.top(), lineNumberAreaWidth(), cr.height() ) );
}

void TextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if ( !isReadOnly() && !lineNumberArea->isHidden() ) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = mainWindow->palette().color ( QPalette::Button );
        // QColor(Qt::lightGray).lighter(125); //palette().color(QPalette::Background).lighter(125);

        selection.format.setBackground ( lineColor );
        selection.format.setProperty ( QTextFormat::FullWidthSelection, true );
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append ( selection );
    }

    setExtraSelections ( extraSelections );

    highlightBrackets ();

}

void TextEdit::lineNumberAreaPaintEvent ( QPaintEvent *event )
{

    QPainter painter ( lineNumberArea );
    painter.fillRect ( event->rect(), mainWindow->palette().color ( QPalette::Button ) );

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = ( int ) blockBoundingGeometry ( block ).translated ( contentOffset() ).top();
    int bottom = top + ( int ) blockBoundingRect ( block ).height();

    while ( block.isValid() && top <= event->rect().bottom() ) {
        ++blockNumber;
        if ( block.isVisible() && bottom >= event->rect().top() ) {

            QString number = QString::number ( blockNumber );
            painter.setPen ( palette().color ( QPalette::ButtonText ) );
            painter.drawText ( 0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number );
        }

        block = block.next();
        top = bottom;
        bottom = top + ( int ) blockBoundingRect ( block ).height();
    }
}

void TextEdit::highlightBrackets ()
{

    static QVector<QPair<QString, QString>> bracketTypes = {
        {"[", "]"},
        {"(", ")"},
        {"{", "}"}
    };

    auto prevChar = document()->characterAt ( textCursor().position()-1 );
    auto thisChar = document()->characterAt ( textCursor().position() );

    for ( auto& bracketPair : bracketTypes ) {
        int direction = 0;

        QChar nextChar;
        QChar currentChar;
        auto position = textCursor().position();

        if ( bracketPair.first == thisChar ) {
            direction = 1;
            currentChar = thisChar;
            nextChar = bracketPair.second[0];
            createBracketSelection ( position );
        } else if ( bracketPair.second == prevChar ) {
            direction = -1;
            currentChar = prevChar;
            nextChar = bracketPair.first[0];
            position--;
            createBracketSelection ( position );
        } else {
            continue;
        }

        auto counter = 1;

        while ( counter != 0 && position > 0 && position < ( document()->characterCount() - 1 ) ) {

            position += direction; // Move

            // Check char
            auto character = document()->characterAt ( position );
            if ( character == currentChar ) ++counter;
            else if ( character == nextChar ) --counter;
        }

        if ( counter == 0 ) { // Found the match
            createBracketSelection ( position );
            return;
        }

        break;
    }
}

void TextEdit::createBracketSelection ( int pos )
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground ( Qt::yellow );
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition ( pos );
    cursor.movePosition ( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
    selection.cursor = cursor;

    selections.append ( selection );

    setExtraSelections ( selections );
}


} // namespace GUI
} // namespace Fragmentarium
