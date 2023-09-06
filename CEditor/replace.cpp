#include "replace.h"
#include "ui_replace.h"

replace::replace(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::replace)
{
    ui->setupUi(this);

    QFont font = ui->lineEdit->font();
    font.setPointSize(10);
    font.setFamily("Arial");
    ui->lineEdit->setFont(font);

    QFont font2 = ui->lineEdit_2->font();
    font2.setPointSize(10);
    font2.setFamily("Arial");
    ui->lineEdit_2->setFont(font2);

}

replace::~replace()
{
    delete ui;
}

void replace::on_pushButton_3_clicked()
{
    QString str_to_search = ui->lineEdit->text();
    QString str_replace = ui->lineEdit_2->text();
    if(str_replace != nullptr && str_to_search != nullptr){
        int state = 0;
        if(ui->checkBox_case->isChecked()){//区分大小写
            state += 2;
        }
        emit sendAllReplaceDataToMain(str_to_search,str_replace,state);
    }
    else{
        QMessageBox::information(this,tr("注意"),tr("输入内容不能为空"),QMessageBox::Ok);
    }
}

void replace::on_pushButton_4_clicked()
{
     this->close();
}

void replace::on_pushButton_clicked()
{
    QString str_to_search = ui->lineEdit->text();
    QString str_replace = ui->lineEdit_2->text();
    if(str_replace != nullptr && str_to_search != nullptr){
        int state = 0;
        if(ui->checkBox_case->isChecked()){//区分大小写
            state += 2;
        }
        emit sendNextReplaceDataToMain(str_to_search,str_replace,state);
    }
    else{
        QMessageBox::information(this,tr("注意"),tr("输入内容不能为空"),QMessageBox::Ok);
    }
}
