#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent):
    QPlainTextEdit(parent)
{
    connect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);

    // 创建一个定时器，用于延迟压入撤回栈
    m_timer=new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(500); // 0.5秒钟
    connect(m_timer,&QTimer::timeout,this,&CodeEditor::pushUndoStack);

    // 初始化栈
    connect(this,&CodeEditor::initText,[=](QString text){
        m_undoStack.push(text);
        this->setPlainText(text);
    });

    //判断是否文本发生变化
    connect(this, &CodeEditor::textChanged, this, &CodeEditor::handleTextChanged);
    //光标移动，高亮匹配括号
    connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::highlightMatchedBrackets);
    //行数变化时自动缩进
    connect(this,&QPlainTextEdit::blockCountChanged,this,&CodeEditor::autoIndent);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    //捕获撤销和重做快捷键
    if(event->matches(QKeySequence::Undo)){
        undo();
        event->accept();
    }
    else if(event->matches(QKeySequence::Redo)){
        redo();
        event->accept();
    }
    //处理需要括号补全的情况
    else if(bracketComplete(event)){}
    else if(event->key() == Qt::Key_Backspace){
        //一对字符整体删除的情况
        if(!m_cursorMoved && abs(document()->characterAt(textCursor().position()).toLatin1() - document()->characterAt(textCursor().position() - 1).toLatin1()) <= 2){
            textCursor().deleteChar();
            textCursor().deletePreviousChar();
        }
        else{
            QPlainTextEdit::keyPressEvent(event);
        }
    }
    else{
        QPlainTextEdit::keyPressEvent(event);
    }
    //检测自动补全括号后光标是否移动
    switch(document()->characterAt(textCursor().position()).toLatin1()){
    case '}':
    case ']':
    case ')':
    case '>':
    case '"':
    case '\'':
        break;
    default:
        //光标发生了移动
        m_cursorMoved = true;
    }
}

bool CodeEditor::bracketComplete(QKeyEvent *event)
{
    switch (event->key()) {
    //补全大括号
    case Qt::Key_BraceLeft:{
        m_cursorMoved = false;
        textCursor().insertText("{}");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return true;
    }
    //补全中括号
    case Qt::Key_BracketLeft:{
        m_cursorMoved = false;
        textCursor().insertText("[]");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return true;
    }
    //补全小括号
    case Qt::Key_ParenLeft:{
        m_cursorMoved = false;
        textCursor().insertText("()");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return true;
    }
    //补全尖括号
    case Qt::Key_Less:{
        if(textCursor().block().text().startsWith("#include")){
            m_cursorMoved = false;
            textCursor().insertText("<>");
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::Left);
            setTextCursor(cursor);
        }
        else{
            QPlainTextEdit::keyPressEvent(event);
        }
        return true;
    }
    //补全双引号
    case Qt::Key_QuoteDbl:{
        //处理需要略过的情况
        if(!m_cursorMoved){
            if(document()->characterAt(textCursor().position()) == '"'){
                QTextCursor cursor = textCursor();
                cursor.movePosition(QTextCursor::Right);
                setTextCursor(cursor);
                return true;
            }
        }
        m_cursorMoved = false;
        textCursor().insertText("\"\"");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return true;
    }
    //补全单引号
    case Qt::Key_Apostrophe:{
        //处理需要略过的情况
        if(!m_cursorMoved){
            if(document()->characterAt(textCursor().position()) == '\''){
                QTextCursor cursor = textCursor();
                cursor.movePosition(QTextCursor::Right);
                setTextCursor(cursor);
                return true;
            }
        }
        m_cursorMoved = false;
        textCursor().insertText("''");
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return true;
    }
    //输入右大括号
    case Qt::Key_BraceRight:{
        //处理需要略过的情况
        if(!m_cursorMoved){
            if(document()->characterAt(textCursor().position()) == '}'){
                QTextCursor cursor = textCursor();
                cursor.movePosition(QTextCursor::Right);
                setTextCursor(cursor);
                return true;
            }
        }
        if(document()->characterAt(textCursor().position() - 1) == '\t'){
            textCursor().deletePreviousChar();
        }
        return false;
    }
    //输入右中括号
    case Qt::Key_BracketRight:{
        //处理需要略过的情况
        if(!m_cursorMoved){
            if(document()->characterAt(textCursor().position()) == ']'){
                QTextCursor cursor = textCursor();
                cursor.movePosition(QTextCursor::Right);
                setTextCursor(cursor);
            }
        }
        else{
            QPlainTextEdit::keyPressEvent(event);
        }
        return true;
    }
    //输入右小括号
    case Qt::Key_ParenRight:{
        //处理需要略过的情况
        if(!m_cursorMoved){
            if(document()->characterAt(textCursor().position()) == ')'){
                QTextCursor cursor = textCursor();
                cursor.movePosition(QTextCursor::Right);
                setTextCursor(cursor);
            }
        }
        else{
            QPlainTextEdit::keyPressEvent(event);
        }
        return true;
    }
    //输入右尖括号
    case Qt::Key_Greater:{
        //处理需要略过的情况
        if(textCursor().block().text().startsWith("#include")){
            if(!m_cursorMoved){
                if(document()->characterAt(textCursor().position()) == '>'){
                    QTextCursor cursor = textCursor();
                    cursor.movePosition(QTextCursor::Right);
                    setTextCursor(cursor);
                }
            }
            else{
                QPlainTextEdit::keyPressEvent(event);
            }
        }
        else{
            QPlainTextEdit::keyPressEvent(event);
        }
        return true;
    }
    default:
        return false;
    }
}

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

