#ifndef TAB_H
#define TAB_H

#include <QWidget>
#include <QString>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextBlock>
#include "codeeditor.h"

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

public slots:
    void prepareTextForSave(int indexId);
    void prepareTextForSaveAs(int indexId);
    void tabClosed(int indexId);

signals:
    void returnTextForSave(int indexId, QString str);
    void returnTextForSaveAs(int indexId, QString str);
    void textChanged(int indexId);

private slots:
    void on_plainTextEdit_textChanged();

private:
    Ui::Tab *ui;
    void update(int blockCount);
    int curIndexId;
};

#endif // TAB_H
