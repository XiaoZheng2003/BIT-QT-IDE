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
    explicit Tab(QWidget *parent = nullptr);
    ~Tab();

private:
    Ui::Tab *ui;
    CodePlainTextEdit *m_codePlainTextEdit;
    QString m_title;
    void update(int blockCount);
};

#endif // TAB_H
