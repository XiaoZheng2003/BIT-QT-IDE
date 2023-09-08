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
    // 判断是否文本发生变化
    connect(this, &CodeEditor::textChanged, this, &CodeEditor::handleTextChanged);
    // 光标移动，高亮匹配括号
    connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::highlightMatchedBrackets);
    // 行数变化时自动缩进
    connect(this,&QPlainTextEdit::blockCountChanged,this,&CodeEditor::autoIndent);
    // TODO 括号匹配结束向其发送修改ui的信号
    connect(this,&CodeEditor::matchFinished,this,&CodeEditor::updateFoldListWidget);
    // 高亮当前行
    connect(this,&QPlainTextEdit::cursorPositionChanged,this,[=](){
        m_lineNumberArea->clearSelection();
        clearLineHighlight();
        QList<QListWidgetItem *> items = m_lineNumberArea->findItems(QString::number(textCursor().blockNumber()+1),Qt::MatchExactly);
        if(!items.empty()){
            items[0]->setSelected(true);
        }
        m_highlightLine.append(textCursor().blockNumber());
    });
    //同时选中多行
    connect(this,&QPlainTextEdit::selectionChanged,this,[=](){
        int startPos = textCursor().selectionStart();
        int endPos = textCursor().selectionEnd();
        int startRow = document()->findBlock(startPos).blockNumber();
        int endRow = document()->findBlock(endPos).blockNumber();
        for(int i = startRow; i <= endRow; i++){
            QList<QListWidgetItem *> items = m_lineNumberArea->findItems(QString::number(i+1),Qt::MatchExactly);
            if(!items.empty()){
                items[0]->setSelected(true);
            }
        }
    });
}

