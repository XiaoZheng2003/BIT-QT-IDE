#ifndef FOLDLISTWIDGET_H
#define FOLDLISTWIDGET_H

#include "headers.h"

class FoldListWidget : public QListWidget
{
    Q_OBJECT

public:
    FoldListWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // FOLDLISTWIDGET_H