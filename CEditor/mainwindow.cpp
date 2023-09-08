#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    search = new Search(this);
    replace = new class replace(this);
    m_astyle = AStyle::getInstance(this);

    row_col = new QLabel;
    all_row = new QLabel;
    ui->statusBar->addWidget(row_col);
    ui->statusBar->addWidget(all_row);

    //初始状态隐藏编译信息栏
    ui->compileTextBrowser->setMaximumHeight(0);

    connect(search,&Search::sendStartSearchDataToMain,this,&MainWindow::receiveStartSearchDataForMain);  //接受搜索信号
    connect(search,&Search::sendNextSearchDataToMain,this,&MainWindow::receiveNextSearchDataForMain);  //接受搜索信号
    connect(search,&Search::sendCloseSearchDataToMain,this,&MainWindow::receiveCloseSearchDataForMain);  //接受搜索信号
    connect(replace,&replace::sendAllReplaceDataToMain,this,&MainWindow::receiveAllReplaceDataForMain);  //接受替换信号
    connect(replace,&replace::sendNextReplaceDataToMain,this,&MainWindow::receiveNextReplaceDataForMain);  //接受替换信号
    connect(ui->tabWidget, &QTabWidget::currentChanged, this,&MainWindow::createFunctionTree); //函数树
    //初始化项目右键菜单
    initProjectTreeMenu();
    //打开格式配置文件
    connect(m_astyle,&AStyle::openAStylePresetFile,this,&MainWindow::openFile);
    //打开格式化窗口读取属性
    connect(this,&MainWindow::openAStyleOption,m_astyle,&AStyle::readPrests);

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
    if(!saveFile.open(QIODevice::WriteOnly|QIODevice::Text)){ //防止保存bug
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
    if(!saveFile.open(QIODevice::WriteOnly|QIODevice::Text)){
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
    createFunctionTree(index);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //窗口关闭前，关闭所有标签页
    on_actionCloseAll_triggered();
    if(~ui->tabWidget->currentIndex())
        event->ignore();
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
    //开启后自动保存，防止乱码
    on_actionSave_triggered();
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
    for(QFileInfo &fileInfo:headerFileList){
        QString name=fileInfo.fileName();
        fileNameToPath.insert(projectName+"@"+name,fileInfo.absoluteFilePath());

        QTreeWidgetItem *child=new QTreeWidgetItem(QStringList()<<name);
        child->setCheckState(1,Qt::Checked);
        child->setIcon(0,iconProvider.icon(fileInfo));
        child->setData(0,Qt::UserRole,QVariant(projectName+"@"+name));
        headerRoot->addChild(child);
    }

    //源文件处理
    for(QFileInfo &fileInfo:sourceFileList){
        QString name=fileInfo.fileName();
        fileNameToPath.insert(projectName+"@"+name,fileInfo.absoluteFilePath());

        QTreeWidgetItem *child=new QTreeWidgetItem(QStringList()<<name);
        child->setCheckState(1,Qt::Checked);
        child->setIcon(0,iconProvider.icon(fileInfo));
        child->setData(0,Qt::UserRole,QVariant(projectName+"@"+name));
        sourceRoot->addChild(child);
        childrenNameList.append(fileInfo.absoluteFilePath());
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
    initConnection(tab);
    row_col->setText("行：1 列：1");
    QString text2 = QString("总行数：%1").arg(tab->getTotalLines());
    all_row->setText(text2);
}

void MainWindow::on_actionClose_triggered()
{
    //关闭文件
    int curIndex=ui->tabWidget->currentIndex();
    if(curIndex!=-1)
        closeTab(curIndex);
    else
        QMessageBox::warning(this,"提示","当前未打开文件！");
}

void MainWindow::on_actionCloseAll_triggered()
{
    //关闭所有文件
    while(~ui->tabWidget->currentIndex())
        if(!closeTab(ui->tabWidget->currentIndex()))
            break;
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
    connect(this,&MainWindow::sendStartSearchDataToTab,tab,&Tab::receiveStartSearchDataForTab);
    connect(this,&MainWindow::sendNextSearchDataToTab,tab,&Tab::receiveNextSearchDataForTab);
    connect(this,&MainWindow::sendCloseSearchDataToTab,tab,&Tab::receiveCloseSearchDataForTab);
    connect(this,&MainWindow::sendAllReplaceDataToTab,tab,&Tab::receiveAllReplaceDataForTab);
    connect(this,&MainWindow::sendNextReplaceDataToTab,tab,&Tab::receiveNextReplaceDataForTab);
    //自动补全
    connect(this,&MainWindow::autoComplete,tab,&Tab::autoComplete);
    //注释选中行
    connect(this,&MainWindow::commentSelectedLines,tab,&Tab::commentSelectedLines);
    //跳转行
    connect(this,&MainWindow::jumpToLine,tab,&Tab::jumpToLine);
    connect(this,&MainWindow::startAStyle,tab,&Tab::prepareTextForAStyle);
}

void MainWindow::openFile(QString openFilePath)
{
    //打开文件
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
    row_col->setText("行：1 列：1");
    QString text2 = QString("总行数：%1").arg(tab->getTotalLines());
    all_row->setText(text2);
}

void MainWindow::saveAllFile()
{
    //保存所有文件
    int tabCount=ui->tabWidget->count();
    for(int i=0;i<tabCount;i++)
        emit prepareTextForSave(i);
}

bool MainWindow::closeTab(int index)
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
            return false;
        case QMessageBox::Save:
            on_actionSave_triggered();
        default:
            break;
        }
    }
    ui->tabWidget->removeTab(index);
    filePath.removeAt(index);
    emit tabClosed(index);
    //清空状态栏
    row_col->setText("");
    all_row->setText("");
    return true;
}

