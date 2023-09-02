#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent):
    QPlainTextEdit(parent)
{
}

//void CodeEditor::keyPressEvent(QKeyEvent *event)
//{
//    QPlainTextEdit::keyPressEvent(event);

//    switch(event->key())
//    {
//    case Qt::Key_Enter:
//        //emit lineNumberChange();
//        break;
//    }
//}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    emit updateLineNumberArea(this->blockCount());
}
