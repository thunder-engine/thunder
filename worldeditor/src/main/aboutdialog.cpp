#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QFile>
#include <QSysInfo>
#include <QClipboard>

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

    ui->revision->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->version->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QFile file(":/Sponsors/sponsors.md");
    if(file.open(QIODevice::ReadOnly)) {
        ui->thanks->setText(file.readAll());
        file.close();
    }
}

AboutDialog::~AboutDialog() {
    delete ui;
}

void AboutDialog::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void AboutDialog::on_pushClipboard_clicked() {
    QStringList data;

    data << QString("Build Version: ") + SDK_VERSION;
    data << QString("Build Revision: ") + REVISION;
    data << "Build CPU Architecture: " + QSysInfo::buildCpuArchitecture();

    data << "OS Name: " + QSysInfo::prettyProductName();
    data << "OS CPU Architecture: " + QSysInfo::currentCpuArchitecture();
    data << "Kernel version: " + QSysInfo::kernelVersion();

    QGuiApplication::clipboard()->setText(data.join("\r\n"));
    ui->pushClipboard->setText(tr("Copied..."));
}

