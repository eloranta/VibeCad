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
    void addCircle(const QPoint &center, int radius);
    void deleteSelected();
    bool saveToFile(const QString &path) const;
    bool loadFromFile(const QString &path);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QVector<QRect> worldRectangles;
    QVector<QPair<QPoint, int>> worldCircles;
    QPoint origin() const;
    QPoint toScreen(const QPoint &world) const;
    QPoint fromScreen(const QPoint &screen) const;

    int selectedRect = -1;
    int selectedCircle = -1;
};

#endif // CANVAS_H
