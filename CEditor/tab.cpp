#include "tab.h"
#include "ui_tab.h"

Tab::Tab(int index, QString text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab),
    curIndexId(index)
{
    ui->setupUi(this);
    ui->lineNumberArea->setSpacing(0);
    //禁用行数显示条的滚动条并隐藏
    ui->lineNumberArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->lineNumberArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->lineNumberArea->verticalScrollBar()->setDisabled(true);
    ui->lineNumberArea->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->plainTextEdit->setLineNumberArea(ui->lineNumberArea);
    ui->foldListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->foldListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->foldListWidget->verticalScrollBar()->setDisabled(true);
    ui->plainTextEdit->setFoldListWidget(ui->foldListWidget);

    //设置行数显示条与文本编辑块一起滚动
    connect(ui->plainTextEdit,&QPlainTextEdit::updateRequest,this,[=](QRect rec,int dy){
        ui->lineNumberArea->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->value()-dy);
        if(ui->plainTextEdit->verticalScrollBar()->value()!=0){
            ui->lineNumberArea->setStyleSheet(m_lineNumberAreaStyleSheetOther);
        }
        else{
            ui->lineNumberArea->setStyleSheet(m_lineNumberAreaStyleSheetTop);
        }
        ui->foldListWidget->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->value()-dy);
        if(ui->plainTextEdit->verticalScrollBar()->value()!=0){
            ui->foldListWidget->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;");
        }
        else{
            ui->foldListWidget->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;");
        }
    });
    connect(ui->plainTextEdit,&CodeEditor::scrollBarValue,[=](int value){
        ui->lineNumberArea->verticalScrollBar()->setValue(value);
        if(value!=0){
            ui->lineNumberArea->setStyleSheet(m_lineNumberAreaStyleSheetOther);
        }
        else{
            ui->lineNumberArea->setStyleSheet(m_lineNumberAreaStyleSheetTop);
        }
        ui->foldListWidget->verticalScrollBar()->setValue(value);
        if(value!=0){
            ui->foldListWidget->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;");
        }
        else{
            ui->foldListWidget->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;");
        }
    });
    connect(ui->plainTextEdit->verticalScrollBar(),&QScrollBar::valueChanged,this,[=](int value){
        ui->lineNumberArea->verticalScrollBar()->setValue(value);
        if(value!=0){
            ui->lineNumberArea->setStyleSheet(m_lineNumberAreaStyleSheetOther);
        }
        else{
            ui->lineNumberArea->setStyleSheet(m_lineNumberAreaStyleSheetTop);
        }
        ui->foldListWidget->verticalScrollBar()->setValue(value);
        if(value!=0){
            ui->foldListWidget->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;");
        }
        else{
            ui->foldListWidget->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;");
        }
    });
    connect(ui->lineNumberArea,&QListWidget::currentItemChanged,this,[=](){
        ui->plainTextEdit->verticalScrollBar()->setValue(ui->lineNumberArea->verticalScrollBar()->value());
    });
    connect(ui->foldListWidget,&QListWidget::currentItemChanged,this,[=](){
        ui->plainTextEdit->verticalScrollBar()->setValue(ui->foldListWidget->verticalScrollBar()->value());
    });
    //当文本编辑块行数改变时更新行数显示条
    connect(ui->plainTextEdit,&QPlainTextEdit::blockCountChanged,ui->plainTextEdit,&CodeEditor::updateLineNumberArea);
    //左侧选中单行
    connect(ui->lineNumberArea,&QListWidget::itemClicked,this,[=](QListWidgetItem *item){
        int row = item->text().toInt() - 1;
        QTextCursor cursor(ui->plainTextEdit->document()->findBlockByNumber(row));
        cursor.select(QTextCursor::LineUnderCursor);
        ui->plainTextEdit->setTextCursor(cursor);
        ui->plainTextEdit->setFocus();
    });
    //左侧选中多行
    connect(ui->lineNumberArea,&QListWidget::itemSelectionChanged,this,[=](){
        QList<QListWidgetItem *> selectedItems = ui->lineNumberArea->selectedItems();
        //选中数量<=1，证明为单选，不处理
        if(selectedItems.count() <= 1){
            return;
        }
        int startRow = selectedItems[0]->text().toInt() - 1;
        int endRow = selectedItems[selectedItems.count() - 1]->text().toInt() - 1;
        int startPos = ui->plainTextEdit->document()->findBlockByNumber(startRow).position();
        int endPos = ui->plainTextEdit->document()->findBlockByNumber(endRow).position() + ui->plainTextEdit->document()->findBlockByNumber(endRow).text().length();
        QTextCursor cursor(ui->plainTextEdit->document());
        cursor.setPosition(startPos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, endPos - startPos);
        ui->plainTextEdit->setTextCursor(cursor);
        ui->plainTextEdit->setFocus();
    });
    //光标位置更新
    connect(ui->plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &Tab::updateCursorPosition);
    //总行数更新
    connect(ui->plainTextEdit, &QPlainTextEdit::blockCountChanged, this, &Tab::updateTotalLineCount);
    connect(ui->plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &Tab::updateTotalLineCount);


    QFont font;
    font.setFamily("Consolas");
    font.setFixedPitch(true);
    font.setPointSize(14);
    //应用关键字高亮
    ui->plainTextEdit->setFont(font);
    highlighter = new Highlighter(ui->plainTextEdit->document());
    emit ui->plainTextEdit->initText(text);

    //自动补全设置
    completer=new QCompleter(this);
    completer->setModel(modelFromFile(":/wordlist.txt"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    ui->plainTextEdit->setCompleter(completer);

    connect(ui->plainTextEdit,&CodeEditor::textRealChanged,[=](){
        //当文本被修改
        emit textChanged(curIndexId);
    });


    connect(ui->foldListWidget, &FoldListWidget::itemClicked, this, &Tab::handleFoldStateChanged);
    //初始化可见性
    connect(ui->plainTextEdit,&CodeEditor::matchFinished,this,&Tab::initrowVisibility);
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

void Tab::editOperate(int indexId, editType type)
{
    if(indexId!=curIndexId) return;
    switch(type){
    case Undo:
        ui->plainTextEdit->undo();
        break;
    case Redo:
        ui->plainTextEdit->redo();
        break;
    case Cut:
        ui->plainTextEdit->cut();
        break;
    case Copy:
        ui->plainTextEdit->copy();
        break;
    case Paste:
        ui->plainTextEdit->paste();
        break;
    case SelectAll:
        ui->plainTextEdit->selectAll();
        break;
    }
}

void Tab::updateCursorPosition()
{
    //更新光标位置
    QTextCursor cursor=ui->plainTextEdit->textCursor();
    int row=cursor.blockNumber()+1;
    int col=cursor.columnNumber()+1;
    emit(updateCursorSignal(row,col));
}

void Tab::updateTotalLineCount()
{
    //更新文本框内的总行数
    int totalLineCount=ui->plainTextEdit->blockCount();
    emit(updateTotalLineSignal(totalLineCount));

}
int Tab::getTotalLines()
{
    //返回总行数
    return(ui->plainTextEdit->blockCount());
}

QAbstractItemModel *Tab::modelFromFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << QString::fromUtf8(line.trimmed());
    }
    return new QStringListModel(words, completer);
}

