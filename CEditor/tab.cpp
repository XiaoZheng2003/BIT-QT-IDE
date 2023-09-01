#include "tab.h"
#include "ui_tab.h"

Tab::Tab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab)
{
    ui->setupUi(this);
    ui->lineNumberArea->setSpacing(0);
    connect(ui->plainTextEdit,&CodeEditor::updateLineNumberArea,this,&Tab::update);
    //禁用行数显示条的滚动条并隐藏
    ui->lineNumberArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->lineNumberArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->lineNumberArea->verticalScrollBar()->setDisabled(true);
    //设置行数显示条与文本编辑块一起滚动
    connect(ui->plainTextEdit->verticalScrollBar(),&QScrollBar::valueChanged,[=](int value){
        ui->lineNumberArea->verticalScrollBar()->setValue(value);
        if(value!=0){
            ui->lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;");
        }
        else{
            ui->lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;");
        }
    });
    //当文本编辑块行数改变时更新行数显示条
    connect(ui->plainTextEdit,&QPlainTextEdit::blockCountChanged,this,&Tab::update);
}

Tab::~Tab()
{
    delete ui;
}

void Tab::update(int blockCount)
{
    qDebug()<<blockCount;
    int lineHeight=ui->plainTextEdit->fontMetrics().lineSpacing();
    int digit=0,totalRow=blockCount;
    while(totalRow!=0)
    {
        digit++;
        totalRow/=10;
    }
    //根据最大行数的位数调整大小
    ui->lineNumberArea->setMaximumSize(QSize((digit+3)*6,ui->plainTextEdit->height()));
    ui->lineNumberArea->resize(QSize((digit+3)*6,ui->plainTextEdit->height()));
    ui->lineNumberArea->clear();
    for(int row=0;row<=blockCount;row++)
    {
        QListWidgetItem *item=new QListWidgetItem(QString::number(row+1),ui->lineNumberArea);
        item->setText(QString::number(row+1));
        item->setSizeHint(QSize(ui->lineNumberArea->width(),lineHeight));
        item->setTextAlignment(Qt::AlignCenter);
        if(row==blockCount)
        {
            item->setText("");
        }
        ui->lineNumberArea->addItem(item);
    }
}
