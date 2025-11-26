#include "canvas.h"

#include <QPainter>
#include <QPaintEvent>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setAutoFillBackground(true);
}

void Canvas::addRectangle(const QPoint &bottomLeft, const QPoint &topRight)
{
    const QPoint blScreen = toScreen(bottomLeft);
    const QPoint trScreen = toScreen(topRight);
    QRect rect(blScreen, trScreen);
    rectangles.append(rect.normalized());
    update();
}

void Canvas::addCircle(const QPoint &center, int radius)
{
    const QPoint c = toScreen(center);
    circles.append(qMakePair(c, radius));
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

    painter.setBrush(QColor(120, 180, 220, 160));
    for (const auto &circle : circles)
    {
        painter.drawEllipse(circle.first, circle.second, circle.second);
    }

    painter.setBrush(QColor(160, 200, 140, 180));
    for (const QRect &rect : rectangles)
    {
        painter.drawRect(rect);
    }

    // Draw simple X/Y axes from the anchor point
    const int margin = 20;
    const QPoint o = origin();
    const QPoint xEnd(width() - margin, o.y());
    const QPoint yEnd(o.x(), margin);

    painter.setPen(QPen(Qt::darkGray, 2));
    painter.drawLine(o, xEnd);
    painter.drawLine(o, yEnd);

    // Tick marks every 100 px
    const int tickLen = 6;
    for (int x = o.x() + 100; x <= xEnd.x(); x += 100)
    {
        painter.drawLine(QPoint(x, o.y() - tickLen), QPoint(x, o.y() + tickLen));
        painter.drawText(QPoint(x - 12, o.y() - 8), QString::number(x - o.x()));
    }
    for (int y = o.y() - 100; y >= yEnd.y(); y -= 100)
    {
        painter.drawLine(QPoint(o.x() - tickLen, y), QPoint(o.x() + tickLen, y));
        painter.drawText(QPoint(o.x() + 8, y + 4), QString::number(o.y() - y));
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

QPoint Canvas::origin() const
{
    return QPoint(40, height() - 40);
}

QPoint Canvas::toScreen(const QPoint &world) const
{
    // World coordinates: x to the right, y upwards from origin
    const QPoint o = origin();
    return QPoint(o.x() + world.x(), o.y() - world.y());
}
