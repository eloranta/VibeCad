#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvas.h"

#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(8, 8, 8, 8);
    layout->addWidget(new Canvas(this));
    ui->centralwidget->setLayout(layout);
}

MainWindow::~MainWindow()
{
    delete ui;
}
