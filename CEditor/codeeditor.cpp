#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent):
    QPlainTextEdit(parent)
{
}

//void CodeEditor::keyPressEvent(QKeyEvent *event)
//{
//    switch(event->key())
//    {
//    case Qt::Key_Enter:
//        emit lineNumberChange();
//        break;
//    }
//}

void CodeEditor::setLineNumberArea(QListWidget *lineNuberArea)
{
    m_lineNumberArea=lineNuberArea;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    emit updateLineNumberArea(this->blockCount());
}

void CodeEditor::matchBrackets() {
    int current_length = 0;
    QList<Brackets> stack; // 储存括号匹配情况的栈
    QTextDocument* document = this->document();
    int allrow = document->blockCount();
    bralist.clear(); // 每一次匹配都清空之前的情况

    int inComment = 0; // 用于跟踪是否在注释内

    for (int i = 0; i < allrow; i++) { // 遍历每一行
        QTextBlock line = this->document()->findBlockByNumber(i);
        QString string = line.text(); // 获取每一行的文本
        if(inComment == 1){
            inComment = 0;
        }
        for (int j = 0; j < string.length(); j++) { // 遍历每一个字符
            QChar currentChar = string.at(j);

            if (!inComment) {
                switch (currentChar.unicode()) {
                    case '{':
                        stack.append(Brackets(current_length + j, -1, 1)); // 向栈内加入
                        break;
                    case '(':
                        stack.append(Brackets(current_length + j, -1, 2)); // 向栈内加入
                        break;
                    case '}':
                        if (!stack.isEmpty()) {
                            int type = stack.last().type; // 判断是否对应
                            if (type == 1) {
                                bralist.append(Brackets(current_length + j, stack.last().currentPos, 1)); // 加入两个括号匹配
                                bralist.append(Brackets(stack.last().currentPos, current_length + j, 1));
                            } else {
                                bralist.append(Brackets(current_length + j, -1, 1)); // 无匹配的情况
                            }
                            stack.removeLast(); // 移除栈的最后一个
                        }
                        break;
                    case ')':
                        if (!stack.isEmpty()) {
                            int type = stack.last().type;
                            if (type == 2) {
                                bralist.append(Brackets(current_length + j, stack.last().currentPos, 2));
                                bralist.append(Brackets(stack.last().currentPos, current_length + j, 2));
                            } else {
                                bralist.append(Brackets(current_length + j, -1, 2));
                            }
                            stack.removeLast();
                        }
                        break;
                    case '/':
                        if (j < string.length() - 1) {
                            QChar nextChar = string[j + 1];
                            if (nextChar == '/') { //如果是“//”注释的情况
                                inComment = 1;
                                j++;  // 跳过单行注释符号
                            } else if (nextChar == '*') { // 如果是“/*”注释的情况
                                inComment = 2;
                                j++;  // 跳过注释的开始符号
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            else{
                if(currentChar == '*'){
                    if (j < string.length() - 1 && inComment) {
                        QChar nextChar = string[j + 1];
                        if (nextChar == '/') {
                            if(inComment == 2){
                                inComment = 0;
                            }
                            j++;  // 跳过注释的结束符号
                        }
                    }
                }
            }
        }

        current_length += string.length() + 1; // 回车符
    }
    stack.clear();
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);

    int digit=0,totalRow=blockCount();

    QTextDocument *document=this->document();

    while(totalRow!=0)
    {
        digit++;
        totalRow/=10;
    }
    //根据最大行数的位数调整大小
    m_lineNumberArea->setMaximumSize(QSize((digit+3)*6,this->height()));
    m_lineNumberArea->resize(QSize((digit+3)*6,this->height()));
    m_lineNumberArea->clear();
    for(int row=0;row<=blockCount();row++)
    {
        QListWidgetItem *item=new QListWidgetItem(QString::number(row+1),m_lineNumberArea);
        item->setText(QString::number(row+1));
        item->setSizeHint(QSize(this->width(),qRound(blockBoundingGeometry(document->findBlockByLineNumber(row)).height())));
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        if(row==blockCount())
        {
            item->setText("");
        }
        m_lineNumberArea->addItem(item);
    }
    m_lineNumberArea->verticalScrollBar()->setValue(verticalScrollBar()->value());
    if(verticalScrollBar()->value()){
        m_lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:0px;");
    }
    else{
        m_lineNumberArea->setStyleSheet("background-color: rgb(246, 245, 244);border:1px solid rgb(192, 191, 188);border-right:none;padding-top:4px;");
    }
}
