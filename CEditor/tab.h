#ifndef TAB_H
#define TAB_H

#include <QDialog>

namespace Ui {
class Tab;
}

class Tab : public QDialog
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
    int curIndexId;
};

#endif // TAB_H
