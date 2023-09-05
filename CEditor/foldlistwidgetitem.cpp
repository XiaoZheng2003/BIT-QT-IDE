// foldlistwidgetitem.cpp
#include "foldlistwidgetitem.h"

FoldListWidgetItem::FoldListWidgetItem(QListWidget *parent)
    : QListWidgetItem(parent)
{
    // 默认情况下未折叠
    isFolded = false;
}
