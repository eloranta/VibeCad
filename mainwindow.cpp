#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvas.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto *layout = new QVBoxLayout;
    layout->setContentsMargins(8, 8, 8, 8);

    canvas = new Canvas(this);
    layout->addWidget(canvas, 1);

    auto *toolbar = new QHBoxLayout;
    addRectButton = new QPushButton(tr("Add Rectangle"), this);
    printButton = new QPushButton(tr("Print"), this);
    toolbar->addWidget(addRectButton);
    toolbar->addStretch();
    toolbar->addWidget(printButton);
    layout->addLayout(toolbar);

    ui->centralwidget->setLayout(layout);

    connect(addRectButton, &QPushButton::clicked, this, &MainWindow::addRectangle);
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

void MainWindow::addRectangle()
{
    if (!canvas)
        return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Add Rectangle"));

    auto *form = new QFormLayout(&dialog);
    auto *xEdit = new QLineEdit(&dialog);
    auto *yEdit = new QLineEdit(&dialog);
    xEdit->setPlaceholderText(tr("e.g. 200"));
    yEdit->setPlaceholderText(tr("e.g. 150"));
    form->addRow(tr("Opposite corner X:"), xEdit);
    form->addRow(tr("Opposite corner Y:"), yEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    bool okX = false;
    bool okY = false;
    const int x = xEdit->text().toInt(&okX);
    const int y = yEdit->text().toInt(&okY);
    if (!okX || !okY)
        return;

    canvas->addRectangle(QPoint(x, y));
}
