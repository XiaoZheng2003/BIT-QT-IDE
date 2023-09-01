#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMenu"
#include "QAction"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Tab *test=new Tab;
    ui->tabWidget->addTab(test,"hello.cpp");
    ui->tabWidget->setTabText(2,"hello.cpp");
}

MainWindow::~MainWindow()
{
    delete ui;
}

