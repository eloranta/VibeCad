#include "canvas.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setAutoFillBackground(true);
}

void Canvas::addRectangle(const QPoint &bottomLeft, const QPoint &topRight)
{
    QRect rect(QPoint(bottomLeft.x(), topRight.y()), QPoint(topRight.x(), bottomLeft.y()));
    worldRectangles.append(rect.normalized());
    update();
}

void Canvas::addCircle(const QPoint &center, int radius)
{
    worldCircles.append(qMakePair(center, radius));
    update();
}

bool Canvas::saveToFile(const QString &path) const
{
    QJsonObject root;
    QJsonArray rects;
    for (const QRect &r : worldRectangles)
    {
        QJsonObject rectObj;
        rectObj["bl_x"] = r.bottomLeft().x();
        rectObj["bl_y"] = r.bottomLeft().y();
        rectObj["tr_x"] = r.topRight().x();
        rectObj["tr_y"] = r.topRight().y();
        rects.append(rectObj);
    }
    root["rectangles"] = rects;

    QJsonArray circArr;
    for (const auto &c : worldCircles)
    {
        QJsonObject circObj;
        circObj["cx"] = c.first.x();
        circObj["cy"] = c.first.y();
        circObj["r"] = c.second;
        circArr.append(circObj);
    }
    root["circles"] = circArr;

    QJsonDocument doc(root);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(doc.toJson(QJsonDocument::Compact));
    return true;
}

bool Canvas::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.exists())
        return false;
    if (!file.open(QIODevice::ReadOnly))
        return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return false;

    worldRectangles.clear();
    worldCircles.clear();

    const QJsonObject root = doc.object();

    const QJsonArray rects = root.value("rectangles").toArray();
    for (const QJsonValue &v : rects)
    {
        const QJsonObject o = v.toObject();
        const int blx = o.value("bl_x").toInt();
        const int bly = o.value("bl_y").toInt();
        const int trx = o.value("tr_x").toInt();
        const int try_ = o.value("tr_y").toInt();
        QRect rect(QPoint(blx, bly), QPoint(trx, try_));
        worldRectangles.append(rect.normalized());
    }

    const QJsonArray circArr = root.value("circles").toArray();
    for (const QJsonValue &v : circArr)
    {
        const QJsonObject o = v.toObject();
        const int cx = o.value("cx").toInt();
        const int cy = o.value("cy").toInt();
        const int r = o.value("r").toInt();
        if (r > 0)
            worldCircles.append(qMakePair(QPoint(cx, cy), r));
    }

    update();
    return true;
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
    for (const auto &circle : worldCircles)
    {
        const QPoint c = toScreen(circle.first);
        painter.drawEllipse(c, circle.second, circle.second);
    }

    painter.setBrush(QColor(160, 200, 140, 180));
    for (const QRect &rect : worldRectangles)
    {
        painter.drawRect(QRect(toScreen(rect.bottomLeft()), toScreen(rect.topRight())).normalized());
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

QPoint Canvas::fromScreen(const QPoint &screen) const
{
    const QPoint o = origin();
    return QPoint(screen.x() - o.x(), o.y() - screen.y());
}
