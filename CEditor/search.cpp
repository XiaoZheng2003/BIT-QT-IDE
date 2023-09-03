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
    if(ui->checkBox_full->isChecked()){//全字匹配
        state += 1;
    }
    if(ui->checkBox_case->isChecked()){//区分大小写
        state += 2;
    }
    if(ui->radioButton_backward->isChecked()){
        state -= 4;
    }
    if(ui->radioButton_2->isChecked()){
        begin = 1;
    }
    emit sendSearchDataToMain(to_search_str,state,begin);
}

void Search::on_pushButton_4_clicked()
{
     this->close();
}
