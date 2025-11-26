#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QVector>
#include <QRect>

class Canvas : public QWidget
{
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);
    void addRectangle(const QPoint &bottomLeft, const QPoint &topRight);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QRect> rectangles;
    QPoint originPoint = QPoint(40, 40);
};

#endif // CANVAS_H
