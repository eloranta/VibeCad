#include "canvas.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Canvas::Canvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(210, 297);
    resize(210, 297);
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

void Canvas::deleteSelected()
{
    if (selectedRect >= 0 && selectedRect < worldRectangles.size())
        worldRectangles.removeAt(selectedRect);
    else if (selectedCircle >= 0 && selectedCircle < worldCircles.size())
        worldCircles.removeAt(selectedCircle);

    selectedRect = -1;
    selectedCircle = -1;
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

    // Highlight A4 area (210x297) in world coords
    painter.setPen(QPen(QColor(120, 120, 120), 1, Qt::DashLine));
    painter.setBrush(QColor(230, 230, 230, 80));
    QRect a4Rect(QPoint(0, 0), QSize(210, 297));
    QRect screenA4 = QRect(toScreen(a4Rect.bottomLeft()), toScreen(a4Rect.topRight())).normalized();
    painter.drawRect(screenA4);

    painter.setPen(Qt::black);
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

    // Highlight selection
    painter.setPen(QPen(QColor(220, 80, 80), 2, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    if (selectedRect >= 0 && selectedRect < worldRectangles.size())
    {
        const QRect r = worldRectangles.at(selectedRect);
        painter.drawRect(QRect(toScreen(r.bottomLeft()), toScreen(r.topRight())).normalized());
    }
    if (selectedCircle >= 0 && selectedCircle < worldCircles.size())
    {
        const auto &c = worldCircles.at(selectedCircle);
        painter.drawEllipse(toScreen(c.first), c.second, c.second);
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

void Canvas::mousePressEvent(QMouseEvent *event)
{
    const QPoint worldPos = fromScreen(event->pos());

    // Check rectangles (topmost last)
    int foundRect = -1;
    for (int i = worldRectangles.size() - 1; i >= 0; --i)
    {
        if (worldRectangles.at(i).contains(worldPos))
        {
            foundRect = i;
            break;
        }
    }

    // Check circles
    int foundCircle = -1;
    for (int i = worldCircles.size() - 1; i >= 0; --i)
    {
        const auto &c = worldCircles.at(i);
        const QPoint delta = worldPos - c.first;
        if (QPointF(delta).manhattanLength() <= c.second ||
            (delta.x() * delta.x() + delta.y() * delta.y()) <= c.second * c.second)
        {
            foundCircle = i;
            break;
        }
    }

    selectedRect = foundRect;
    selectedCircle = foundCircle;
    update();

    QWidget::mousePressEvent(event);
}
