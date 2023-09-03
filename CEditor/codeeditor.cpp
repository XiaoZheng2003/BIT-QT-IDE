#include "codeeditor.h"
#include <QDebug>

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

void CodeEditor::setLineNumberArea(QListWidget *lineNumberArea)
{
    m_lineNumberArea=lineNumberArea;
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    updateLineNumberArea();
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
    int fontSize=metrics.averageCharWidth();
    m_lineNumberArea->setMaximumSize(QSize((digit+1)*fontSize,this->height()));
    m_lineNumberArea->resize(QSize((digit+3)*6,this->height()));
    m_lineNumberArea->clear();
    for(int row=0;row<=blockCount();row++)
    {
        QListWidgetItem *item=new QListWidgetItem(QString::number(row+1),m_lineNumberArea);
        item->setFont(documentFont);
        item->setText(QString::number(row+1));
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
    QTimer::singleShot(1,this,&CodeEditor::sendCurrentScrollBarValue);
}

void CodeEditor::sendCurrentScrollBarValue()
{
    emit scrollBarValue(verticalScrollBar()->value());
}
