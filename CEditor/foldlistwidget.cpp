#include "foldlistwidget.h"

FoldListWidget::FoldListWidget(QWidget* parent)
    : QListWidget(parent)
{
    connect(this, &FoldListWidget::itemClicked, this, &FoldListWidget::handleItemClicked);
}

void FoldListWidget::setRowVisible(int row, bool isVisible)
{
    if (row >= 0 && row < rowVisibility.size())
    {
        rowVisibility[row] = isVisible;
    }
}

void FoldListWidget::handleItemClicked(QListWidgetItem* item)
{
    FoldListWidgetItem* foldListWidgetItem = dynamic_cast<FoldListWidgetItem*>(item);
    //qDebug()<<"isCollapsed"<<foldListWidgetItem->isCollapsed();
    if (foldListWidgetItem)
    {
        foldListWidgetItem->setCollapsed(!foldListWidgetItem->isCollapsed());
        update(); // 发出重绘的指令
    }
}

void FoldListWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(viewport());
    //qDebug()<<rowVisibility.size();
    for (int i = 0; i < count(); ++i)
    {
        FoldListWidgetItem* item = dynamic_cast<FoldListWidgetItem*>(this->item(i));
        if (!item)
            continue;

        QRect rect = visualItemRect(item);

        // 根据type的值绘制不同的形状
        switch (item->getType())
        {
        case 1:
        {
            // 绘制正方形和加号/减号
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(200, 200, 200)); // 正方形的颜色
            painter.drawRect(rect.left(), rect.top(), rect.width(), rect.height());
            // 判断列表项的状态，如果是折叠的，则绘制减号；否则，绘制加号
            if (!item->isCollapsed())
            {
                // 绘制减号
                painter.setPen(QColor(0, 0, 0)); // 减号的颜色
                painter.drawLine(rect.left(), rect.center().y(), rect.right(), rect.center().y()); // 横线
            }
            else
            {
                // 绘制加号
                painter.setPen(QColor(0, 0, 0)); // 加号的颜色
                painter.drawLine(rect.left(), rect.center().y(), rect.right(), rect.center().y()); // 横线
                painter.drawLine(rect.center().x(), rect.top(), rect.center().x(), rect.bottom()); // 竖线
            }
            break;
        }


        case 2:
            {
            // 绘制居中的竖线
            painter.setPen(QColor(0, 0, 0)); // 线的颜色
            painter.drawLine(rect.center().x(), rect.top(), rect.center().x(), rect.bottom()); // 竖线
            break;
        }


        case 3:
            {
            // 绘制居中的竖线和右边一半的横线
            painter.setPen(QColor(0, 0, 0)); // 线的颜色
            painter.drawLine(rect.center().x(), rect.center().y(), rect.right(), rect.center().y()); // 横线
            painter.drawLine(rect.center().x(), rect.top(), rect.center().x(), rect.bottom()); // 竖线
            break;

        }

        case 4:
            {
            // 绘制上边一半的竖线和右边一半的横线
            painter.drawLine(rect.center().x(), rect.center().y(), rect.right(), rect.center().y()); // 横线
            painter.drawLine(rect.center().x(), rect.top(), rect.center().x(), rect.center().y()); // 竖线
            break;
        }


        default:
            // 处理未知类型
            break;
        }
    }



    QListWidget::paintEvent(event);
}
