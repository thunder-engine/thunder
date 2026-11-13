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

#include <algorithm>

#include <editor/projectsettings.h>
#include <editor/assetmanager.h>

ImportQueue::ImportQueue(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ImportQueue),
        m_started(false),
        m_building(false),
        m_totalImports(0),
        m_processedImports(0) {
    ui->setupUi(this);

    connect(&m_importTimer, &QTimer::timeout, this, &ImportQueue::pollImport);
    m_importTimer.start(16);

    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);

    QRect r = QApplication::screens().at(0)->geometry();
    move(r.center() - rect().center());
}

ImportQueue::~ImportQueue() {
    delete ui;
}

void ImportQueue::pollImport() {
    AssetManager *manager = Editor::assets();
    if(!m_started) {
        if(manager->pendingImportCount() == 0) {
            return;
        }
        startImport();
    }

    if(m_building) {
        m_building = manager->runBuilders();
        if(!m_building) {
            m_started = false;
            hide();
            emit importFinished();
        }
    } else if(manager->pendingImportCount() > 0) {
        manager->importNext();
        ++m_processedImports;
        m_totalImports = std::max(m_totalImports, m_processedImports + manager->pendingImportCount());
        ui->progressBar->setMaximum(m_totalImports);
        ui->progressBar->setValue(m_processedImports);
    } else if(manager->finishImport()) {
        m_building = manager->runBuilders();
        if(!m_building) {
            m_started = false;
            hide();
            emit importFinished();
        }
    }
}

void ImportQueue::startImport() {
    if(m_started) {
        return;
    }

    m_started = true;
    m_building = false;
    m_totalImports = Editor::assets()->pendingImportCount();
    m_processedImports = 0;
    show();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(m_totalImports);
    ui->label->setText(tr("Importing resources"));
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
