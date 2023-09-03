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
    // 返回总行数的逻辑
    return(ui->plainTextEdit->blockCount());
}


void Tab::on_jumpto_clicked()
{
    int linenum=ui->lineEdit->text().toInt();
     jumpToLine(linenum);
}

void Tab::jumpToLine(int line)
{
    //跳转到某一行
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
                ui->lineNumberArea->scrollToItem(item,QAbstractItemView::PositionAtCenter);
            }
            break;
        }
        cursor.movePosition(QTextCursor::NextBlock);
    }
}
