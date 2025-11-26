#include "canvas.h"

#include <QPainter>
#include <QPaintEvent>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setAutoFillBackground(true);
}

void Canvas::addRectangle(const QPoint &oppositeCorner)
{
    QRect rect(anchorPoint, oppositeCorner);
    rect = rect.normalized();
    rectangles.append(rect);
    update();
}

void Canvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), QColor(245, 245, 245));

    painter.setPen(Qt::black);
    painter.setBrush(QColor(180, 220, 250));
    painter.drawRect(50, 50, 150, 100);

    painter.setBrush(QColor(200, 180, 230));
    painter.drawRect(250, 120, 200, 120);

    painter.setBrush(QColor(255, 200, 120));
    painter.drawEllipse(QPoint(180, 220), 60, 60);

    painter.setBrush(QColor(160, 200, 140, 180));
    for (const QRect &rect : rectangles)
    {
        painter.drawRect(rect);
    }

    // Draw simple X/Y axes from the anchor point
    const int margin = 20;
    const QPoint origin = anchorPoint;
    const QPoint xEnd(width() - margin, origin.y());
    const QPoint yEnd(origin.x(), height() - margin);

    painter.setPen(QPen(Qt::darkGray, 2));
    painter.drawLine(origin, xEnd);
    painter.drawLine(origin, yEnd);

    // Tick marks every 100 px
    const int tickLen = 6;
    for (int x = origin.x() + 100; x <= xEnd.x(); x += 100)
    {
        painter.drawLine(QPoint(x, origin.y() - tickLen), QPoint(x, origin.y() + tickLen));
        painter.drawText(QPoint(x - 10, origin.y() - 8), QString::number(x - origin.x()));
    }
    for (int y = origin.y() + 100; y <= yEnd.y(); y += 100)
    {
        painter.drawLine(QPoint(origin.x() - tickLen, y), QPoint(origin.x() + tickLen, y));
        painter.drawText(QPoint(origin.x() + 8, y + 4), QString::number(y - origin.y()));
    }

    // Arrow heads
    painter.drawLine(xEnd, xEnd + QPoint(-8, -5));
    painter.drawLine(xEnd, xEnd + QPoint(-8, 5));
    painter.drawLine(yEnd, yEnd + QPoint(-5, -8));
    painter.drawLine(yEnd, yEnd + QPoint(5, -8));

    painter.setPen(Qt::darkGray);
    painter.drawText(xEnd + QPoint(-15, -8), "X");
    painter.drawText(yEnd + QPoint(8, -12), "Y");
}
