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
#include <QChar>

class Brackets // 括号匹配类
{
public:
    Brackets():currentPos(0),correspondingPos(-1),type(0){};
    Brackets(int currentPosition, int correspondingPosition, int bracketType) {
        currentPos = currentPosition;
        correspondingPos = correspondingPosition;
        type = bracketType;
    }
    int currentPos = 0;
    int correspondingPos = -1;
    int type = 0; // 1: '{' / -1: '}'  2: '(' / -2: ')'
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent=nullptr);
    void matchBrackets();
    void setLineNumberArea(QListWidget *lineNumberArea);
    void undo();
    void redo();

public slots:
    void updateLineNumberArea();
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

private:
    QMap<int, Brackets> bramap; // 使用 QMap 存储括号匹配情况
    QListWidget *m_lineNumberArea;
    QString m_previousText;
    QStack<QString> m_undoStack;
    QStack<QString> m_redoStack;
    QTimer *m_timer;
    bool m_cursorMoved = true;// 记录成对符号自动补全后光标是否发生移动

    void sendCurrentScrollBarValue();
    void highlightMatchedBrackets();
    void pushUndoStack();
    void restartTimer();
    bool bracketComplete(QKeyEvent *event);
    int findFirstDifference(const QString &str1, const QString &str2);
    void moveCursorToPostion(int pos);
};

#endif // CODEEDITOR_H
