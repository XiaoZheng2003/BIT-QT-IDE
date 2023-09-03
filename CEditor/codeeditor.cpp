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
