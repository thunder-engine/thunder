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
#include "consolemanager.h"
#include "ui_consolemanager.h"

#include <stdint.h>

#include <QMenu>
#include <QClipboard>
#include <QMessageBox>

#include "logmodel.h"
#include "qlog.h"

ConsoleManager::ConsoleManager(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ConsoleManager),
        m_model(new LogModel),
        m_menu(new QMenu(this)),
        m_handler(new QLog) {

    ui->setupUi(this);
    ui->consoleOutput->setModel(m_model);
    ui->toolButton->hide();

    ui->consoleOutput->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *action = m_menu->addAction(tr("Copy"));

    connect(action, &QAction::triggered, this, &ConsoleManager::onCopy);
    connect(m_handler, &QLog::postRecord, this, &ConsoleManager::onLogRecord);

    Log::addHandler(m_handler);
}

ConsoleManager::~ConsoleManager() {
    delete ui;
}

void ConsoleManager::onLogRecord(uint8_t type, const QString &str) {
    m_model->addRecord(type, str);
    ui->consoleOutput->scrollToBottom();
}

void ConsoleManager::on_clearButton_clicked() {
    m_model->clear();
}

void ConsoleManager::on_consoleOutput_customContextMenuRequested(const QPoint &pos) {
    m_menu->exec(mapToGlobal(pos));
}

void ConsoleManager::onCopy() {
    QStringList list;
    foreach(const QModelIndex &index, ui->consoleOutput->selectionModel()->selectedIndexes()) {
        list.push_back(m_model->data(index, Qt::DisplayRole).toString());
    }
    if(!list.isEmpty()) {
        QApplication::clipboard()->setText(list.join("\n"));
    }
}

void ConsoleManager::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void ConsoleManager::parseLogs(const QString &log) {
    static const QRegularExpression expr("[\r\n]");
    QStringList list = log.split(expr, Qt::SkipEmptyParts);

    foreach(QString it, list) {
        if(it.contains(" critical ") || it.contains(" critical:", Qt::CaseInsensitive)) {
            onLogRecord(Log::CRT, qPrintable(it));
        } else if(it.contains(" error ") || it.contains(" error:", Qt::CaseInsensitive)) {
            onLogRecord(Log::ERR, qPrintable(it));
        } else if(it.contains(" warning ") || it.contains(" warning:", Qt::CaseInsensitive)) {
            onLogRecord(Log::WRN, qPrintable(it));
        } else {
            onLogRecord(Log::INF, qPrintable(it));
        }
    }
}
