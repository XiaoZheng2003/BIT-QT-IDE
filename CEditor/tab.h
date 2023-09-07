#ifndef TAB_H
#define TAB_H

#include "headers.h"
#include "codeeditor.h"
#include "highlighter.h"
#include "highlighttype.h"
#include "foldlistwidgetitem.h"

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

    void jumpToLine(int indexId, int line);
    int getTotalLines();

public slots:
    void prepareTextForSave(int indexId);
    void prepareTextForSaveAs(int indexId);
    void tabClosed(int indexId);
    void editOperate(int indexId, editType type);
    void receiveStartSearchDataForTab(QString,int,int state,int);
    void receiveCloseSearchDataForTab();
    void receiveNextSearchDataForTab(QString,int,int state);
    void receiveAllReplaceDataForTab(QString,QString,int,int state);
    void receiveNextReplaceDataForTab(QString,QString,int,int state);
    void handleFoldStateChanged(QListWidgetItem* item);
    void autoComplete(int indexId);
    void commentSelectedLines(int indexId);
    void prepareTextForAStyle(int indexId);
    void receiveAStyledText(QString str);
    void handleAStyleError(int type, QString msg);

signals:
    void returnTextForSave(int indexId, QString str);
    void returnTextForSaveAs(int indexId, QString str);
    void textChanged(int indexId);
    void updateCursorSignal(int row,int ccl);
    void updateTotalLineSignal(int count);
    void scrollBarValueChanged(int value);
    void sendTextForAStyle(QString filePath, QString str);

private slots:
    void updateCursorPosition();
    void updateTotalLineCount();
    void setBlockVisible(bool flag, int start, int end);

private:
    Ui::Tab *ui;
    int curIndexId;
    std::map<QString,HighLightType> codeTable;
    std::map<HighLightType,QColor>* codeColor;
    std::map<int,std::map<HighLightType,std::map<int,int>>>* allHighLightTable;
    Highlighter* highlighter;
    QCompleter* completer;
    QString m_lineNumberAreaStyleSheetOther = "QListWidget{background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;color: rgb(170,170,170);font-weight:normal;}QListWidget::item:selected{background-color: transparent; color: rgb(80,80,80);font-weight:bold;}QListView::item:hover{background-color:transparent;}";
    QString m_lineNumberAreaStyleSheetTop = "QListWidget{background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;color: rgb(170,170,170);font-weight:normal;}QListWidget::item:selected{background-color: transparent; color: rgb(80,80,80);font-weight:bold;}QListView::item:hover{background-color:transparent;}";

    QAbstractItemModel* modelFromFile(const QString &fileName);
    void initrowVisibility();
};

#endif // TAB_H
