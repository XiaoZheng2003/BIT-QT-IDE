#ifndef FOLDLISTWIDGET_H
#define FOLDLISTWIDGET_H

#include "headers.h"
#include "foldlistwidgetitem.h"

class FoldListWidget : public QListWidget
{
    Q_OBJECT

public:
    FoldListWidget(QWidget* parent = nullptr);
    void setRowVisible(int row, bool isVisible);
    QList<bool> rowVisibility; // 存储每一行的可见性状态

private slots:
    void handleItemClicked(QListWidgetItem* item);

protected:
    void paintEvent(QPaintEvent* event) override;

private:

};

#endif // FOLDLISTWIDGET_H