void Tab::jumpToLine(int indexId, int line) {
    if(indexId!=curIndexId) return;
    if (line > ui->plainTextEdit->blockCount() || line < 1) {
        QMessageBox::critical(this, "提示", "该行不存在");
        return;
    }
    QTextCursor cursor(ui->plainTextEdit->document());
    int lineNumber=0;
    while (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::StartOfLine);
        lineNumber++;
        if (lineNumber==line) {
            ui->plainTextEdit->setTextCursor(cursor);

            //获取目标行所在的列表项
            QListWidgetItem* item=ui->lineNumberArea->item(lineNumber - 1);
            if (item!=nullptr) {
                //设置列表项为当前选中项
                ui->lineNumberArea->setCurrentItem(item);
                //行数进度条和文本框同步跳转
                ui->lineNumberArea->scrollToItem(item, QAbstractItemView::PositionAtCenter);

                //遍历目标行，找到第一个非空字符的位置
                QString lineText=cursor.block().text();
                int column=lineText.indexOf(QRegularExpression("\\S"));
                if (column==-1) {
                    column=0; //如果该行没有字符，则将光标放在该行的第一列
                }

                //将光标移动到目标行的第一个字符前面
                QTextCursor targetLineCursor=ui->plainTextEdit->textCursor();
                targetLineCursor.setPosition(cursor.block().position() + column);
                ui->plainTextEdit->setTextCursor(targetLineCursor);

                //将光标选中的行居中显示
                ui->plainTextEdit->centerCursor();
                ui->lineNumberArea->scrollToItem(item, QAbstractItemView::PositionAtCenter);


                //设置编辑器的焦点
                ui->plainTextEdit->setFocus();
            }
            break;
        }
        cursor.movePosition(QTextCursor::NextBlock);
    }
}


