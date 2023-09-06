#ifndef LINENUMBERAREAWIDGET_H
#define LINENUMBERAREAWIDGET_H

#include <QWidget>
#include "headers.h"

class LineNumberAreaWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit LineNumberAreaWidget(QWidget *parent = nullptr);
    void setRowVisible(int row, bool isVisible);

private:
    QList<bool> rowVisibility; // 存储每一行的可见性状态
};

#endif // LINENUMBERAREAWIDGET_H