void CodeEditor::undo()
{
    //撤销操作
    disconnect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
    if(m_undoStack.count()>1){
        QString currentText = this->toPlainText();
        m_redoStack.push(currentText); // 将当前文本压入重做栈
        m_undoStack.pop();
        QString undoText=m_undoStack.top();
        this->setPlainText(undoText);
        moveCursorToPostion(findFirstDifference(currentText,undoText));
    }
    connect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
}

void CodeEditor::redo()
{
    //恢复操作
    disconnect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
    if(!m_redoStack.isEmpty()){
        QString currentText = this->toPlainText();
        QString redoText = m_redoStack.pop();
        m_undoStack.push(redoText);
        this->setPlainText(redoText);
        moveCursorToPostion(findFirstDifference(currentText,redoText));
    }
    connect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
}

void CodeEditor::matchBrackets() {
    int current_length = 0;
    QList<Brackets> stack1, stack2; // 储存两种括号匹配情况的栈
    QTextDocument* document = this->document();
    int allrow = document->blockCount();
    bramap.clear();// 每一次匹配都清空之前的情况

    int inComment = 0; // 用于跟踪是否在注释内

    for (int i = 0; i < allrow; i++) { // 遍历每一行
        QTextBlock line = this->document()->findBlockByNumber(i);
        QString string = line.text(); // 获取每一行的文本
        if(inComment == 1 || inComment == 3 || inComment == 4){
            inComment = 0;
        }
        for (int j = 0; j < string.length(); j++) { // 遍历每一个字符
            QChar currentChar = string.at(j);

            if (!inComment) {
                switch (currentChar.unicode()) {
                case '{':
                    stack1.append(Brackets(current_length + j, -1, 1)); // 向栈内加入
                    break;
                case '(':
                    stack2.append(Brackets(current_length + j, -1, 2)); // 向栈内加入
                    break;
                case '}':
                    if (!stack1.isEmpty()) {
                        int end = current_length + j;
                        int start = stack1.last().currentPos;
                        bramap.insert(end, Brackets(end, start, -1)); // 加入两个括号匹配
                        bramap.insert(start, Brackets(start, end, 1));
                        stack1.removeLast(); // 移除栈的最后一个
                    }
                    else{
                        int end = current_length + j;
                        bramap.insert(end, Brackets(end, -1, -1)); // 无匹配的情况
                    }
                    break;
                case ')':
                    if (!stack2.isEmpty()) {
                        int end = current_length + j;
                        int start = stack2.last().currentPos;
                        bramap.insert(end, Brackets(end, start, -2)); // 加入两个括号匹配
                        bramap.insert(start, Brackets(start, end, 2));
                        stack2.removeLast();
                    }
                    else{
                        int end = current_length + j;
                        bramap.insert(end, Brackets(end, -1, -2));
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
                case '"':
                    if (j < string.length() - 1) {
                        inComment = 3;
                    }
                    break;
                case '\'':
                    if (j < string.length() - 1) {
                        inComment = 4;
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
                if(currentChar == '"'){
                    if (j < string.length() - 1 && inComment) {
                        if(inComment == 3){
                            inComment = 0;
                        }
                    }
                }
                if(currentChar == '\''){
                    if (j < string.length() - 1 && inComment) {
                        if(inComment == 4){
                            inComment = 0;
                        }
                    }
                }
            }
        }

        current_length += string.length() + 1; // 回车符
    }
    // 插入栈内没匹配上的
    while(!stack1.empty()){
        bramap.insert(stack1.last().currentPos,stack1.last());
        stack1.removeLast();
    }
    while(!stack2.empty()){
        bramap.insert(stack2.last().currentPos,stack2.last());
        stack2.removeLast();
    }
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
    setTabStopDistance(metrics.averageCharWidth()*4);
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
    //qDebug()<<maxItemSize;
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

void CodeEditor::handleTextChanged()
{
    QString currentText = this->toPlainText();

    // 比较当前文本内容与上一次保存的文本内容
    if (currentText != m_previousText) {
        // 只有文本内容发生了修改
        emit textRealChanged();
        // 文本发生改变，重新匹配括号
        matchBrackets();
        // 更新上一次保存的文本内容
        m_previousText = currentText;
    }
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
    highlightCursor.setPosition(0);
    highlightCursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
    highlightCursor.setCharFormat(normalFormat);

    //判断括号并高亮
    int pos=cursor.position();
    QChar before=document->characterAt(pos-1);
    QChar after=document->characterAt(pos);
    if(before=='}'||before==')'){
        int matchingPos = bramap.value(pos-1).correspondingPos;
        qDebug()<<1<<"end"<<"pos:"<<pos<<" matchingPos:"<<matchingPos;
        highlightCursor.setPosition(pos-1);
        highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        highlightCursor.setCharFormat(highlightFormat);
        //匹配
        if(matchingPos != -1){
            highlightCursor.setPosition(matchingPos);
            highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            highlightCursor.setCharFormat(highlightFormat);
        }
    }
    if(after=='{'||after=='('){
        int matchingPos = bramap.value(pos).correspondingPos;
        qDebug()<<2<<"begin"<<"pos:"<<pos<<" matchingPos:"<<matchingPos;
        highlightCursor.setPosition(pos);
        highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        highlightCursor.setCharFormat(highlightFormat);
        //匹配
        if(matchingPos != -1){
            highlightCursor.setPosition(matchingPos);
            highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            highlightCursor.setCharFormat(highlightFormat);
        }
    }
}

void CodeEditor::pushUndoStack()
{
    QString currentText = this->toPlainText();
    if (!m_undoStack.isEmpty() && currentText == m_undoStack.top()) {
        return; // 重复文本不入栈
    }

    m_undoStack.push(currentText);
    m_redoStack.clear(); // 清空重做栈
}

void CodeEditor::restartTimer()
{
    //重启计时器
    if(m_timer->isActive())
        m_timer->stop();
    m_timer->start();
}

int CodeEditor::findFirstDifference(const QString &str1, const QString &str2)
{
    //比较两个QString第一处不同的位置
    int minLength = qMin(str1.length(), str2.length());
    for (int i = 0; i < minLength; i++) {
        if (str1[i] != str2[i]) {
            return i;
        }
    }
    return minLength; // 如果前面的字符都相同，则返回较短字符串的长度
}

void CodeEditor::moveCursorToPostion(int pos)
{
    //移动光标到指定位置
    QTextCursor cursor=this->textCursor();
    cursor.setPosition(pos);
    this->setTextCursor(cursor);
}

void CodeEditor::autoIndent()
{
    QTextCursor currentCursor = this->textCursor();
    int pos = currentCursor.position(), level = 0;
    bool insertEmptyLine = false;
    if(document()->characterAt(pos - 2) == '{'){
        insertEmptyLine = true;
    }
    for(QMap<int,Brackets>::Iterator it = bramap.begin(); it != bramap.end(); it++){
        if(it.value().currentPos >= pos){
            break;
        }
        if(it.value().type == 1){
            level++;
        }
        else if(it.value().type == -1 && level > 0){
            level--;
        }
    }
    if(currentCursor.atBlockStart()){
        for(int i = 0; i < level; i++){
            currentCursor.insertText("\t");
        }
    }
    if(document()->characterAt(pos - 2) == '{'){
        if(document()->characterAt(currentCursor.position()) == '}'){
            QString text = "\n";
            for(int i = 0; i < level; i++){
                text += "\t";
            }
            currentCursor.insertText(text);
            currentCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
            currentCursor.deleteChar();
            currentCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, level);
            setTextCursor(currentCursor);
        }
    }
}
