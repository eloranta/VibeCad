#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvas.h"

#include <QCoreApplication>
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
#include <QCloseEvent>
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
    addCircleButton = new QPushButton(tr("Add Circle"), this);
    deleteButton = new QPushButton(tr("Delete Selected"), this);
    printButton = new QPushButton(tr("Print"), this);
    toolbar->addWidget(addRectButton);
    toolbar->addWidget(addCircleButton);
    toolbar->addWidget(deleteButton);
    toolbar->addStretch();
    toolbar->addWidget(printButton);
    layout->addLayout(toolbar);

    ui->centralwidget->setLayout(layout);

    connect(addRectButton, &QPushButton::clicked, this, &MainWindow::addRectangle);
    connect(addCircleButton, &QPushButton::clicked, this, &MainWindow::addCircle);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelected);
    connect(printButton, &QPushButton::clicked, this, &MainWindow::printCanvas);

    sceneFilePath = QCoreApplication::applicationDirPath() + "/scene.json";
    canvas->loadFromFile(sceneFilePath);
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
    auto *blX = new QLineEdit(&dialog);
    auto *blY = new QLineEdit(&dialog);
    auto *trX = new QLineEdit(&dialog);
    auto *trY = new QLineEdit(&dialog);
    blX->setPlaceholderText(tr("e.g. 60"));
    blY->setPlaceholderText(tr("e.g. 180"));
    trX->setPlaceholderText(tr("e.g. 220"));
    trY->setPlaceholderText(tr("e.g. 40"));
    form->addRow(tr("Bottom-left X:"), blX);
    form->addRow(tr("Bottom-left Y:"), blY);
    form->addRow(tr("Top-right X:"), trX);
    form->addRow(tr("Top-right Y:"), trY);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    bool okBlX = false, okBlY = false, okTrX = false, okTrY = false;
    const int blx = blX->text().toInt(&okBlX);
    const int bly = blY->text().toInt(&okBlY);
    const int trx = trX->text().toInt(&okTrX);
    const int try_ = trY->text().toInt(&okTrY);
    if (!okBlX || !okBlY || !okTrX || !okTrY)
        return;

    canvas->addRectangle(QPoint(blx, bly), QPoint(trx, try_));
}

void MainWindow::addCircle()
{
    if (!canvas)
        return;

    QDialog dialog(this);
    dialog.setWindowTitle(tr("Add Circle"));

    auto *form = new QFormLayout(&dialog);
    auto *cx = new QLineEdit(&dialog);
    auto *cy = new QLineEdit(&dialog);
    auto *r = new QLineEdit(&dialog);
    cx->setPlaceholderText(tr("e.g. 120"));
    cy->setPlaceholderText(tr("e.g. 80"));
    r->setPlaceholderText(tr("e.g. 40"));
    form->addRow(tr("Center X:"), cx);
    form->addRow(tr("Center Y:"), cy);
    form->addRow(tr("Radius:"), r);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    bool okCx = false, okCy = false, okR = false;
    const int centerX = cx->text().toInt(&okCx);
    const int centerY = cy->text().toInt(&okCy);
    const int radius = r->text().toInt(&okR);
    if (!okCx || !okCy || !okR || radius <= 0)
        return;

    canvas->addCircle(QPoint(centerX, centerY), radius);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (canvas)
        canvas->saveToFile(sceneFilePath);
    QMainWindow::closeEvent(event);
}

void MainWindow::deleteSelected()
{
    if (canvas)
        canvas->deleteSelected();
}
