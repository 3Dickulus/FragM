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

TextBlockData::TextBlockData() = default;
// {
    // Nothing to do
// }

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}


void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() && info->position > m_parentheses.at(i)->position) {
        ++i;
    }

    m_parentheses.insert(i, info);
}

void TextBlockData::append(ParenthesisInfo *info)
{
    m_parentheses.append(info);
}

TextEdit::TextEdit(MainWindow *parent) : QPlainTextEdit(parent), mainWindow(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
}

void TextEdit::insertText()
{
    QString text = ((QAction*)sender())->text();
    insertPlainText(text.section("//",0,0)); // strip comments
}

void TextEdit::insertFromMimeData ( const QMimeData * source )
{
    insertPlainText ( source->text() );
}

void TextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    mainWindow->createCommandHelpMenu(menu, this, mainWindow);
    menu->exec(event->globalPos());
    delete menu;

}

int TextEdit::lineNumberAreaWidth()
{
    if (!mainWindow->wantLineNumbers) {
        return 0;
    }

    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + (fontMetrics().width(QLatin1Char('9')) * digits);

    return space;
}

void TextEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TextEdit::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy != 0) {
        lineNumberArea->scroll(0, dy);
    } else {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    //if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void TextEdit::resizeEvent(QResizeEvent *ev)
{
    QPlainTextEdit::resizeEvent(ev);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TextEdit::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly() && !lineNumberArea->isHidden() ) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::lightGray).lighter(125);

        if (!mainWindow->wantLineNumbers) {
            lineColor = QColor(Qt::white);
        }

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);

    matchParentheses();

}

void TextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{

    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {

            QString number = QString::number(blockNumber);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void TextEdit::matchParentheses()
{
    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data != nullptr) {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i) {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && (info->character == '(')) {
                if (matchLeftParenthesis(textCursor().block(), i + 1, 0)) {
                    createParenthesisSelection(pos + info->position);
                }
            } else if (info->position == curPos - 1 && (info->character == ')')) {
                if (matchRightParenthesis(textCursor().block(), i - 1, 0)) {
                    createParenthesisSelection(pos + info->position);
                }
            }
        }
    }
}

bool TextEdit::matchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses)
{
    auto *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i) {
        ParenthesisInfo *info = infos.at(i);
        if (info->character == '(') {
            ++numLeftParentheses;
            continue;
        }
        if ( (info->character == ')')  && numLeftParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        --numLeftParentheses;
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid()) {
        return matchLeftParenthesis(currentBlock, 0, numLeftParentheses);
    }

    return false;
}

bool TextEdit::matchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses)
{
    auto *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    if (!infos.isEmpty()) {
        for (; i > -1; --i) {
            ParenthesisInfo *info = infos.at(i);
            if (info->character == ')') {
                ++numRightParentheses;
                continue;
            }
            if ( (info->character == '(') && numRightParentheses == 0) {
                createParenthesisSelection(docPos + info->position);
                return true;
            }
            --numRightParentheses;
        }
    }
    currentBlock = currentBlock.previous();
    if (currentBlock.isValid()) {
        return matchRightParenthesis(currentBlock, 0, numRightParentheses);
    }

    return false;
}

void TextEdit::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground(Qt::yellow);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}

} // namespace GUI
} // namespace Fragmentarium
