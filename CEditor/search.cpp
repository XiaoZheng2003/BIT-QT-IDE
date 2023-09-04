#include "search.h"
#include "ui_search.h"

Search::Search(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Search)
{
    ui->setupUi(this);
    QFont font = ui->lineEdit->font();
    font.setPointSize(10);
    font.setFamily("Arial");
    ui->lineEdit->setFont(font);
}

Search::~Search()
{
    delete ui;
}

void Search::on_pushButton_3_clicked()
{
    QString to_search_str = ui->lineEdit->text();
    int state = 0, begin = 0;
    if(to_search_str != nullptr){
        if(ui->checkBox_full->isChecked()){//全字匹配
            state += 1;
        }
        if(ui->checkBox_case->isChecked()){//区分大小写
            state += 2;
        }
        if(ui->radioButton_forward->isChecked()){//向后查找
            state -= 4;
        }
        if(ui->radioButton_2->isChecked()){//从光标开始
            begin = 1;
        }
        emit sendStartSearchDataToMain(to_search_str,state,begin);
    }
    else{
        QMessageBox::information(this,tr("注意"),tr("输入内容不能为空"),QMessageBox::Ok);
    }
}

void Search::on_pushButton_4_clicked()
{
     this->close();
}

void Search::on_pushButton_clicked()
{
    QString to_search_str = ui->lineEdit->text();
    int state = 0;
    if(to_search_str != nullptr){
        if(ui->checkBox_full->isChecked()){//全字匹配
            state += 1;
        }
        if(ui->checkBox_case->isChecked()){//区分大小写
            state += 2;
        }
        if(ui->radioButton_forward->isChecked()){//向后查找
            state -= 4;
        }
        emit sendNextSearchDataToMain(to_search_str,state);
    }
    else{
        QMessageBox::information(this,tr("注意"),tr("输入内容不能为空"),QMessageBox::Ok);
    }
}

void Search::closeEvent(QCloseEvent *event)
{
    emit sendCloseSearchDataToMain();
    QDialog::closeEvent(event);
}
