#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QScrollBar>
#include <QTextBlock>
#include <QPainter>
#include <QTextDocument>
#include <QTextBlock>
#include <QTimer>
#include <QDebug>
#include <QStack>

#include "foldlistwidget.h"

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
    int type = 0; // 1: '{' / -1: '}'  2: '(' / -2: ')'
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
    void undo();
    void redo();

public slots:
    void updateLineNumberArea();
    void updateFoldListWidget();
    void handleTextChanged();

private slots:
    void autoIndent();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    //void paintEvent(QPaintEvent *event) override;

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
    bool m_completeBrace = false;
    
    void sendCurrentScrollBarValue();
    void highlightMatchedBrackets();
    void pushUndoStack();
    void restartTimer();
    int findFirstDifference(const QString& str1, const QString& str2);
    void moveCursorToPostion(int pos);
};

#endif // CODEEDITOR_H
