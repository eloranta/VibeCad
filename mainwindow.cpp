#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canvas.h"

#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QtMath>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPageLayout>
#include <QPageSize>
#include <QMarginsF>
#include <QPrintDialog>
#include <QPrinter>
#include <QPdfWriter>
#include <QStandardPaths>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QDir>
#include <QImage>
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

    const auto tryPrintToPath = [&](const QString &outPath) -> bool {
        QFileInfo fi(outPath);
        if (!fi.dir().exists())
        {
            QDir().mkpath(fi.dir().absolutePath());
        }

        QPdfWriter writer(outPath);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageOrientation(QPageLayout::Landscape);
        writer.setPageMargins(QMarginsF(0, 0, 0, 0));
        writer.setResolution(25); // ~1 px per mm

        const QRectF pageRect = writer.pageLayout().paintRectPixels(writer.resolution());
        if (pageRect.isEmpty())
            return false;

        const qreal scaleX = writer.resolution() / 25.4; // device units per mm (1 px -> 1 mm)
        const qreal scaleY = scaleX;
        if (scaleX <= 0 || scaleY <= 0)
            return false;

        const qreal targetW = canvas->width() * scaleX;
        const qreal targetH = canvas->height() * scaleY;

        const int cols = std::max(1, qCeil(targetW / pageRect.width()));
        const int rows = std::max(1, qCeil(targetH / pageRect.height()));

        QPainter painter(&writer);
        if (!painter.isActive())
            return false;

        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                painter.save();
                const qreal tx = pageRect.x() - col * pageRect.width();
                const qreal ty = pageRect.y() - row * pageRect.height();
                painter.translate(tx, ty);
                painter.scale(scaleX, scaleY);
                canvas->render(&painter);
                painter.restore();

                if (!(row == rows - 1 && col == cols - 1))
                    writer.newPage();
            }
        }

    QMessageBox::information(this, tr("Printed to PDF"),
                             tr("Printed to %1").arg(outPath));
        return true;
    };

    const QString primaryPath = QCoreApplication::applicationDirPath() + "/print.pdf";
    if (tryPrintToPath(primaryPath))
        return;

    const QString fallbackPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                                 + "/VibeCad/print.pdf";
    if (tryPrintToPath(fallbackPath))
        return;

    // Fallback to PNG tiles for debugging
    const QString pngDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                               + "/VibeCad/print_tiles";
    QDir().mkpath(pngDirPath);
    const QSizeF pageSizePx(297.0, 210.0); // 1 px == 1 mm
    const int cols = std::max(1, qCeil(canvas->width() / pageSizePx.width()));
    const int rows = std::max(1, qCeil(canvas->height() / pageSizePx.height()));
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            QImage img(pageSizePx.toSize(), QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::white);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing, true);
            p.translate(-col * pageSizePx.width(), row * pageSizePx.height() - canvas->height());
            canvas->render(&p);
            p.end();
            img.save(QString("%1/tile_r%2_c%3.png").arg(pngDirPath).arg(row).arg(col));
        }
    }
    QMessageBox::warning(this,
                         tr("Print failed"),
                         tr("Could not write PDF to %1 or %2. PNG tiles saved to %3")
                             .arg(primaryPath, fallbackPath, pngDirPath));
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
