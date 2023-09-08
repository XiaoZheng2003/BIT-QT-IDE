// foldlistwidgetitem.cpp
#include "foldlistwidgetitem.h"

FoldListWidgetItem::FoldListWidgetItem(int itemType, QListWidget *parent)
    : QListWidgetItem(parent)
{
    // 默认情况下未折叠
    collapsed = false;
    type = itemType;
}

int FoldListWidgetItem::getType(){
    return type;
}

bool FoldListWidgetItem::isCollapsed(){
    return collapsed;
}

void FoldListWidgetItem::setCollapsed(bool c){
    collapsed=c;
}
