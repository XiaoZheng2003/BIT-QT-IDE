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

class Brackets // 括号匹配类
{
public:
     Brackets(int currentPosition, int correspondingPosition, int bracketType, int bracketLevel) {
         currentPos = currentPosition;
         correspondingPos = correspondingPosition;
         type = bracketType;
         level = bracketLevel;
     }
     int currentPos = 0;
     int correspondingPos = 0;
     int type = 0; // 1: '{' / -1: '}'  2: '(' / -2: ')'
     int level = 0; // 括号的层级
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent=nullptr);
    void matchBrackets();
    void setLineNumberArea(QListWidget *lineNumberArea);
    void undo();
//    void redo();

public slots:
    void updateLineNumberArea();
    void handleTextChanged();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    //void paintEvent(QPaintEvent *event) override;

signals:
    void scrollBarValue(int value);
    void textRealChanged();

private:
    QMap<int, Brackets> bramap; // 使用 QMap 存储括号匹配情况
    QListWidget *m_lineNumberArea;
    QString m_previousText;
    QStack<QString> m_undoStack;
//    QStack<QString> m_redoStack;
    QTimer *m_timer;

    void sendCurrentScrollBarValue();
    void highlightMatchedBrackets();
    void pushUndoStack();
//    void clearRedoStack();
    void restartTimer();
};

#endif // CODEEDITOR_H
