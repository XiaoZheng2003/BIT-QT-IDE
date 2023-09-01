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

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    emit updateLineNumberArea(this->blockCount());
}
