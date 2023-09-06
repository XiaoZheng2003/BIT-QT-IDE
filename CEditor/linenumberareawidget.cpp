#include "linenumberareawidget.h"


LineNumberAreaWidget::LineNumberAreaWidget(QWidget* parent)
    : QListWidget(parent)
{
    int rowCount = this->count();
    for (int i = 0; i < rowCount; ++i) {
        rowVisibility.append(true);
    }
}
void LineNumberAreaWidget::setRowVisible(int row, bool isVisible)
{
    if (row >= 0 && row < rowVisibility.size())
    {
        rowVisibility[row] = isVisible;
    }
}
