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
#include "importqueue.h"
#include "ui_importqueue.h"

#include <QKeyEvent>
#include <QScreen>

#include <editor/projectsettings.h>
#include <editor/assetmanager.h>

#include "iconrender.h"

ImportQueue::ImportQueue(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ImportQueue) {
    ui->setupUi(this);

    AssetManager *manager = Editor::assets();
    connect(manager, &AssetManager::importStarted, this, &ImportQueue::onStarted);
    connect(manager, &AssetManager::imported, this, &ImportQueue::onProcessed);
    connect(manager, &AssetManager::importFinished, this, &ImportQueue::onImportFinished);

    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    QRect r = QApplication::screens().at(0)->geometry();
    move(r.center() - rect().center());
}

ImportQueue::~ImportQueue() {
    delete ui;
}

void ImportQueue::onProcessed() {
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void ImportQueue::onStarted(int count, const TString &action) {
    show();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(count);
    ui->label->setText(action.data());
}

void ImportQueue::onImportFinished() {
    hide();
    emit importFinished();
}

void ImportQueue::keyPressEvent(QKeyEvent *e) {
    if(e->key() == Qt::Key_Escape) {
        e->ignore();
    }
}

void ImportQueue::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
