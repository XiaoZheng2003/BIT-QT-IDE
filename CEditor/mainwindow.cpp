#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::returnTextForSave(int indexId, QString str)
{
    //保存文件
    QString saveFilePath=filePath[indexId];
    QFile saveFile(saveFilePath);
    if(!saveFile.open(QIODevice::ReadWrite|QIODevice::Text)){
        QMessageBox::warning(this,"提示",QString("无法保存文件 %1:\n%2.")
                             .arg(saveFilePath,saveFile.errorString()));
        return;
    }

    //保存为UTF-8编码
    QByteArray strBytes;
    strBytes=str.toUtf8();
    saveFile.write(strBytes,strBytes.length());
    saveFile.close();

    //保存文件，去除*
    refreshFilename(indexId);
}

void MainWindow::returnTextForSaveAs(int indexId, QString str)
{
    //另存为文件
    QString saveFilePath = QFileDialog::getSaveFileName(
        this,"另存为一个文件",QDir::currentPath(),"所有文件(*.*);;C头文件(*.h *.hpp);;C源代码(*.c *.cpp)");
    if(saveFilePath.isEmpty()){
        return;
    }
    QFile saveFile(saveFilePath);
    if(!saveFile.open(QIODevice::ReadWrite|QIODevice::Text)){
        QMessageBox::warning(this,"提示",QString("无法保存文件 %1:\n%2.")
                             .arg(saveFilePath,saveFile.errorString()));
        return;
    }

    //保存为UTF-8编码
    QByteArray strBytes;
    strBytes=str.toUtf8();
    saveFile.write(strBytes,strBytes.length());
    saveFile.close();

    //绑定文件路径
    filePath[indexId]=saveFilePath;

    //保存文件，去除*
    refreshFilename(indexId);
}

void MainWindow::tabTextChanged(int index)
{
    //文件被改变，加上*
    QString filename=filePath[index].split("/").last();
    ui->tabWidget->setTabText(index,"* "+filename);
}

void MainWindow::on_actionOpen_triggered()
{
    //打开文件
    QString openFilePath = QFileDialog::getOpenFileName(
        this,"打开一个文件",QDir::currentPath(),"所有文件(*.*);;C头文件(*.h *.hpp);;C源代码(*.c *.cpp)");
    if(openFilePath.isEmpty()){
        return;
    }

    QFile openFile(openFilePath);
    if(!openFile.exists()){
        QMessageBox::warning(this,"提示","文件不存在！");
        return;
    }
    if(!openFile.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,"提示",QString("无法读取文件 %1:\n%2.")
                             .arg(openFilePath,openFile.errorString()));
        return;
    }

    //获取文件内容
    QByteArray strBytes=openFile.readAll();
    QString str=getCorrentUnicode(strBytes);
    openFile.close();

    //获取文件名
    QString singleFileName=openFilePath.split("/").last();

    //添加新标签页
    int newTabNo=ui->tabWidget->count();
    Tab *tab=new Tab(newTabNo,str);
    int index=ui->tabWidget->insertTab(newTabNo,tab,singleFileName);
    ui->tabWidget->setCurrentIndex(index);

    //绑定文件路径
    filePath.append(openFilePath);

    initConnection(tab);
}

QString MainWindow::getCorrentUnicode(const QByteArray &text)
{
    //获取文件编码并返回字符串
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString strtext = codec->toUnicode( text.constData(), text.size(), &state);
    if (state.invalidChars > 0)
        strtext = QTextCodec::codecForName( "GBK" )->toUnicode(text);
    else
        strtext = text;
    return strtext;
}

void MainWindow::on_actionNewFile_triggered()
{
    //新建文件
    QString newFilePath = QFileDialog::getSaveFileName(
        this,"新建一个文件",QDir::currentPath(),"所有文件(*.*);;C头文件(*.h *.hpp);;C源代码(*.c *.cpp)");
    if(newFilePath.isEmpty()){
        return;
    }

    QFile newFile(newFilePath);
    if(newFile.exists()){
        QMessageBox::warning(this,"提示","文件已存在！");
        return;
    }
    if(!newFile.open(QIODevice::ReadWrite|QIODevice::Text)){
        QMessageBox::warning(this,"提示",QString("文件创建失败 %1:\n%2.")
                             .arg(newFilePath,newFile.errorString()));
        return;
    }
    newFile.close();

    //获取文件名
    QString singleFileName=newFilePath.split("/").last();

    //添加新标签页
    int newTabNo=ui->tabWidget->count();
    Tab *tab=new Tab(newTabNo,"");
    int index=ui->tabWidget->insertTab(newTabNo,tab,singleFileName);
    ui->tabWidget->setCurrentIndex(index);

    //绑定文件路径
    filePath.append(newFilePath);

    initConnection(tab);
}

void MainWindow::on_actionClose_triggered()
{
    //关闭文件
    int curIndex=ui->tabWidget->currentIndex();
    QString filename=filePath[curIndex].split("/").last();
    if(ui->tabWidget->tabText(curIndex).startsWith("*")){
        //当前文件未保存
        int ret=QMessageBox::information(this,"提示",
                                           QString("%1 已被修改，是否保存对此文件所做的更改？").arg(filename),
                                           QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel,
                                           QMessageBox::Save);
        switch(ret){
        case QMessageBox::Cancel:
            return;
        case QMessageBox::Save:
            on_actionSave_triggered();
        }
    }
    closeTab(curIndex);
}

void MainWindow::on_actionCloseAll_triggered()
{
    //关闭所有文件
    while(~ui->tabWidget->currentIndex())
        on_actionClose_triggered();
}

void MainWindow::on_actionSave_triggered()
{
    //文件保存
    emit prepareTextForSave(ui->tabWidget->currentIndex());
}

void MainWindow::on_actionSaveas_triggered()
{
    //文件另存为
    emit prepareTextForSaveAs(ui->tabWidget->currentIndex());
}

void MainWindow::refreshFilename(int index)
{
    //刷新文件名，去除*
    QString filename=filePath[index].split("/").last();
    ui->tabWidget->setTabText(index,filename);
}

void MainWindow::initConnection(Tab *tab)
{
    //保存文件
    connect(this,&MainWindow::prepareTextForSave,tab,&Tab::prepareTextForSave);
    connect(tab,&Tab::returnTextForSave,this,&MainWindow::returnTextForSave);
    connect(this,&MainWindow::prepareTextForSaveAs,tab,&Tab::prepareTextForSaveAs);
    connect(tab,&Tab::returnTextForSaveAs,this,&MainWindow::returnTextForSaveAs);
    //关闭文件
    connect(this,&MainWindow::tabClosed,tab,&Tab::tabClosed);
    //文件未保存加*
    connect(tab,&Tab::textChanged,this,&MainWindow::tabTextChanged);
}

void MainWindow::closeTab(int index)
{
    //关闭标签页
    ui->tabWidget->removeTab(index);
    filePath.removeAt(index);
    emit tabClosed(index);
}
