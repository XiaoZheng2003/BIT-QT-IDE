#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    search = new Search(this);
    replace = new class replace(this);

    ui->compileTextBrowser->setMaximumHeight(0);
    connect(search,SIGNAL(sendSearchDataToMain(QString,int,int)),this,SLOT(receiveSearchDataForMain(QString,int,int)));  //接受搜索信号
    connect(replace,SIGNAL(sendReplaceDataToMain(QString,QString,int,int)),this,SLOT(receiveReplaceDataForMain(QString,QString,int,int)));  //接受替换信号
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    //窗口关闭前，关闭所有标签页
    Q_UNUSED(event);
    on_actionCloseAll_triggered();
}

void MainWindow::on_actionOpen_triggered()
{
    //打开文件
    QString openFilePath = QFileDialog::getOpenFileName(
        this,"打开一个文件",QDir::currentPath(),"所有文件(*.*);;C头文件(*.h *.hpp);;C源代码(*.c *.cpp)");
    if(openFilePath.isEmpty()){
        return;
    }

    openFile(openFilePath);
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

void MainWindow::createProjectTree(QTreeWidgetItem *root, QString projectName, QString projectPath)
{
    //创建项目树
    QFileIconProvider iconProvider;
    QIcon folderIcon = iconProvider.icon(QFileIconProvider::Folder);
    root->setIcon(0,folderIcon);
    root->setCheckState(1,Qt::Checked);

    //获取项目文件夹
    QDir dir(projectPath);
    if(!dir.exists()){
        QMessageBox::information(this,"提示","文件夹不存在，请重试！");
        return;
    }

    //分别筛选头文件，源文件
    QTreeWidgetItem *headerRoot=new QTreeWidgetItem(root,QStringList()<<"头文件");
    QTreeWidgetItem *sourceRoot=new QTreeWidgetItem(root,QStringList()<<"源文件");
    headerRoot->setIcon(0,folderIcon);
    sourceRoot->setIcon(0,folderIcon);
    QFileInfoList headerFileList=dir.entryInfoList(QStringList()<<"*.hpp"<<"*.h");
    QFileInfoList sourceFileList=dir.entryInfoList(QStringList()<<"*.cpp"<<"*.c");
    QStringList childrenNameList;

    //头文件处理
    for(QFileInfo fileInfo:headerFileList){
        QString name=fileInfo.fileName();
        fileNameToPath.insert(name,fileInfo.absoluteFilePath());
        //TODO: bug 同名文件可能会发生覆盖

        QTreeWidgetItem *child=new QTreeWidgetItem(QStringList()<<name);
        child->setCheckState(1,Qt::Checked);
        child->setIcon(0,iconProvider.icon(fileInfo));
        headerRoot->addChild(child);
    }

    //源文件处理
    for(QFileInfo fileInfo:sourceFileList){
        QString name=fileInfo.fileName();
        fileNameToPath.insert(name,fileInfo.absoluteFilePath());
        //TODO: bug 同名文件可能会发生覆盖

        QTreeWidgetItem *child=new QTreeWidgetItem(QStringList()<<name);
        child->setCheckState(1,Qt::Checked);
        child->setIcon(0,iconProvider.icon(fileInfo));
        sourceRoot->addChild(child);
        childrenNameList.append(fileInfo.absoluteFilePath());
        //TODO: 理解为什么只需加源文件
    }

    projectToChildren.insert(projectName,childrenNameList);
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
    if(!newFile.open(QIODevice::WriteOnly|QIODevice::Text)){
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


    int crow=1;
    int ccol=1;
    //int call=0;
    //设置光标位置
    QString text1 = QString("行：%1，列：%2").arg(crow).arg(ccol);
    ui->rowLabel->setText(text1);
    //设置总行数
    QString text2 = QString("总行数：");
    ui->allLabel->setText(text2);

    initConnection(tab);
}

void MainWindow::on_actionClose_triggered()
{
    //关闭文件
    int curIndex=ui->tabWidget->currentIndex();
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
    //绑定编辑操作
    connect(this,&MainWindow::editOperate,tab,&Tab::editOperate);
    //更新光标位置
    connect(tab,&Tab::updateCursorSignal,this,&MainWindow::updateCursorReceive);
    //总行数更新
    connect(tab,&Tab::updateTotalLineSignal,this,&MainWindow::totalCountReceive);
    //查找替换
    connect(this,SIGNAL(sendSearchDataToTab(QString,int,int,int)),tab,SLOT(receiveSearchDataForTab(QString,int,int,int)));
    connect(this,SIGNAL(sendReplaceDataToTab(QString,QString,int,int,int)),tab,SLOT(receiveReplaceDataForTab(QString,QString,int,int,int)));
}

void MainWindow::openFile(QString openFilePath)
{

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

    int crow=1;
    int ccol=1;
    //int call=0;
    //设置光标位置
    QString text1 = QString("行：%1，列：%2").arg(crow).arg(ccol);
    ui->rowLabel->setText(text1);
    //设置总行数
    QString text2 = QString("总行数：");
     ui->allLabel->setText(text2);

    initConnection(tab);
}

void MainWindow::closeTab(int index)
{
    //关闭标签页
    QString filename=filePath[index].split("/").last();
    if(ui->tabWidget->tabText(index).startsWith("*")){
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
    ui->tabWidget->removeTab(index);
    filePath.removeAt(index);
    emit tabClosed(index);
}

void MainWindow::on_actionCompile_triggered()
{
    // 处理未打开任何文件的情况
    if (ui->tabWidget->count() == 0) {
        QMessageBox::warning(this, "警告", "未打开任何文件");
        return;
    }
    // 保存当前文件
    on_actionSave_triggered();
    // 打开编译信息框
    ui->compileTextBrowser->setMaximumHeight(220);
    // 获得当前编译文件对应序号
    int index = ui->tabWidget->currentIndex();
    // 获得文件对应路径
    QString currentFilePath = filePath.at(index);

    // 创建编译线程并连接信号槽
    CompilationThread *compileThread = new CompilationThread(0, currentFilePath, this);
    connect(compileThread, &CompilationThread::compilationFinished, this, &MainWindow::handleCompilationFinished);

    // 启动编译线程
    compileThread->start();
}


void MainWindow::handleCompilationFinished(int exitCode, const QString &outputText)
{
    if (exitCode == 0) {
        // 编译成功，处理编译结果
        ui->compileTextBrowser->setPlainText(outputText);
    } else {
        // 编译失败，显示错误信息
        QMessageBox::critical(this, "编译失败", "编译过程中出现错误");
        ui->compileTextBrowser->setPlainText(outputText);
    }
}

void MainWindow::on_actionRun_triggered()
{
    if (ui->tabWidget->count() == 0) {
        QMessageBox::warning(this, "警告", "未打开任何文件");
        return;
    }

    int index = ui->tabWidget->currentIndex();
    QString currentFilePath = filePath.at(index);
    QString exePath = currentFilePath;
    exePath.replace(".cpp", ".exe");
    QFile exefile(exePath);
    if (!exefile.exists()) {
        QMessageBox::warning(this, "提示", "还没有进行编译");
        return;
    }
    // 创建编译线程并连接信号槽
    CompilationThread *compileThread = new CompilationThread(1, exePath, this);
    connect(compileThread, &CompilationThread::compilationFinished, this, &MainWindow::handleRunFinished);

    // 启动编译线程
    compileThread->start();
}

void MainWindow::handleRunFinished(int exitCode, const QString &outputText) {
    if (exitCode == 0) {
        ui->compileTextBrowser->setPlainText("运行成功");
    } else {
        QMessageBox::critical(this, "运行失败", "运行过程中出现错误");
        ui->compileTextBrowser->setPlainText("运行失败");
    }
}

void MainWindow::on_actionCompileRun_triggered()
{
    on_actionCompile_triggered();
    on_actionRun_triggered();
}

void MainWindow::on_actionOpenProject_triggered()
{
    //打开项目
    QString projectPath = QFileDialog::getExistingDirectory(
        this,"打开一个项目",QDir::currentPath());
    if(projectPath.isEmpty()){
        return;
    }

    //获取项目名
    QString projectName=projectPath.split('/').last();
    if(projectNameToPath.count(projectName)){
        QMessageBox::information(this,"提示","同名项目已经打开！");
        return;
    }
    projectNameToPath.insert(projectName,projectPath);
    ui->currentProject->setText(projectName);

    //添加项目树
    QTreeWidgetItem *root=new QTreeWidgetItem(QStringList()<<projectName);
    createProjectTree(root,projectName,projectPath);
    ui->projectTreeWidget->insertTopLevelItem(0,root);
}

void MainWindow::on_compileInfoButton_clicked()
{
    int height=ui->compileTextBrowser->height();
    ui->compileTextBrowser->setMaximumHeight(height>0?0:220);
}

void MainWindow::on_projectTreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    //双击项目打开
    QString text=item->text(column);

    //切换当前项目
    QTreeWidgetItem *root=item;
    while(root->parent()!=nullptr)
        root=root->parent();
    ui->currentProject->setText(root->text(0));

    //双击目标非文件
    if(fileNameToPath.find(text)==fileNameToPath.end()) return;
    //获取当前文件路径
    QString itemPath=fileNameToPath.find(item->text(column)).value();
    openFile(itemPath);
}

void MainWindow::updateCursorReceive(int row,int col)
{
    //光标位置更新
    QString text=QString("行：%1，列：%2").arg(row).arg(col);
    ui->rowLabel->setText(text);

}

void MainWindow::totalCountReceive(int count)
{
    //总行数更新
    QString text=QString("总行数：%1").arg(count);
    ui->allLabel->setText(text);
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    closeTab(index);
}

void MainWindow::on_actionUndo_triggered()
{
    emit editOperate(ui->tabWidget->currentIndex(),Undo);
}

void MainWindow::on_actionRedo_triggered()
{
    emit editOperate(ui->tabWidget->currentIndex(),Redo);
}

void MainWindow::on_actionCut_triggered()
{
    emit editOperate(ui->tabWidget->currentIndex(),Cut);
}

void MainWindow::on_actionCopy_triggered()
{
    emit editOperate(ui->tabWidget->currentIndex(),Copy);
}

void MainWindow::on_actionPaste_triggered()
{
    emit editOperate(ui->tabWidget->currentIndex(),Paste);
}

void MainWindow::receiveSearchDataForMain(QString data,int state,int begin) //从搜索对话框接收搜索数据，发送给指定页面
{
    int index = ui->tabWidget->currentIndex();
    emit sendSearchDataToTab(data,index,state,begin);
}



void MainWindow::receiveReplaceDataForMain(QString sear, QString rep,int state,int begin) //接受替换数据
{
    int index = ui->tabWidget->currentIndex();
    emit sendReplaceDataToTab(sear,rep,index,state,begin);
}

void MainWindow::on_actionSearch_triggered() //搜索
{
    search->setModal(false);  //设置为非模态对话框,在其没有关闭前，用户可以与其它窗口交互
    search->show();
}

void MainWindow::on_actionReplace_triggered() //替换
{
    replace->setModal(false);
    replace->show();
}

