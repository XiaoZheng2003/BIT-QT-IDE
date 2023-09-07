#include "astyle.h"
#include "ui_astyle.h"

AStyle::AStyle(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AStyle)
{
    ui->setupUi(this);
}

AStyle::~AStyle()
{
    delete ui;
}

void AStyle::on_buttonBox_accepted()
{

}

