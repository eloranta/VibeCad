#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvas.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(8, 8, 8, 8);

    auto *toolbar = new QHBoxLayout;
    printButton = new QPushButton(tr("Print"), this);
    toolbar->addWidget(printButton);
    toolbar->addStretch();
    layout->addLayout(toolbar);

    canvas = new Canvas(this);
    layout->addWidget(canvas, 1);

    ui->centralwidget->setLayout(layout);

    connect(printButton, &QPushButton::clicked, this, &MainWindow::printCanvas);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::printCanvas()
{
    if (!canvas)
        return;

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    if (!painter.isActive())
        return;

    const QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);
    const QSize canvasSize = canvas->size();
    const qreal xScale = pageRect.width() / canvasSize.width();
    const qreal yScale = pageRect.height() / canvasSize.height();
    const qreal scale = std::min(xScale, yScale);

    painter.save();
    painter.translate(pageRect.x(), pageRect.y());
    painter.scale(scale, scale);
    canvas->render(&painter);
    painter.restore();
}
