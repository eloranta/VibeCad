#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QString>

class Canvas;
class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void printCanvas();
    void addRectangle();
    void addCircle();
    void deleteSelected();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    Canvas *canvas = nullptr;
    QPushButton *printButton = nullptr;
    QPushButton *addRectButton = nullptr;
    QPushButton *addCircleButton = nullptr;
    QPushButton *deleteButton = nullptr;
    QString sceneFilePath;
};
#endif // MAINWINDOW_H
