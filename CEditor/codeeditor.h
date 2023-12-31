﻿#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include "headers.h"
#include "foldlistwidget.h"
#include "foldlistwidgetitem.h"

struct LineToDraw {
    int lineNumber;
    bool draw;

    LineToDraw() : lineNumber(0), draw(false) {} // 默认构造函数

    LineToDraw(int line, bool d) : lineNumber(line), draw(d) {}
};

class Brackets // 括号匹配类
{
public:
    Brackets():currentPos(0),correspondingPos(-1),type(0),row(-1){};
    Brackets(int currentPosition, int correspondingPosition, int bracketType, int bracketRow) {
        currentPos = currentPosition;
        correspondingPos = correspondingPosition;
        type = bracketType;
        row = bracketRow;
    }
    int currentPos = 0;
    int correspondingPos = -1;
    int type = 0; // 1: '{' / -1: '}'  2: '(' / -2: ')'  3: '[' / -3: ']'
    int row = -1;
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent=nullptr);

    void matchBrackets();
    void setLineNumberArea(QListWidget *lineNumberArea);
    void setFoldListWidget(FoldListWidget *foldListWidget);
    void setCompleter(QCompleter *c);
    void autoComplete();
    void commentSelectedLines();
    void undo();
    void redo();
    void clearLineHighlight();
    void setLine(int lineNumber, bool draw);

public slots:
    void updateLineNumberArea();
    void updateFoldListWidget();
    void handleTextChanged();
    void clearAllLines();
    void moveToChangedPlace(const QString& str1, const QString& str2);

private slots:
    void autoIndent();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void scrollBarValue(int value);
    void textRealChanged();
    void initText(QString text);
    void matchFinished();

private:
    QMap<int, Brackets> bramap; // 使用 QMap 存储括号匹配情况
    QListWidget *m_lineNumberArea;
    FoldListWidget *m_foldListWidget;
    QString m_previousText;
    QStack<QString> m_undoStack;
    QStack<QString> m_redoStack;
    QTimer *m_timer;
    QCompleter *completer=nullptr;
    bool m_cursorMoved = true;// 记录成对符号自动补全后光标是否发生移动
    QList<int> m_highlightLine;// 储存需要高亮的行
    int m_maxItemSize;

    void sendCurrentScrollBarValue();
    bool bracketComplete(QKeyEvent *event);
    void highlightMatchedBrackets();
    void pushUndoStack();
    void restartTimer();
    int findFirstDifference(const QString& str1, const QString& str2);
    void moveCursorToPostion(int pos);
    void insertCompletion(const QString &completion);
    QString textUnderCursor();
    int getLineHeight(int lineNumber);
    int lineHeight = 0;
    QVector<LineToDraw> linesToDraw;
    
};

#endif // CODEEDITOR_H
