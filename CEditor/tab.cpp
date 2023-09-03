#include "tab.h"
#include "ui_tab.h"


Tab::Tab(int index, QString text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tab),
    curIndexId(index)
{
    ui->setupUi(this);
    ui->lineNumberArea->setSpacing(0);
    connect(ui->plainTextEdit,&CodeEditor::updateLineNumberArea,this,&Tab::update);
    //禁用行数显示条的滚动条并隐藏
    ui->lineNumberArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->lineNumberArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->lineNumberArea->verticalScrollBar()->setDisabled(true);
    //设置行数显示条与文本编辑块一起滚动
    connect(ui->plainTextEdit,&QPlainTextEdit::updateRequest,this,[=](QRect rec,int dy){
        ui->lineNumberArea->verticalScrollBar()->setValue(ui->plainTextEdit->verticalScrollBar()->value()-dy);
        if(ui->plainTextEdit->verticalScrollBar()->value()!=0){
        ui->lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;");
        }
        else{
            ui->lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;");
        }
    });
    connect(this,&Tab::scollBarValueChanged,[=](int value){
        ui->lineNumberArea->verticalScrollBar()->setValue(value);
        if(value!=0){
            ui->lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;");
        }
        else{
            ui->lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;");
        }
    });
    connect(ui->plainTextEdit->verticalScrollBar(),&QScrollBar::valueChanged,this,[=](int value){
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
    //光标位置更新
    connect(ui->plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &Tab::updateCursorPosition);
    //总行数更新
    connect(ui->plainTextEdit, &QPlainTextEdit::blockCountChanged, this, &Tab::updateTotalLineCount);
    connect(ui->plainTextEdit, &QPlainTextEdit::cursorPositionChanged, this, &Tab::updateTotalLineCount);


    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(14);
    //应用关键字高亮
    ui->plainTextEdit->setFont(font);
    highlighter = new Highlighter(ui->plainTextEdit->document());
    ui->plainTextEdit->setPlainText(text);

    connect(ui->plainTextEdit,&CodeEditor::textChanged,[=](){
        //当文本被修改
        emit textChanged(curIndexId);
    });
}

Tab::~Tab()
{
    delete ui;
}

void Tab::update(int blockCount)
{
    int digit=0,totalRow=blockCount;
    while(totalRow!=0)
    {
        digit++;
        totalRow/=10;
    }
    //根据最大行数的位数调整大小
    int lineHeight=ui->plainTextEdit->fontMetrics().lineSpacing();
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
    QTimer::singleShot(1,this,&Tab::sendScrollBarValue);
}

void Tab::sendScrollBarValue()
{
    emit scollBarValueChanged(ui->plainTextEdit->verticalScrollBar()->value());
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


void Tab::on_jumpto_clicked()
{
    int linenum=ui->lineEdit->text().toInt();
     jumpToLine(linenum);
}

void Tab::jumpToLine(int line)
{
    // 跳转到某一行
    QTextCursor cursor(ui->plainTextEdit->document());
    int lineNumber = 0;

    while (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::StartOfLine);
        lineNumber++;
        if (lineNumber == line) {
            ui->plainTextEdit->setTextCursor(cursor);
            // 获取目标行所在的列表项
            QListWidgetItem* item = ui->lineNumberArea->item(lineNumber - 1);
            if (item != nullptr) {
                // 设置列表项为当前选中项
                ui->lineNumberArea->setCurrentItem(item);
                // 行数进度条和文本框同步跳转
                ui->lineNumberArea->scrollToItem(item, QAbstractItemView::PositionAtCenter);

                // 将光标移动到目标行开头
                QTextCursor targetLineCursor = ui->plainTextEdit->textCursor();
                targetLineCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                ui->plainTextEdit->setTextCursor(targetLineCursor);

                // 将光标选中的行居中显示
                ui->plainTextEdit->centerCursor();
                ui->lineNumberArea->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            }
            break;
        }
        cursor.movePosition(QTextCursor::NextBlock);
    }
}





void Tab::receiveSearchDataForTab(QString data,int index,int state,int begin)//开始搜索指定字符串
{
    if(index != curIndexId)
        return;
    QString real_search_str = data;

    if(real_search_str != NULL){
        QTextDocument *document = ui->plainTextEdit->document();
        QTextCursor cursor(document);

        QTextCharFormat clear_format;
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.setCharFormat(clear_format);

        bool found = false;
        QTextCursor highlight_cursor(document);

        if (begin == 1) {
            highlight_cursor.setPosition(ui->plainTextEdit->textCursor().position());
        }

        cursor.beginEditBlock();
        QTextCharFormat color_format(highlight_cursor.charFormat());
        color_format.setBackground(Qt::yellow);

        while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()){
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
            if (!highlight_cursor.isNull())
            {
                if(!found)
                {
                    found = true;
                }
                highlight_cursor.movePosition(QTextCursor::NoMove,QTextCursor::KeepAnchor);
                highlight_cursor.mergeCharFormat(color_format);
            }
        }
        cursor.endEditBlock();
//        document->undo();

        if(found == false){
            QMessageBox::information(this,tr("注意"),tr("没有找到内容"),QMessageBox::Ok);
            qDebug("not found");
        }
    }
}

void Tab::receiveReplaceDataForTab(QString sear, QString rep, int index, int state)//开始替换指定字符串
{
    if(index != curIndexId)
        return;
    QStringList qslist;
    QTextDocument* doc=ui->plainTextEdit->document(); //文本对象
    int row_num=doc->blockCount () ;//回车符是一个 block
    if(state >= 0){
        for (int i=0; i<row_num;i++)
        {
            QTextBlock textLine=doc->findBlockByNumber (i) ; // 文本中的一段
            QString str=textLine.text();//将该段转换为QString
            if(state == 2)//区分大小写且不全字匹配
                str.replace(sear,rep);
            else if(state == 0){//不区分大小写且不全字匹配
                str.replace(sear,rep,Qt::CaseInsensitive);
            }
            qslist.append(str);
        }
        QStringList::Iterator it;
        for(it = qslist.begin();it != qslist.end();it++){
            if(it == qslist.begin()){
                ui->plainTextEdit->setPlainText(*it);
            }
            else{
                ui->plainTextEdit->appendPlainText(*it);
            }
        }
    }
    qDebug("suc!");
    QMessageBox::information(NULL,"information","ok");
}