void CodeEditor::clearLineHighlight()
{
    //取消之前高亮的行
    QTextBlockFormat format = QTextBlockFormat();
    format.setBackground(QBrush(Qt::white));
    for(int i = 0; i < m_highlightLine.count(); i++){
        QTextCursor cursor = QTextCursor(document()->findBlockByNumber(m_highlightLine[i]));
        cursor.setBlockFormat(format);
    }
    m_highlightLine.clear();
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    //捕获快捷键
    if(completer&&completer->popup()->isVisible()){
        //使快捷键优先作用于widget
        switch(event->key()){
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            event->ignore();
            return;
        default:
            break;
        }
    }
    if(event->matches(QKeySequence::Undo)){
        undo();
        event->accept();
    }
    else if(event->matches(QKeySequence::Redo)){
        redo();
        event->accept();
    }
    //处理需要括号补全的情况
    else if(bracketComplete(event));
    else if(event->key() == Qt::Key_Backspace&&
             (!m_cursorMoved && abs(document()->characterAt(textCursor().position()).toLatin1() - document()->characterAt(textCursor().position() - 1).toLatin1()) <= 2)){
        //一对字符整体删除的情况
        textCursor().deleteChar();
        textCursor().deletePreviousChar();
    }
    else if(event->key() == Qt::Key_Colon){
        if(textCursor().block().text().contains("case")){
            QTextCursor cursor=textCursor();
            cursor.movePosition(QTextCursor::StartOfBlock);
            if(document()->characterAt(cursor.position())=='\t'){
                cursor.deleteChar();
            }
        }
        QPlainTextEdit::keyPressEvent(event);
    }
    else{
        // 若completer不存在，或按下键非Ctrl+E
        bool isShortCut = event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_E;
        if (!completer || !isShortCut)
            QPlainTextEdit::keyPressEvent(event);

        // 判断是否按下了Ctrl键或Shift键
        bool ctrlOrShift = event->modifiers() == Qt::ControlModifier || event->modifiers() == Qt::ShiftModifier;
        if (ctrlOrShift && event->text().isEmpty())
            return;

        // 判断是否按下修饰键（除了Ctrl和Shift键）
        bool hasModifier = event->modifiers() != Qt::NoModifier && !ctrlOrShift;

        /* 如果不是快捷键，并且满足以下条件之一，则不显示自动完成窗口：
        ** 1. 按下修饰键
        ** 2. 按下的文本为空
        ** 3. 光标下的文本长度小于2
        ** 4. 按下的文本的最后一个字符在结束词中
        */
        // 结束词
        QString endOfWord("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");

        if (!isShortCut && (hasModifier || event->text().isEmpty() ||
                            textUnderCursor().length() < 2 || endOfWord.contains(event->text().right(1)))){
            completer->popup()->hide();
            return;
        }

        autoComplete();
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

void CodeEditor::autoComplete()
{
    // 自动补全
    // 获取光标下的文本作为自动完成的前缀
    QString completionPrefix = this->textUnderCursor();

    // 如果completionPrefix与当前自动完成的前缀不相等，则更新自动完成的前缀，并将弹出窗口的当前索引设置为第一个选项
    if (completionPrefix != completer->completionPrefix())
    {
        completer->setCompletionPrefix(completionPrefix);
        completer->popup()->setCurrentIndex(completer->completionModel()->index(0, 0));
    }

    // 获取光标位置的矩形区域，并设置弹出窗口的宽度以适应内容和垂直滚动条的宽度
    QRect rect = this->cursorRect();
    rect.setWidth(completer->popup()->sizeHintForColumn(0) +
                  completer->popup()->verticalScrollBar()->sizeHint().width());

    // 在光标位置弹出自动完成的弹出窗口
    completer->complete(rect);
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

void CodeEditor::setFoldListWidget(FoldListWidget *foldListWidget)
{
    m_foldListWidget=foldListWidget;
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
        updateFoldListWidget();
    }
    else
    {
        QPlainTextEdit::wheelEvent(event);
    }
}

void CodeEditor::focusInEvent(QFocusEvent *event)
{
    //设置窗口焦点
    if(completer) completer->setWidget(this);
    QPlainTextEdit::focusInEvent(event);
}

void CodeEditor::undo()
{
    //撤销操作
    //防止撤销带来的更新重新压入撤销栈
    disconnect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
    //最少保留初始状态
    if(m_undoStack.count()>1){
        QString currentText = this->toPlainText();
        m_redoStack.push(currentText); // 将当前文本压入重做栈
        m_undoStack.pop();
        QString undoText=m_undoStack.top();
        this->setPlainText(undoText);
        moveCursorToPostion(findFirstDifference(currentText,undoText));
        this->centerCursor();
    }
    //重新连接信号和槽
    connect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
}

void CodeEditor::redo()
{
    //恢复操作
    //防止恢复带来的更新重新压入撤销栈
    disconnect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
    if(!m_redoStack.isEmpty()){
        QString currentText = this->toPlainText();
        QString redoText = m_redoStack.pop();
        m_undoStack.push(redoText);
        this->setPlainText(redoText);
        moveCursorToPostion(findFirstDifference(currentText,redoText));
        this->centerCursor();
    }
    //重新连接信号和槽
    connect(this,&CodeEditor::textRealChanged,this,&CodeEditor::restartTimer);
}

void CodeEditor::matchBrackets() {
    int current_length = 0;
    QList<Brackets> stack1, stack2, stack3; // 储存两种括号匹配情况的栈
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
                    stack1.append(Brackets(current_length + j, -1, 1, i)); // 向栈内加入
                    break;
                case '(':
                    stack2.append(Brackets(current_length + j, -1, 2, i)); // 向栈内加入
                    break;
                case '[':
                    stack3.append(Brackets(current_length + j, -1, 3, i)); // 向栈内加入
                    break;
                case '}':
                    if (!stack1.isEmpty()) {
                        int end = current_length + j;
                        int start = stack1.last().currentPos;
                        bramap.insert(end, Brackets(end, start, -1, stack1.last().row)); // 加入两个括号匹配
                        bramap.insert(start, Brackets(start, end, 1, i));
                        stack1.removeLast(); // 移除栈的最后一个
                    }
                    else{
                        int end = current_length + j;
                        bramap.insert(end, Brackets(end, -1, -1, -1)); // 无匹配的情况
                    }
                    break;
                case ')':
                    if (!stack2.isEmpty()) {
                        int end = current_length + j;
                        int start = stack2.last().currentPos;
                        bramap.insert(end, Brackets(end, start, -2, stack2.last().row)); // 加入两个括号匹配
                        bramap.insert(start, Brackets(start, end, 2, i));
                        stack2.removeLast();
                    }
                    else{
                        int end = current_length + j;
                        bramap.insert(end, Brackets(end, -1, -2, -1));
                    }
                    break;
                case ']':
                    if (!stack3.isEmpty()) {
                        int end = current_length + j;
                        int start = stack3.last().currentPos;
                        bramap.insert(end, Brackets(end, start, -3, stack3.last().row)); // 加入两个括号匹配
                        bramap.insert(start, Brackets(start, end, 3, i));
                        stack3.removeLast();
                    }
                    else{
                        int end = current_length + j;
                        bramap.insert(end, Brackets(end, -1, -3, -1));
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
    while(!stack3.empty()){
        bramap.insert(stack3.last().currentPos,stack3.last());
        stack3.removeLast();
    }
    emit matchFinished();
}

void CodeEditor::updateLineNumberArea()
{
    //根据最大行数的位数调整大小
    QFont documentFont=this->font();
    QFontMetrics metrics(documentFont);
    setTabStopDistance(metrics.averageCharWidth()*4);
    int maxItemSize=0;

    m_lineNumberArea->clear();
    for(int row=0;row<=blockCount();row++)
    {
        if(this->document()->findBlockByNumber(row).isVisible()){
            QListWidgetItem *item=new QListWidgetItem(QString::number(row+1),m_lineNumberArea);
            item->setFont(documentFont);
            item->setText(QString::number(row+1));
            int itemSize =metrics.size(Qt::TextSingleLine,QString::number(row+1),0).width();
            if(itemSize>maxItemSize)
                maxItemSize=itemSize;
            item->setSizeHint(QSize(this->width(),getLineHeight(row)));
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
    }
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

void CodeEditor::updateFoldListWidget()
{
    QFont documentFont=this->font();
    QFontMetrics metrics(documentFont);
    int current_length = 0;
    QTextDocument* document = this->document();
    int totalRow = document->blockCount();
    m_foldListWidget->clear();
    clearAllLines();
    QVector<QVector<int>> rowType(totalRow+1,QVector<int>(3)); //0:type;1:end;3:level
    for(int i=0;i<totalRow;i++)
    {
        QTextBlock line = this->document()->findBlockByNumber(i);
        QString string = line.text(); // 获取每一行的文本
        for (int j = 0; j < string.length(); j++) { // 遍历每一个字符
            QChar currentChar = string.at(j);
            if(currentChar == '{'){
                if(bramap.value(current_length + j).row!=i&&bramap.value(current_length + j).row!=-1){
                    rowType[i][0]=1;
                    rowType[i][1]=bramap.value(current_length + j).row;
                    rowType[bramap.value(current_length + j).row][0]=3;
                    int pos = bramap.value(current_length + j).correspondingPos, level = 0;
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
                    if(level == 1){
                        rowType[bramap.value(current_length + j).row][0]=4;
                        rowType[i][2]=1;
                    }
                    //TODO设置flag减少循环
                    for (int k=i+1; k<bramap.value(current_length + j).row; k++){
                        rowType[k][0] = 2;
                    }
                    break;
                }
            }
        }
        current_length += string.length() + 1;
    }
    int typeFlag = 0;
    for (int k=0; k<totalRow; k++){
        if(typeFlag != 1 && rowType[k][0] == 3){
            typeFlag = 1;
        }
        if(typeFlag == 1 && rowType[k][0] == 0){
            rowType[k-1][0] = 4;
            typeFlag = 0;
        }
    }

    //根据最大行数的位数调整大小
    for(int row=0;row<totalRow;row++)
    {
        if(this->document()->findBlockByNumber(row).isVisible()){
            FoldListWidgetItem *item=new FoldListWidgetItem(rowType[row][0]);
            if(rowType[row][0]==1){
                if(rowType[row][2]==1){
                    item->level=1;
                }
                item->start = row + 1;
                item->end = rowType[row][1];
                QTextBlock qtb = this->document()->findBlockByNumber(row+1);
                item->setCollapsed(!qtb.isVisible());
                if(!qtb.isVisible()){
                  setLine(row, !qtb.isVisible());
                }
            }
            item->setSizeHint(QSize(metrics.averageCharWidth(),getLineHeight(row)));
            m_foldListWidget->addItem(item);
        }

    }
    m_foldListWidget->setMaximumWidth(m_foldListWidget->item(0)->sizeHint().height());
    QTimer::singleShot(1,this,&CodeEditor::sendCurrentScrollBarValue);
}

int CodeEditor::getLineHeight(int lineNumber)
{
    int newLineHeight;
    newLineHeight = qRound(blockBoundingGeometry(this->document()->findBlockByNumber(lineNumber)).height());
    if(newLineHeight==0){
       newLineHeight = lineHeight;
    }
    else{
       lineHeight = newLineHeight;
    }
    return newLineHeight;
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
    if(before=='}'||before==')'||before==']'){
        int matchingPos = bramap.value(pos-1).correspondingPos;
        //匹配
        if(matchingPos != -1){
            highlightCursor.setPosition(pos-1);
            highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            highlightCursor.setCharFormat(highlightFormat);

            highlightCursor.setPosition(matchingPos);
            highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            highlightCursor.setCharFormat(highlightFormat);
        }
    }
    if(after=='{'||after=='('||after=='['){
        int matchingPos = bramap.value(pos).correspondingPos;
        //匹配
        if(matchingPos != -1){
            highlightCursor.setPosition(pos);
            highlightCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            highlightCursor.setCharFormat(highlightFormat);

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

void CodeEditor::setCompleter(QCompleter *c)
{
    //设置自动补全
    if(completer) completer->disconnect(this);
    completer=c;
    if(!c) return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseSensitive);
    connect(c,QOverload<const QString &>::of(&QCompleter::activated),this,&CodeEditor::insertCompletion);
}

void CodeEditor::insertCompletion(const QString &completion)
{
    //将选中的选项的文本插入
    if (completer->widget() != this)
        return;
    QTextCursor cursor = textCursor();
    int extra = completion.length() - completer->completionPrefix().length();
    cursor.movePosition(QTextCursor::Left);
    cursor.movePosition(QTextCursor::EndOfWord);
    cursor.insertText(completion.right(extra));
    setTextCursor(cursor);
}

QString CodeEditor::textUnderCursor()
{
    //获取当前光标下词语
    QTextCursor cursor=textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}

void CodeEditor::commentSelectedLines()
{
    QTextCursor cursor = this->textCursor();
    int selectionStart = cursor.selectionStart();
    int selectionEnd = cursor.selectionEnd();

    // 获取选中行的行号
    int startLine = this->document()->findBlock(selectionStart).blockNumber();
    int endLine = this->document()->findBlock(selectionEnd).blockNumber();

    // 切换选中的行的注释状态
    for (int line = startLine; line <= endLine; line++) {
        QTextBlock block = this->document()->findBlockByNumber(line);
        QTextCursor blockCursor(block);
        blockCursor.select(QTextCursor::LineUnderCursor);

        QString lineText = blockCursor.selectedText();

        // 切换注释状态
        QString commentedLine="//"+lineText;
        if(lineText.trimmed().startsWith("//")){
            int index=lineText.indexOf("//");
            if(~index)
                commentedLine=lineText.replace(index,2,"");
        }
        blockCursor.setPosition(block.position());
        blockCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        blockCursor.removeSelectedText();
        blockCursor.insertText(commentedLine);
    }
}

void CodeEditor::autoIndent()
{
    QTextCursor currentCursor = this->textCursor();
    int pos = currentCursor.position(), level = 0;
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

void CodeEditor::setLine(int lineNumber, bool draw)
{
    // 先检查是否已经存在该行的记录，如果存在则更新状态，否则添加新记录
    for (int i = 0; i < linesToDraw.size(); ++i)
    {
        if (linesToDraw[i].lineNumber == lineNumber)
        {
            linesToDraw[i].draw = draw;
            update();
            return;
        }
    }

    LineToDraw lineToDraw(lineNumber, draw);
    linesToDraw.append(lineToDraw);
    update();
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    QTextBlockFormat format = QTextBlockFormat();
    format.setBackground(QBrush(QColor(Qt::cyan).lighter(180)));
    for(int i = 0; i < m_highlightLine.count(); i++){
        QTextCursor cursor = QTextCursor(document()->findBlockByNumber(m_highlightLine[i]));
        cursor.setBlockFormat(format);
    }

    QPlainTextEdit::paintEvent(event);

    QPainter painter(viewport());

    for (const LineToDraw &lineToDraw : linesToDraw)
    {
        if (lineToDraw.draw)
        {
            QTextBlock block = document()->findBlockByNumber(lineToDraw.lineNumber);
            if (!block.isValid())
                continue;

            painter.setPen(QPen(Qt::black, 2)); // 设置线的颜色和宽度
            int y = blockBoundingGeometry(block).bottom(); // 获取指定行底部的Y坐标
            painter.drawLine(0, y+1.5, viewport()->width(), y+1.5); // 绘制横线
        }
    }
}

void CodeEditor::clearAllLines()
{
    linesToDraw.clear();
    update();
}
