#ifndef REPLACE_H
#define REPLACE_H

#include <QDialog>

namespace Ui {
class replace;
}

class replace : public QDialog
{
    Q_OBJECT

public:
    explicit replace(QWidget *parent = nullptr);
    ~replace();
signals:
    void sendReplaceDataToMain(QString seastr,QString repstr,int state,int begin);
private slots:
    void on_pushButton_clicked();

private:
    Ui::replace *ui;
};

#endif // REPLACE_H
