#include "canvas.h"

#include <QPainter>
#include <QPaintEvent>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setAutoFillBackground(true);
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
}
