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

class Brackets // 括号匹配类
{
public:
     Brackets(int currentPosition, int correspondingPosition, int bracketType) {
         currentPos = currentPosition;
         correspondingPos = correspondingPosition;
         type = bracketType;
     }
     int currentPos = 0;
     int correspondingPos = 0;
     int type = 0; // 1: '{' / -1: '}'  2: '(' / -2: ')'
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent=nullptr);
    void matchBrackets();
    void setLineNumberArea(QListWidget *lineNumberArea);

public slots:
    void updateLineNumberArea();

protected:
    //void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    //void paintEvent(QPaintEvent *event) override;

signals:
    void scrollBarValue(int value);

private:
    QMap<int, Brackets> bramap; // 使用 QMap 存储括号匹配情况
    QListWidget *m_lineNumberArea;
    int highlightPos[4]={-1,-1,-1,-1};    //当前高亮位置

    void sendCurrentScrollBarValue();
    void highlightMatchedBrackets();
};

#endif // CODEEDITOR_H
