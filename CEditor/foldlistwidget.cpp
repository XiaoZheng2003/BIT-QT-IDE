#include "foldlistwidget.h"
#include <QPainter>

FoldListWidget::FoldListWidget(QWidget* parent)
    : QListWidget(parent)
{
}

void FoldListWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(viewport());

    for (int i = 0; i < count(); ++i)
    {
        QListWidgetItem* item = this->item(i);
        QRect rect = visualItemRect(item);

        // 绘制包裹的正方形
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(200, 200, 200)); // 正方形的颜色
        int squareSize = 20; // 正方形的大小
        int x = rect.left() + 10; // 调整位置
        int y = rect.center().y() - squareSize / 2;
        painter.drawRect(x, y, squareSize, squareSize);

        // 判断列表项的状态，如果是折叠的，则绘制减号；否则，绘制加号
        if (item->isSelected())
        {
            // 绘制减号
            painter.setPen(QColor(0, 0, 0)); // 减号的颜色
            int lineLength = 15; // 减号的线长度
            int halfLineLength = lineLength / 2;
            int centerX = x + squareSize / 2;
            int centerY = y + squareSize / 2;
            painter.drawLine(centerX - halfLineLength, centerY, centerX + halfLineLength, centerY); // 横线
        }
        else
        {
            // 绘制加号
            painter.setPen(QColor(0, 0, 0)); // 加号的颜色
            int lineLength = 15; // 加号的线长度
            int halfLineLength = lineLength / 2;
            int centerX = x + squareSize / 2;
            int centerY = y + squareSize / 2;
            painter.drawLine(centerX - halfLineLength, centerY, centerX + halfLineLength, centerY); // 横线
            painter.drawLine(centerX, centerY - halfLineLength, centerX, centerY + halfLineLength); // 竖线
        }
    }

    QListWidget::paintEvent(event);
}
