#ifndef SEARCH_H
#define SEARCH_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class Search;
}

class Search : public QDialog
{
    Q_OBJECT

public:
    explicit Search(QWidget *parent = nullptr);
    ~Search();

signals:
    void  sendStartSearchDataToMain(QString,int,int);

private slots:
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

private:
    Ui::Search *ui;
};

#endif // SEARCH_H
