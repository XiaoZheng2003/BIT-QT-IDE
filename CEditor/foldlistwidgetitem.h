// foldlistwidgetitem.h
#ifndef FOLDLISTWIDGETITEM_H
#define FOLDLISTWIDGETITEM_H

#include "headers.h"

class FoldListWidgetItem : public QListWidgetItem
{
public:
    explicit FoldListWidgetItem(QListWidget *parent = nullptr);

    void setFolded(bool folded);

protected:
//    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);
//    QSize sizeHint();

private:
    bool isFolded = false;
};

#endif // FOLDLISTWIDGETITEM_H
