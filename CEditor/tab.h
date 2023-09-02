#ifndef TAB_H
#define TAB_H

#include <QWidget>
#include <QString>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextBlock>
#include "codeeditor.h"
#include <QDialog>
#include"highlighter.h"
#include"highlighttype.h"

class CodePlainTextEdit;

namespace Ui {
class Tab;
}

class Tab : public QWidget
{
    Q_OBJECT

public:
    explicit Tab(int index, QString text, QWidget *parent = nullptr);
    ~Tab();

    void jumpToLine(int);

public slots:
    void prepareTextForSave(int indexId);
    void prepareTextForSaveAs(int indexId);
    void tabClosed(int indexId);

signals:
    void returnTextForSave(int indexId, QString str);
    void returnTextForSaveAs(int indexId, QString str);
    void textChanged(int indexId);

private slots:
    void updateCursorPosition();
    void updateTotalLineCount();
    void on_jumpto_clicked();

private:
    Ui::Tab *ui;
    void update(int blockCount);
    int curIndexId;
    std::map<QString,HighLightType> codeTable;
    std::map<HighLightType,QColor>* codeColor;
    std::map<int,std::map<HighLightType,std::map<int,int>>>* allHighLightTable;

     Highlighter* highlighter;
};

#endif // TAB_H