void MainWindow::closeProject(QTreeWidgetItem *project)
{
    //关闭项目
    QString projectName=project->text(0);
    //删除相关信息
    projectNameToPath.remove(projectName);
    projectToChildren.remove(projectName);
    for(auto mapIt=fileNameToPath.begin();mapIt!=fileNameToPath.end();){
        if(!mapIt.key().startsWith(projectName)){
            mapIt++;
            continue;
        }
        fileNameToPath.erase(mapIt++);
    }
    //若为当前打开项目，则关闭
    if(ui->currentProject->text()==projectName)
        ui->currentProject->setText("无");
    removeItem(project);
    //清空状态栏
    row_col->setText("");
    all_row->setText("");
}

void MainWindow::removeItem(QTreeWidgetItem *item)
{
    //递归删除item
    int n=item->childCount();
    for(int i=0;i<n;i++)
        removeItem(item->child(0));
    delete item;
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
    CompilationThread *compileThread = new CompilationThread(0, currentFilePath, QStringList(), this);
    connect(compileThread, &CompilationThread::compilationFinished, this, &MainWindow::handleCompilationFinished);

    // 启动编译线程
    compileThread->start();
}


void MainWindow::handleCompilationFinished(int exitCode, const QString &outputText)
{
    Q_UNUSED(outputText);
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
    CompilationThread *compileThread = new CompilationThread(1, exePath, QStringList(), this);
    connect(compileThread, &CompilationThread::compilationFinished, this, &MainWindow::handleRunFinished);

    // 启动编译线程
    compileThread->start();
}

void MainWindow::handleRunFinished(int exitCode, const QString &outputText)
{
    Q_UNUSED(outputText);
    if (exitCode == 0) {
        ui->compileTextBrowser->setPlainText("运行成功");
    } else {
        QMessageBox::critical(this, "运行失败", "运行过程中出现错误");
        ui->compileTextBrowser->setPlainText("运行失败");
    }
}

void MainWindow::initProjectTreeMenu()
{
    //初始化项目树右键菜单
    QMenu *popMenu=new QMenu(this);

    //项目操作
    QAction *actionCloseProject=new QAction("关闭项目",this);
    QAction *actionOpenFile=new QAction("在资源管理器中显示",this);
    //文件操作
    QAction *actionOpenNode=new QAction("打开文件",this);
    //公有
    QAction *actionExpandAll=new QAction("展开全部",this);
    QAction *actionCollapseAll=new QAction("折叠全部",this);

    connect(ui->projectTreeWidget,&QTreeWidget::customContextMenuRequested,[=](QPoint pos){
        QTreeWidgetItem *curItem=ui->projectTreeWidget->itemAt(pos);
        popMenu->clear();
        if(curItem!=nullptr){
            //分层级添加
            switch(getItemLevel(curItem)){
            case 0:
                popMenu->addAction(actionCloseProject);
                popMenu->addAction(actionOpenFile);
                break;
            case 2:
                popMenu->addAction(actionOpenNode);
            }
        }
        popMenu->addAction(actionExpandAll);
        popMenu->addAction(actionCollapseAll);
        QAction *selectAction=popMenu->exec(QCursor::pos());
        if(selectAction==actionCloseProject){
            //关闭项目
            closeProject(curItem);
        }
        else if(selectAction==actionOpenFile){
            //在资源管理器中显示
            QString projectName=curItem->text(0);
            QDesktopServices::openUrl(projectNameToPath.value(projectName));
        }
        else if(selectAction==actionOpenNode){
            //打开文件
            on_projectTreeWidget_itemDoubleClicked(curItem,0);
        }
        else if(selectAction==actionExpandAll){
            //展开全部
            ui->projectTreeWidget->expandAll();
        }
        else if(selectAction==actionCollapseAll){
            //折叠全部
            ui->projectTreeWidget->collapseAll();
        }
    });
}

