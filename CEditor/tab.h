#ifndef TAB_H
#define TAB_H

#include <QDialog>
#include <QWidget>
#include <QString>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextBlock>
#include <QTimer>
#include <QDialog>
#include <QFont>
#include <QMessageBox>
#include "codeeditor.h"
#include "highlighter.h"
#include "highlighttype.h"

enum editType{
    Redo,
    Undo,
    Cut,
    Copy,
    Paste
};

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
    int getTotalLines();

public slots:
    void prepareTextForSave(int indexId);
    void prepareTextForSaveAs(int indexId);
    void tabClosed(int indexId);
    void editOperate(int indexId, editType type);
    void receiveSearchDataForTab(QString,int,int state,int);
    void receiveReplaceDataForTab(QString,QString,int,int state,int);

signals:
    void returnTextForSave(int indexId, QString str);
    void returnTextForSaveAs(int indexId, QString str);
    void textChanged(int indexId);
    void scollBarValueChanged(int value);
    void updateCursorSignal(int row,int ccl);
    void updateTotalLineSignal(int count);

private slots:
    void updateCursorPosition();
    void updateTotalLineCount();
    void on_jumpto_clicked();

private:
    Ui::Tab *ui;
    void update(int blockCount);
    int curIndexId;
    void sendScrollBarValue();
    std::map<QString,HighLightType> codeTable;
    std::map<HighLightType,QColor>* codeColor;
    std::map<int,std::map<HighLightType,std::map<int,int>>>* allHighLightTable;

    Highlighter* highlighter;
};

#endif // TAB_H