void Tab::receiveStartSearchDataForTab(QString data,int index,int state,int begin)//开始搜索指定字符串
{
    if(index != curIndexId)
        return;
    QString real_search_str = data;
    int n = data.length();

    if(real_search_str != nullptr){
        QTextDocument *document = ui->plainTextEdit->document();
        QTextCursor cursor(document);

        //清除之前格式
        QTextCharFormat clear_format;
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.setCharFormat(clear_format);

        bool found = false;
        QTextCursor highlight_cursor(document);

        if(begin == 1) {
            highlight_cursor.setPosition(ui->plainTextEdit->textCursor().position());
        }
        else if (begin == 0 && state < 0){
            highlight_cursor.movePosition(QTextCursor::End);
        }

//        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setBackground(Qt::yellow);

        switch (state) {
        case 0:
            //向前搜索、不区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor);
            break;
        case 1:
            //向前搜索、不区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindWholeWords);
            break;
        case 2:
            //向前搜索、区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindCaseSensitively);
            break;
        case 3:
            //向前搜索、不区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindWholeWords|QTextDocument::FindCaseSensitively);
            break;
        case -4:
            //向后搜索、不区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward);
            break;
        case -3:
            //向后搜索、不区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindWholeWords);
            break;
        case -2:
            //向后搜索、区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindCaseSensitively);
            break;
        case -1:
            //向后搜索、区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindWholeWords|QTextDocument::FindCaseSensitively);
            break;
        }
        if (!highlight_cursor.isNull() && state < 0)//向前查找光标置于查找单词前
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.movePosition(QTextCursor::NoMove,QTextCursor::KeepAnchor);
            highlight_cursor.mergeCharFormat(color_format);
            highlight_cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,n);
            ui->plainTextEdit->setTextCursor(highlight_cursor);
        }
        else if (!highlight_cursor.isNull()){
            if(!found)
            {
                found = true;
            }
            highlight_cursor.movePosition(QTextCursor::NoMove,QTextCursor::KeepAnchor);
            highlight_cursor.mergeCharFormat(color_format);
            ui->plainTextEdit->setTextCursor(highlight_cursor);
        }

//        cursor.endEditBlock();
//        document->undo();

        if(found == false){
            QMessageBox::information(this,tr("注意"),tr("没有找到内容"),QMessageBox::Ok);
            qDebug("not found");
        }
    }
}

void Tab::receiveNextSearchDataForTab(QString data,int index,int state)//开始搜索指定字符串
{
    if(index != curIndexId)
        return;
    QString real_search_str = data;
    int n = data.length();

    if(real_search_str != nullptr){
        QTextDocument *document = ui->plainTextEdit->document();
        QTextCursor cursor(document);

        //清除之前格式
        QTextCharFormat clear_format;
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.setCharFormat(clear_format);

        bool found = false;
        QTextCursor highlight_cursor(document);


        highlight_cursor.setPosition(ui->plainTextEdit->textCursor().position());

//        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setBackground(Qt::yellow);

        switch (state) {
        case 0:
            //向前搜索、不区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor);
            break;
        case 1:
            //向前搜索、不区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindWholeWords);
            break;
        case 2:
            //向前搜索、区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindCaseSensitively);
            break;
        case 3:
            //向前搜索、不区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindWholeWords|QTextDocument::FindCaseSensitively);
            break;
        case -4:
            //向后搜索、不区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward);
            break;
        case -3:
            //向后搜索、不区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindWholeWords);
            break;
        case -2:
            //向后搜索、区分大小写、不全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindCaseSensitively);
            break;
        case -1:
            //向后搜索、区分大小写、全字匹配
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindBackward|QTextDocument::FindWholeWords|QTextDocument::FindCaseSensitively);
            break;
        }
        if (!highlight_cursor.isNull() && state < 0)//向前查找光标置于查找单词前
        {
            if(!found)
            {
                found = true;
            }
            highlight_cursor.movePosition(QTextCursor::NoMove,QTextCursor::KeepAnchor);
            highlight_cursor.mergeCharFormat(color_format);
            highlight_cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,n);
            ui->plainTextEdit->setTextCursor(highlight_cursor);
        }
        else if (!highlight_cursor.isNull()){
            if(!found)
            {
                found = true;
            }
            highlight_cursor.movePosition(QTextCursor::NoMove,QTextCursor::KeepAnchor);
            highlight_cursor.mergeCharFormat(color_format);
            ui->plainTextEdit->setTextCursor(highlight_cursor);
        }

//        cursor.endEditBlock();
//        document->undo();

        if(found == false){
            QMessageBox::information(this,tr("注意"),tr("没有找到内容"),QMessageBox::Ok);
            qDebug("not found");
        }
    }
}