int MainWindow::getItemLevel(QTreeWidgetItem *item)
{
    int level=0;
    while(item->parent()!=nullptr){
        level++;
        item=item->parent();
    }
    return level;
}

void MainWindow::on_actionCompileRun_triggered()
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
    CompilationThread *compileThread1 = new CompilationThread(0, currentFilePath, QStringList(), this);
    connect(compileThread1, &CompilationThread::compilationFinished, this, [=](int exitCode, const QString &outputText) {
        if (exitCode == 0) {
            // 编译成功，处理编译结果
            ui->compileTextBrowser->setPlainText(outputText);
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
            CompilationThread *compileThread2 = new CompilationThread(1, exePath, QStringList(), this);
            connect(compileThread2, &CompilationThread::compilationFinished, this, [=](int exitCode, const QString &outputText){
                Q_UNUSED(outputText);
                if (exitCode == 0) {
                    ui->compileTextBrowser->setPlainText("运行成功");
                } else {
                    QMessageBox::critical(this, "运行失败", "运行过程中出现错误");
                    ui->compileTextBrowser->setPlainText("运行失败");
                }
            });
            // 启动编译线程
            compileThread2->start();
        } else {
            // 编译失败，显示错误信息
            QMessageBox::critical(this, "编译失败", "编译过程中出现错误");
            ui->compileTextBrowser->setPlainText(outputText);
        }
    });
    // 启动编译线程
    compileThread1->start();
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
    static QRegularExpression re("[\\x{4e00}-\\x{9fa5}]+");
    QRegularExpressionMatch match = re.match(projectName);
    if(match.hasMatch()){
        QMessageBox::information(this,"提示","项目名禁止包含中文！");
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
    QString text=item->data(column,Qt::UserRole).toString();

    //切换当前项目
    QTreeWidgetItem *root=item;
    while(root->parent()!=nullptr)
        root=root->parent();
    ui->currentProject->setText(root->text(0));

    //双击目标非文件
    if(fileNameToPath.find(text)==fileNameToPath.end()) return;
    //获取当前文件路径
    QString itemPath=fileNameToPath.find(text).value();
    openFile(itemPath);
}

void MainWindow::updateCursorReceive(int row,int col)
{
    //光标位置更新
    QString text=QString("行：%1，列：%2").arg(row).arg(col);
    row_col->setText(text);

}

void MainWindow::totalCountReceive(int count)
{
    //总行数更新;
    QString text=QString("总行数：%1").arg(count);
    all_row->setText(text);
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    //关闭标签页
    closeTab(index);
}

void MainWindow::on_actionUndo_triggered()
{
    //撤销
    emit editOperate(ui->tabWidget->currentIndex(),EditType::Undo);
}

void MainWindow::on_actionRedo_triggered()
{
    //恢复
    emit editOperate(ui->tabWidget->currentIndex(),EditType::Redo);
}

void MainWindow::on_actionCut_triggered()
{
    //剪切
    emit editOperate(ui->tabWidget->currentIndex(),EditType::Cut);
}

void MainWindow::on_actionCopy_triggered()
{
    //复制
    emit editOperate(ui->tabWidget->currentIndex(),EditType::Copy);
}

void MainWindow::on_actionPaste_triggered()
{
    //粘贴
    emit editOperate(ui->tabWidget->currentIndex(),EditType::Paste);
}

void MainWindow::receiveStartSearchDataForMain(QString data,int state,int begin) //从搜索对话框接收搜索数据，发送给指定页面
{
    int index = ui->tabWidget->currentIndex();
    emit sendStartSearchDataToTab(data,index,state,begin);
}

void MainWindow::receiveNextSearchDataForMain(QString data,int state) //从搜索对话框接收搜索数据，发送给指定页面
{
    int index = ui->tabWidget->currentIndex();
    emit sendNextSearchDataToTab(data,index,state);
}

