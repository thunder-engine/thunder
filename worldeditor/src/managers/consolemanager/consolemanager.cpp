#include "consolemanager.h"
#include "ui_consolemanager.h"

#include <stdint.h>

#include "log.h"
#include "logmodel.h"

ConsoleManager::ConsoleManager(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ConsoleManager),
        m_pItems(new LogModel()){
    ui->setupUi(this);
    ui->consoleOutput->setModel(m_pItems);
    ui->infoButton->hide();
    ui->warningButton->hide();
    ui->errorButton->hide();
}

ConsoleManager::~ConsoleManager() {
    delete ui;
}

void ConsoleManager::onLogRecord(uint8_t type, const QString &str) {
    m_pItems->addRecord(type, str);
    ui->consoleOutput->scrollToBottom();
}

void ConsoleManager::on_clearButton_clicked() {
    m_pItems->clear();
}

void ConsoleManager::on_infoButton_clicked() {
}

void ConsoleManager::on_warningButton_clicked() {
}

void ConsoleManager::on_errorButton_clicked() {
}
