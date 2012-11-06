/*
 * watermark.cpp
 *
 * Copyright 2012 Jon Cross <joncross.cooljc@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */
#include <QPixmap>
#include <QPainter>
#include <QFile>
#include <QDir>
#include <QFont>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QColorDialog>

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
    for (int size=40; size<310; size+=10) {
        ui->cmbSize->addItem(tr("%1").arg(size), size);
    }

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
    ui->spbWeight->setValue(settings.value("weight", 75).toInt());
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
        // Autogenerate out file
        QString new_filename = file.mid(0, (file.length()-4)) + ".wm.jpg";
        ui->txtOutFile->setText(new_filename);
        // save infile path
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

void WaterMark::on_btnBrowseInDir_clicked()
{
#if 0
    QColor c = QColorDialog::getColor(Qt::white, this,
                                      tr("Select Colour"),
                                      QColorDialog::ShowAlphaChannel);
    //setBackgroundRole();
    QString sColour = c.name(); //tr("#%1%2%3%4").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
    ui->txtInDir->setText(sColour);
    ui->lblPicture->setStyleSheet(tr("QLabel { background-color :%1 }").arg(c.name()));
#endif
}

void WaterMark::on_btnConvert_clicked()
{
    QPixmap resultImage;
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

    int weight = ui->spbWeight->value();

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
        painter.setPen(QColor(255,255,255,weight));
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

    ui->lblPicture->setPixmap(resultImage.scaled(500, 500, Qt::KeepAspectRatio));

    // save settings
    QSettings settings;
    settings.setValue("watermark", ui->txtWatermark->text());
    settings.setValue("location", pos);
    settings.setValue("size", size);
    settings.setValue("font", ui->cmbFont->currentText());
    settings.setValue("weight", weight);

    // enable controls
    ui->groupBox->setEnabled(true);
    ui->groupBox_2->setEnabled(true);
    ui->groupBox_3->setEnabled(true);
}