void MainWindow::receiveCloseSearchDataForMain() //从搜索对话框接收搜索数据，发送给指定页面
{
    emit sendCloseSearchDataToTab();
}

void MainWindow::receiveAllReplaceDataForMain(QString sear, QString rep,int state) //接受替换数据
{
    int index = ui->tabWidget->currentIndex();
    emit sendAllReplaceDataToTab(sear,rep,index,state);
}

void MainWindow::receiveNextReplaceDataForMain(QString sear, QString rep,int state) //接受替换数据
{
    int index = ui->tabWidget->currentIndex();
    emit sendNextReplaceDataToTab(sear,rep,index,state);
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

void MainWindow::on_actionCompileProject_triggered()
{
    //编译项目
    QString projectName=ui->currentProject->text();
    if(projectName=="无"){
        QMessageBox::warning(this,"警告","当前未打开任何项目！");
        return;
    }
    //保存所有文件
    saveAllFile();
    //打开编译信息框
    ui->compileTextBrowser->setMaximumHeight(220);

    QString projectPath=projectNameToPath.find(projectName).value();
    //编译项目
    QString exe = projectPath + "/" + projectName + ".exe";
    // 设置要执行的命令和参数
    QStringList arguments;
    arguments << "-o" << exe << "-g"; // 添加命令行参数
    QStringList list = projectToChildren.value(projectName);
    for (int i = 0; i < list.size(); i++) {
        arguments << list[i];
    }

    // 创建编译线程并连接信号槽
    CompilationThread *compileThread = new CompilationThread(2, exe, arguments, this);
    connect(compileThread, &CompilationThread::compilationFinished, this, &MainWindow::handleProjectCompilationFinished);

    // 启动编译线程
    compileThread->start();
}

void MainWindow::handleProjectCompilationFinished(int exitCode, const QString &outputText)
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

void MainWindow::on_actionRunProject_triggered()
{
    //运行项目
    QString projectName=ui->currentProject->text();
    if(projectName=="无"){
        QMessageBox::warning(this,"警告","当前未打开任何项目！");
        return;
    }
    QString projectPath=projectNameToPath.find(projectName).value();
    //运行项目

    QString exePath = projectPath + "/" +projectName+".exe";;
    QFile exefile(exePath);
    if (!exefile.exists()) {
        QMessageBox::warning(this, "提示", "还没有进行编译");
        return;
    }
    // 创建编译线程并连接信号槽
    CompilationThread *compileThread = new CompilationThread(1, exePath, QStringList(), this);
    connect(compileThread, &CompilationThread::compilationFinished, this, &MainWindow::handleRunFinished);
    // 启动编译线程
    compileThread->start();
}

void MainWindow::handleProjectRunFinished(int exitCode, const QString &outputText)
{
    Q_UNUSED(outputText);
    if (exitCode == 0) {
        ui->compileTextBrowser->setPlainText("运行成功");
    } else {
        QMessageBox::critical(this, "运行失败", "运行过程中出现错误");
        ui->compileTextBrowser->setPlainText("运行失败");
    }
}

void MainWindow::on_actionCompileRunProject_triggered()
{
    //编译项目
    QString projectName=ui->currentProject->text();
    if(projectName=="无"){
        QMessageBox::warning(this,"警告","当前未打开任何项目！");
        return;
    }
    //保存所有文件
    saveAllFile();
    //打开编译信息框
    ui->compileTextBrowser->setMaximumHeight(220);

    QString projectPath=projectNameToPath.find(projectName).value();
    //编译项目
    QString exe = projectPath + "/" + projectName + ".exe";
    // 设置要执行的命令和参数
    QStringList arguments;
    arguments << "-o" << exe << "-g"; // 添加命令行参数
    QStringList list = projectToChildren.value(projectName);
    for (int i = 0; i < list.size(); i++) {
        arguments << list[i];
    }

    // 创建编译线程并连接信号槽
    CompilationThread *compileThread1 = new CompilationThread(2, exe, arguments, this);
    connect(compileThread1, &CompilationThread::compilationFinished, this, [=](int exitCode, const QString &outputText) {
        if (exitCode == 0) {
            // 编译成功，处理编译结果
            ui->compileTextBrowser->setPlainText(outputText);
            //运行项目
            QString projectName=ui->currentProject->text();
            if(projectName=="无"){
                QMessageBox::warning(this,"警告","当前未打开任何项目！");
                return;
            }
            QString projectPath=projectNameToPath.find(projectName).value();
            //运行项目

            QString exePath = projectPath + "/" +projectName+".exe";;
            QFile exefile(exePath);
            if (!exefile.exists()) {
                QMessageBox::warning(this, "提示", "还没有进行编译");
                return;
            }
            // 创建编译线程并连接信号槽
            CompilationThread *compileThread2 = new CompilationThread(1, exePath, QStringList(), this);
            connect(compileThread2, &CompilationThread::compilationFinished, this, [=](int exitCode, const QString &outputText) {
                Q_UNUSED(outputText);
                if (exitCode == 0) {
                    ui->compileTextBrowser->setPlainText("运行成功");
                } else {
                    QMessageBox::critical(this, "运行失败", "运行过程中出现错误");
                    ui->compileTextBrowser->setPlainText("运行失败");
                }
            });
            // 启动编译线程
            compileThread2->start();
        } else {
            // 编译失败，显示错误信息
            QMessageBox::critical(this, "编译失败", "编译过程中出现错误");
            ui->compileTextBrowser->setPlainText(outputText);
        }
    });

    // 启动编译线程
    compileThread1->start();
}

void MainWindow::on_actionAutoComplete_triggered()
{
    //自动补全
    emit autoComplete(ui->tabWidget->currentIndex());
}

void MainWindow::on_actionSelectAll_triggered()
{
    //全选
    emit editOperate(ui->tabWidget->currentIndex(),EditType::SelectAll);
}

void MainWindow::on_actionComment_triggered()
{
    //一键注释/反注释
    emit commentSelectedLines(ui->tabWidget->currentIndex());
}

void MainWindow::on_actionJumpLine_triggered()
{
    //跳转行
    bool ok=false;
    int line=QInputDialog::getInt(this,"跳转行","跳转行",1,0,2147483647,1,&ok);
    if(ok) emit jumpToLine(ui->tabWidget->currentIndex(),line);
}

void MainWindow::on_actionAstyle_triggered()
{
    if (ui->tabWidget->count() == 0) {
        QMessageBox::warning(this, "警告", "未打开任何文件");
        return;
    }
    emit startAStyle(ui->tabWidget->currentIndex());
}

void MainWindow::on_actionAstyleOption_triggered()
{
    emit openAStyleOption();
    m_astyle->exec();
}

void MainWindow::on_actionHelp_triggered()
{
    //打开帮助文档
    QProcess *helpProcess=new QProcess(this);
    QStringList argument("cppreference.chm");
    helpProcess->start("hh.exe",argument);
}

void MainWindow::on_actionAbout_triggered()
{
    //关于CEditor
    QMessageBox::about(this,"关于CEditor",
        QString("本项目为北京理工大学小学期软件工程综合实践IDE项目，由基因重组历时2周开发完成，具有一般IDE所具备的功能。\n"
                "在此感谢所有开发者作出的贡献！\n"
                "开发者：郑俊烽、陈玺、郭荆、蔡昕怡、吕春吉、马翊程"));
}

void MainWindow::on_actionAboutQt_triggered()
{
    //关于Qt
    QMessageBox::aboutQt(this);
}

void MainWindow::createFunctionTree(int index)
{
    QWidget *tabWidget = ui->tabWidget->widget(index);
    ui->functionTreeWidget->clear();
    if(tabWidget == nullptr){
        return;
    }
    QString str = tabWidget->findChild<CodeEditor*>("plainTextEdit")->toPlainText();
    findFunction.clear();
    QStringList rows = str.split("\n");
    QList<QTreeWidgetItem*> functionTree;
    QRegularExpression pattern("(\\b(const)?\\s*(unsigned)?\\s*(int|short|long|long long|void|float|double|char|bool)\\s+(\\w+|\\w+::\\w+)\\s*(\\(.*?\\))(?=\\s*))");

    for (int i = 0; i < rows.size(); ++i)
    {
        QString rowStr = rows.at(i);
        // 使用正则表达式创建匹配迭代器
        QRegularExpressionMatchIterator matchIterator = pattern.globalMatch(rowStr);
        // 迭代匹配项
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            QString matchedText = match.captured();
            findFunction.insert(matchedText,i+1);
            QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<<matchedText);
            item->setIcon(0, QIcon(":/pic/function.png"));
            functionTree.append(item);
        }
    }
    ui->functionTreeWidget->addTopLevelItems(functionTree);
}

void MainWindow::on_functionTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString functionName = item->text(column);
    int line = findFunction.find(functionName).value();
    int index = ui->tabWidget->currentIndex();
    emit jumpToLine(index,line);
}
