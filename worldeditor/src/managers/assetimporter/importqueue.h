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
#ifndef IMPORTQUEUE_H
#define IMPORTQUEUE_H

#include <QDialog>

#include <stdint.h>
#include <astring.h>

namespace Ui {
    class ImportQueue;
}

class ImportQueue : public QDialog {
    Q_OBJECT

public:
    explicit ImportQueue(QWidget *parent = nullptr);
    ~ImportQueue();

signals:
    void importFinished();

private slots:
    void onProcessed();

    void onStarted(int count, const TString &action);
    void onImportFinished();

private:
    void keyPressEvent(QKeyEvent *e) override;
    void changeEvent(QEvent *event) override;

private:
    Ui::ImportQueue *ui;

};

#endif // IMPORTQUEUE_H
