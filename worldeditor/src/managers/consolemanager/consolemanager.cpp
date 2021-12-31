#include "consolemanager.h"
#include "ui_consolemanager.h"

#include <stdint.h>

#include <QMenu>
#include <QClipboard>
#include <QProcess>

#include "logmodel.h"
#include "qlog.h"

#include <editor/projectmanager.h>

ConsoleManager::ConsoleManager(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ConsoleManager),
        m_pItems(new LogModel()){
    ui->setupUi(this);
    ui->consoleOutput->setModel(m_pItems);
    ui->toolButton->hide();

    ui->consoleOutput->setContextMenuPolicy(Qt::CustomContextMenu);

    m_pMenu = new QMenu(this);
    QAction *action = m_pMenu->addAction(tr("Copy"));
    connect(action, SIGNAL(triggered()), this, SLOT(onCopy()));

    connect(ProjectManager::instance(), &ProjectManager::readBuildLogs, this, &ConsoleManager::parseLogs);

    connect(static_cast<QLog *>(Log::handler()), SIGNAL(postRecord(uint8_t,QString)), this, SLOT(onLogRecord(uint8_t,QString)));
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

void ConsoleManager::on_consoleOutput_customContextMenuRequested(const QPoint &pos) {
    m_pMenu->exec(mapToGlobal(pos));
}

void ConsoleManager::onCopy() {
    QStringList list;
    foreach(const QModelIndex &index, ui->consoleOutput->selectionModel()->selectedIndexes()) {
        list.push_back(m_pItems->data(index, Qt::DisplayRole).toString());
    }
    if(!list.isEmpty()) {
        QApplication::clipboard()->setText(list.join("\n"));
    }
}

void ConsoleManager::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void ConsoleManager::parseLogs(const QString &log) {
    QStringList list = log.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

    foreach(QString it, list) {
        if(it.contains(" error ")) {
            onLogRecord(Log::ERR, qPrintable(it));
        } else if(it.contains(" warning ")) {
            onLogRecord(Log::WRN, qPrintable(it));
        } else {
            onLogRecord(Log::INF, qPrintable(it));
        }
    }
}
