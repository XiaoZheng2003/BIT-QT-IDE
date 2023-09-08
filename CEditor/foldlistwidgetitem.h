// foldlistwidgetitem.h
#ifndef FOLDLISTWIDGETITEM_H
#define FOLDLISTWIDGETITEM_H

#include "headers.h"

class FoldListWidgetItem : public QListWidgetItem
{
public:
    explicit FoldListWidgetItem(int itemType, QListWidget *parent = nullptr);
    int getType();
    bool isCollapsed();
    void setCollapsed(bool collapsed);
    int start = -1;
    int end = -1;
    int level = 0;

signals:
    void stateChanged(bool c);

private:
    bool collapsed = false;
    int type = 0;
};

#endif // FOLDLISTWIDGETITEM_H
