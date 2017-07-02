#ifndef TEXTEDIT_H
#define TEXTEDIT_H


#include <QMainWindow>
#include <QTabBar>
#include <QStackedWidget>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QOpenGLShaderProgram>
#include <QTextBlock>
#include <QScrollBar>
#include <QAbstractScrollArea>
#include <QAction>
#include <QMenu>
#include "MainWindow.h"

class MainWindow;
class FragmentHighlighter;

namespace Fragmentarium {
    namespace GUI {

	// A modified QTextEdit with an extended context menu, syntax highlighting and line numbers
	class TextEdit : public QPlainTextEdit
	{
	    Q_OBJECT

	public:
	    TextEdit(MainWindow *parent = 0);

	    void lineNumberAreaPaintEvent(QPaintEvent *event);
	    int lineNumberAreaWidth();
	    void contextMenuEvent(QContextMenuEvent *event);
	    void insertFromMimeData (const QMimeData * source );
	    void saveSettings( QString ss) {savedSettings = ss;}
	    QString lastSettings() { return savedSettings; }
	    FragmentHighlighter *fh;
	    QWidget *lineNumberArea;
	public slots:
	    void insertText();

	protected:
	    void resizeEvent(QResizeEvent *event);

	private slots:
	    void updateLineNumberAreaWidth(int newBlockCount);
	    void highlightCurrentLine();
	    void updateLineNumberArea(const QRect &, int);
	    void matchParentheses();

	private:
	    MainWindow* mainWindow;
	    QString savedSettings;
	    bool matchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses);
	    bool matchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses);
	    void createParenthesisSelection(int pos);
	};


	class LineNumberArea : public QWidget
	{
	public:
	    LineNumberArea(TextEdit *editor) : QWidget(editor) {
		codeEditor = editor;
	    }

	    QSize sizeHint() const {
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	    }

	protected:
	    void paintEvent(QPaintEvent *event) {
		codeEditor->lineNumberAreaPaintEvent(event);
	    }

	private:
	    TextEdit *codeEditor;
	};
    }
}
#endif // TEXTEDIT_H