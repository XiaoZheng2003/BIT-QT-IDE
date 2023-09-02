#ifndef SEARCH_H
#define SEARCH_H

#include <QDialog>

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
    void  sendSearchDataToMain(QString,int,int);
private slots:
    void  on_search_button_clicked();

private:
    Ui::Search *ui;
};

#endif // SEARCH_H
