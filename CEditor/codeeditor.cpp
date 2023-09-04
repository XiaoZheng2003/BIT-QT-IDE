#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent):
    QPlainTextEdit(parent)
{
    //文本改变，自动匹配括号
    connect(this,&CodeEditor::textChanged,this,&CodeEditor::matchBrackets);
    //光标移动，高亮匹配括号
    connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::highlightMatchedBrackets);
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

void CodeEditor::setLineNumberArea(QListWidget *lineNumberArea)
{
    m_lineNumberArea=lineNumberArea;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    updateLineNumberArea();
}

void CodeEditor::wheelEvent(QWheelEvent *event)
{
    if(event->modifiers()==Qt::ControlModifier)
    {
        int delta=event->angleDelta().y();
        QFont documentFont=this->font();

        if(delta>0)
        {
            documentFont.setPointSize(documentFont.pointSize()+1);
        }
        else
        {
            if(documentFont.pointSize()>2)
            documentFont.setPointSize(documentFont.pointSize()-1);
        }
        setFont(documentFont);
        updateLineNumberArea();
    }
    else
    {
        QPlainTextEdit::wheelEvent(event);
    }
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

void CodeEditor::updateLineNumberArea()
{
    int digit=0,totalRow=blockCount();

    QTextDocument *document=this->document();

    while(totalRow!=0)
    {
        digit++;
        totalRow/=10;
    }
    //根据最大行数的位数调整大小
    QFont documentFont=this->font();
    QFontMetrics metrics(documentFont);
    int maxItemSize=0;

    m_lineNumberArea->clear();
    for(int row=0;row<=blockCount();row++)
    {
        QListWidgetItem *item=new QListWidgetItem(QString::number(row+1),m_lineNumberArea);
        item->setFont(documentFont);
        item->setText(QString::number(row+1));
        int itemSize =metrics.size(Qt::TextSingleLine,QString::number(row+1),0).width();
        if(itemSize>maxItemSize)
            maxItemSize=itemSize;
        item->setSizeHint(QSize(this->width(),qRound(blockBoundingGeometry(document->findBlockByLineNumber(row)).height())));
        item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
        if(row==blockCount()-1)
        {
            item->setSizeHint(QSize(this->width(),this->fontMetrics().lineSpacing()));
        }
        if(row==blockCount())
        {
            item->setSizeHint(QSize(this->width(),this->fontMetrics().lineSpacing()));
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
            item->setText("");
        }
        m_lineNumberArea->addItem(item);
    }
    qDebug()<<maxItemSize;
    //对字体过小的情况特殊处理
    if(maxItemSize>15)
    {
        m_lineNumberArea->setMaximumSize(QSize(1.5*maxItemSize,this->height()));
        m_lineNumberArea->resize(QSize(1.5*maxItemSize,this->height()));
    }
    else
    {
        if(maxItemSize*2.2>22)
        {
            m_lineNumberArea->setMaximumSize(QSize(22,this->height()));
            m_lineNumberArea->resize(QSize(22,this->height()));
        }
        else
        {
            m_lineNumberArea->setMaximumSize(QSize(2.2*maxItemSize,this->height()));
            m_lineNumberArea->resize(QSize(2.2*maxItemSize,this->height()));
        }
    }
    QTimer::singleShot(1,this,&CodeEditor::sendCurrentScrollBarValue);
}

void CodeEditor::sendCurrentScrollBarValue()
{
    emit scrollBarValue(verticalScrollBar()->value());
}

void CodeEditor::highlightMatchedBrackets()
{
    //将匹配括号高亮
    QTextCursor cursor=this->textCursor();
    QTextDocument* document = this->document();
    QTextCursor highlightCursor(document);

    //设置样式
    QTextCharFormat normalFormat;
    normalFormat.setForeground(Qt::black);
    normalFormat.setBackground(Qt::transparent);
    QTextCharFormat highlightFormat;
    highlightFormat.setForeground(Qt::red); // 设置字体颜色为红色
    highlightFormat.setBackground(Qt::green); // 设置背景色为绿色

    //取消原高亮
    for(int i=0;i<4;i++){
        if(highlightPos[i]!=-1){
            highlightCursor.setPosition(highlightPos[i]);
            highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            highlightCursor.mergeCharFormat(normalFormat);
        }
        highlightPos[i]=-1;
    }

    //判断括号并高亮
    int pos=cursor.position();
    QChar before=document->characterAt(pos-1);
    QChar after=document->characterAt(pos);
    if(before=='}'||before==')'){
        qDebug()<<"end";
        highlightCursor.setPosition(pos-1);
        highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        highlightCursor.mergeCharFormat(highlightFormat);
        highlightPos[0]=pos-1;
        //TODO:匹配
        highlightCursor.setPosition(0/*这里填匹配光标*/);
        highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        highlightCursor.mergeCharFormat(highlightFormat);
        highlightPos[1]=0/*这里填匹配光标*/;
    }
    if(after=='{'||after=='('){
        qDebug()<<"begin";
        highlightCursor.setPosition(pos);
        highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        highlightCursor.mergeCharFormat(highlightFormat);
        highlightPos[2]=pos;
        //TODO:匹配
        highlightCursor.setPosition(0/*这里填匹配光标*/);
        highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        highlightCursor.mergeCharFormat(highlightFormat);
        highlightPos[3]=0/*这里填匹配光标*/;
    }
}
