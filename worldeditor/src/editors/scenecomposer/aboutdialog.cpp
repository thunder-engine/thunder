#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::AboutDialog) {

    ui->setupUi(this);

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    setWindowTitle(tr("About %1...").arg(EDITOR_NAME));
    ui->name->setText(EDITOR_NAME);
    ui->revision->setText(tr("From revision %1").arg(REVISION));
    ui->version->setText(tr("Based on %1 %2").arg(PRODUCT_NAME).arg(SDK_VERSION));
    ui->copyright->setText(tr("Copyright 2007-%1 by %2. All rights reserved.").arg(COPYRIGHT_YEAR).arg(COPYRIGHT_AUTHOR));
    ui->legal->setText(LEGAL);
    ui->thanks->setText(tr("Special thanks to: %1").arg(SPONSORS));
}

AboutDialog::~AboutDialog() {
    delete ui;
}
