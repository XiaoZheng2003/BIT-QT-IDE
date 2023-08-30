#include "tab.h"
#include "ui_tab.h"

Tab::Tab(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Tab)
{
    ui->setupUi(this);
}

Tab::~Tab()
{
    delete ui;
}
