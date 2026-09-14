/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
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
    ui->version->setText(tr("Based on %2 rev %3").arg(SDK_VERSION, REVISION));
    ui->copyright->setText(tr("Copyright 2007-%1 by %2. All rights reserved.").arg(COPYRIGHT_YEAR).arg(COMPANY_NAME));
    ui->legal->setText(LEGAL);

    ui->version->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QFile file(":/Sponsors/sponsors.md");
    if(file.open(QFile::ReadOnly)) {
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