void Tab::receiveCloseSearchDataForTab()
{

    QTextDocument *document = ui->plainTextEdit->document();
    QTextCursor cursor(document);
    QTextCharFormat clear_format;
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.setCharFormat(clear_format);
    qDebug("suc!");
}

void Tab::receiveAllReplaceDataForTab(QString sear, QString rep, int index, int state)//开始替换指定字符串
{
    if(index != curIndexId)
        return;
    QString real_search_str = sear;

    QTextDocument *document = ui->plainTextEdit->document();
    QTextCursor cursor(document);
    bool found = false;
    QTextCursor highlight_cursor(document);
    highlight_cursor.setPosition(0);

    while(!highlight_cursor.isNull() && !highlight_cursor.atEnd())
    {
        switch (state) {
        case 0:
            highlight_cursor = document->find(real_search_str,highlight_cursor);
            break;
        case 2:
            highlight_cursor = document->find(real_search_str,highlight_cursor, QTextDocument::FindCaseSensitively);
            break;
        }
        if(!found)
        {
            found = true;
        }
        highlight_cursor.movePosition(QTextCursor::NoMove,QTextCursor::KeepAnchor);
        highlight_cursor.insertText(rep);
    }
    if(found == false){
            QMessageBox::information(this,tr("注意"),tr("没有找到内容"),QMessageBox::Ok);
            qDebug("not found");
    }
    else{
            qDebug() << "替换成功！";
            QMessageBox::information(NULL, "信息", "替换成功");
    }
}

void Tab::receiveNextReplaceDataForTab(QString sear, QString rep, int index, int state)
{
    if(index != curIndexId)
        return;

    QString real_search_str = sear;
    QTextDocument *document = ui->plainTextEdit->document();
    QTextCursor cursor(document);
    bool found = false;
    QTextCursor highlight_cursor(document);
    highlight_cursor.setPosition(ui->plainTextEdit->textCursor().position());

    if (!highlight_cursor.isNull() && !highlight_cursor.atEnd())
    {
        switch (state) {
        case 0:
            highlight_cursor = document->find(real_search_str, highlight_cursor);
            break;
        case 2:
            highlight_cursor = document->find(real_search_str, highlight_cursor, QTextDocument::FindCaseSensitively);
            break;
        }

        if (highlight_cursor.isNull() || highlight_cursor.atEnd()) {
            found = false;
        } else {
            found = true;
            highlight_cursor.movePosition(QTextCursor::NoMove, QTextCursor::KeepAnchor);
            highlight_cursor.insertText(rep);
        }
    }

    if (!found) {
        QMessageBox::information(this, tr("注意"), tr("没有找到内容"), QMessageBox::Ok);
        qDebug("not found");
    } else {
        qDebug() << "替换成功！";
        QMessageBox::information(NULL, "信息", "替换成功");
    }
}

void Tab::handleFoldStateChanged(QListWidgetItem* item){
    ui->plainTextEdit->clearLineHighlight();
    FoldListWidgetItem* foldListWidgetItem = dynamic_cast<FoldListWidgetItem*>(item);
    bool flag = foldListWidgetItem->isCollapsed();
    //qDebug()<<"flag"<<flag;
    setBlockVisible(!flag, foldListWidgetItem->start, foldListWidgetItem->end);
    ui->plainTextEdit->updateLineNumberArea();
    ui->plainTextEdit->updateFoldListWidget();
}

void Tab::setBlockVisible(bool flag, int start, int end)
{
    //qDebug()<<flag<<start<<end;
    for(int i=start;i<=end;++i)
    {
        QTextBlock qtb = ui->plainTextEdit->document()->findBlockByNumber(i);
        qtb.setVisible(flag);
        //ui->plainTextEdit->setLine(start-1, !flag);
        //ui->foldListWidget->setRowVisible(i, flag);
        //ui->lineNumberArea->setRowHidden(i, flag);
    }
    ui->plainTextEdit->viewport()->update(); //重绘
    ui->plainTextEdit->document()->adjustSize(); //重新适应大小
    //
    /*
    */
}

void Tab::initrowVisibility(){
    // 初始化rowVisibility列表，默认所有行都可见
    int rowCount = ui->plainTextEdit->document()->blockCount();
    for (int i = 0; i < rowCount; ++i) {
        ui->foldListWidget->rowVisibility.append(true);
    }
}

void Tab::autoComplete(int indexId)
{
    if(indexId!=curIndexId) return;
    ui->plainTextEdit->autoComplete();
}

void Tab::commentSelectedLines(int indexId)
{
    if(indexId!=curIndexId) return;
    ui->plainTextEdit->commentSelectedLines();
}
