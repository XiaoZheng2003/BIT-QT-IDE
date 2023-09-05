#ifndef REPLACE_H
#define REPLACE_H

#include <QDialog>
#include <QMessageBox>

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
    void sendAllReplaceDataToMain(QString seastr,QString repstr,int state);
    void sendNextReplaceDataToMain(QString seastr,QString repstr,int state);

private slots:
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_clicked();

private:
    Ui::replace *ui;
};

#endif // REPLACE_H
