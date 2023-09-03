#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QScrollBar>
#include <QTextBlock>

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

protected:
    //void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event) override;

signals:
    void updateLineNumberArea(int blockCount);

private:
    QList<Brackets> bralist; // 括号匹配情况

};




#endif // CODEEDITOR_H
