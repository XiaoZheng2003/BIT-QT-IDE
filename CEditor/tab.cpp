#include "tab.h"
#include "ui_tab.h"

Tab::Tab(int index, QString text, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Tab),
    curIndexId(index)
{
    ui->setupUi(this);

    ui->plainTextEdit->setPlainText(text);
}

Tab::~Tab()
{
    delete ui;
}

void Tab::prepareTextForSave(int indexId)
{
    //返回文件保存文本
    if(indexId!=curIndexId) return;
    emit returnTextForSave(indexId,ui->plainTextEdit->toPlainText());
}

void Tab::prepareTextForSaveAs(int indexId)
{
    //返回文件另存为文本
    if(indexId!=curIndexId) return;
    emit returnTextForSaveAs(indexId,ui->plainTextEdit->toPlainText());
}

void Tab::tabClosed(int indexId)
{
    //关闭标签
    if(indexId==curIndexId) curIndexId=-1; //当前标签被关闭
    if(indexId<curIndexId) curIndexId--;   //当前标签前面的标签被关闭
}

void Tab::on_plainTextEdit_textChanged()
{
    //当文本被修改
    emit textChanged(curIndexId);
}

