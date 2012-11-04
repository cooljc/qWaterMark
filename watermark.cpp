#include <QImage>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QFont>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

#include "watermark.h"
#include "ui_watermark.h"

enum {
    TEXT_CENTRE = 0,
    TEXT_TOP_LEFT,
    TEXT_TOP_CENTRE,
    TEXT_TOP_RIGHT,
    TEXT_BOTTOM_LEFT,
    TEXT_BOTTOM_CENTRE,
    TEXT_BOTTOM_RIGHT
};

WaterMark::WaterMark(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WaterMark)
{
    QSettings settings;

    ui->setupUi(this);

    // fill size combos
    for (int size=40; size<210; size+=10) {
        ui->cmbSize->addItem(tr("%1").arg(size), size);
    }
    //ui->cmbSize->addItem(tr("11"), 11);
    //ui->cmbSize->addItem(tr("12"), 12);
    //ui->cmbSize->addItem(tr("13"), 13);
    //ui->cmbSize->addItem(tr("14"), 14);

    // fill location combos
    ui->cmbLocation->addItem(tr("Centre"), TEXT_CENTRE);
    ui->cmbLocation->addItem(tr("Top Left"), TEXT_TOP_LEFT);
    ui->cmbLocation->addItem(tr("Top Centre"), TEXT_TOP_CENTRE);
    ui->cmbLocation->addItem(tr("Top Right"), TEXT_TOP_RIGHT);
    ui->cmbLocation->addItem(tr("Bottom Left"), TEXT_BOTTOM_LEFT);
    ui->cmbLocation->addItem(tr("Bottom Centre"), TEXT_BOTTOM_CENTRE);
    ui->cmbLocation->addItem(tr("Bottom Right"), TEXT_BOTTOM_RIGHT);

    // used for testing...
#if 0
    ui->txtInFile->setText(tr("/home/jon/Pictures/JonCross.jpg"));
    ui->txtOutFile->setText(tr("/home/jon/JonCross.jpg"));
#endif

    // load previos options
    int index = 0;
    ui->txtWatermark->setText(settings.value("watermark", "Hello World").toString());
    index = ui->cmbLocation->findData(settings.value("location", TEXT_CENTRE).toInt());
    ui->cmbLocation->setCurrentIndex(index);
    index = ui->cmbSize->findData(settings.value("size", 40).toInt());
    ui->cmbSize->setCurrentIndex(index);
    index = ui->cmbFont->findText(settings.value("font").toString());
    ui->cmbFont->setCurrentIndex(index);

}

WaterMark::~WaterMark()
{
    delete ui;
}

void WaterMark::on_btnBrowseIn_clicked()
{
    QString file;
    QSettings settings;
    file = QFileDialog::getOpenFileName(this, tr("Select Input file"),
                                        settings.value("inpath", QDir::homePath()).toString(),
                                        tr("Images (*.png *.xpm *.jpg)"));
    if (!file.isEmpty()) {
        ui->txtInFile->setText(file);
        QString new_filename = file.mid(0, (file.length()-4)) + ".watermark.jpg";
        ui->txtOutFile->setText(new_filename);
        settings.setValue("inpath", file);
    }
}

void WaterMark::on_btnBrowseOut_clicked()
{
    QString file;
    QSettings settings;
    file = QFileDialog::getSaveFileName(this, tr("Select Output file"),
                                        settings.value("outpath", QDir::homePath()).toString(),
                                        tr("Images (*.png *.xpm *.jpg)"));
    if (!file.isEmpty()) {
        ui->txtOutFile->setText(file);
        settings.setValue("outpath", file);
    }
}

