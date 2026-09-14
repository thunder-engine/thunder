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
#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <QWidget>
#include <stdint.h>

namespace Ui {
    class ConsoleManager;
}

class LogModel;
class QMenu;
class QLog;

class ConsoleManager : public QWidget {
    Q_OBJECT
public:
    explicit ConsoleManager(QWidget *parent = 0);
    ~ConsoleManager();

public slots:
    void onLogRecord(uint8_t type, const QString &str);

    void parseLogs(const QString &log);

private slots:
    void on_clearButton_clicked();

    void on_consoleOutput_customContextMenuRequested(const QPoint &pos);

    void onCopy();

private:
    void changeEvent(QEvent *event) override;

    Ui::ConsoleManager *ui;

    LogModel *m_model;

    QMenu *m_menu;

    QLog *m_handler;
};

#endif // CONSOLEMANAGER_H