void WaterMark::on_btnConvert_clicked()
{
    QImage resultImage;
    QPainter painter;
    QFont font;

    int size = 0;


    // sanity check
    if (ui->txtInFile->text().isEmpty()) {
        QMessageBox::critical(this, tr("Missing Infile"),
                              tr("Please select an input picture file!!"));
        ui->txtInFile->setFocus();
    }

    if (ui->txtOutFile->text().isEmpty()) {
        QMessageBox::critical(this, tr("Missing Outfile"),
                              tr("Please select an output picture file!!"));
        ui->txtOutFile->setFocus();
    }
    if (ui->txtInFile->text() == ui->txtOutFile->text()) {
        int ret = QMessageBox::question(this, tr("Overwrite?"),
                              tr("Output file is the same as input file. Are you sure you want to overwrite?"),
                              QMessageBox::Yes|QMessageBox::No);
        if (ret == QMessageBox::No) {
            return;
        }
    }
    // disable controls
    ui->groupBox->setEnabled(false);
    ui->groupBox_2->setEnabled(false);
    ui->groupBox_3->setEnabled(false);

    // load input image
    resultImage.load(ui->txtInFile->text());

    // calculate size and position of text overlay
    font = ui->cmbFont->currentFont();
    size = ui->cmbSize->itemData(ui->cmbSize->currentIndex()).toInt();
    font.setPointSize(size);

    QFontMetrics fm(font);
    int textWidthInPixels = fm.width(ui->txtWatermark->text());
    int textHeightInPixels = fm.height();

    // make sure text is not wider that image
    if (textWidthInPixels > resultImage.width()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Watermark is to wide for target image. Try and reduce the font."));
        // enable controls
        ui->groupBox->setEnabled(true);
        ui->groupBox_2->setEnabled(true);
        ui->groupBox_3->setEnabled(true);
        return;
    }

    int posX = 0;
    int posY = 0;
    int pos = ui->cmbLocation->itemData(ui->cmbLocation->currentIndex()).toInt();
    switch (pos)
    {
    case TEXT_CENTRE:
        posX = resultImage.width()/2 - textWidthInPixels/2;
        posY = resultImage.height()/2 + textHeightInPixels/2;
        break;
    case TEXT_TOP_LEFT:
        posX = 10;
        posY = textHeightInPixels;
        break;
    case TEXT_TOP_CENTRE:
        posX = resultImage.width()/2 - textWidthInPixels/2;
        posY = textHeightInPixels;
        break;
    case TEXT_TOP_RIGHT:
        posX = resultImage.width() - (textWidthInPixels + 10);
        posY = textHeightInPixels;
        break;
    case TEXT_BOTTOM_LEFT:
        posX = 10;
        posY = resultImage.height() - 10;
        break;
    case TEXT_BOTTOM_CENTRE:
        posX = resultImage.width()/2 - textWidthInPixels/2;
        posY = resultImage.height() - 10;
        break;
    case TEXT_BOTTOM_RIGHT:
        posX = resultImage.width() - (textWidthInPixels + 10);
        posY = resultImage.height() - 10;
        break;
    }

    // create new image for text overlay
    if(painter.begin(&resultImage)) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setBackgroundMode(Qt::TransparentMode);

        // Set background Opaque
        painter.setBackgroundMode(Qt::OpaqueMode);

        // Set pen to white with transparent
        painter.setPen(QColor(255,255,255,75));
        painter.setBackground(QBrush(Qt::transparent));

        // set font
        painter.setFont(font);

        // position text over image
        painter.drawText(posX, posY, tr("%1").arg(ui->txtWatermark->text()));

        painter.end();
    }

    // save image
    if (!resultImage.save(ui->txtOutFile->text(), "JPG", 100)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to save new image...."));
    }

    // save settings
    QSettings settings;
    settings.setValue("watermark", ui->txtWatermark->text());
    settings.setValue("location", pos);
    settings.setValue("size", size);
    settings.setValue("font", ui->cmbFont->currentText());

    // enable controls
    ui->groupBox->setEnabled(true);
    ui->groupBox_2->setEnabled(true);
    ui->groupBox_3->setEnabled(true);
}
