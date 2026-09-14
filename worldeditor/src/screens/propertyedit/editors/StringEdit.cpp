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
#include "StringEdit.h"
#include "ui_StringEdit.h"

#include <QTimer>

StringEdit::StringEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::StringEdit) {

    ui->setupUi(this);
    ui->pushButton->hide();

    connect(ui->lineEdit, SIGNAL(editingFinished()), this, SIGNAL(editFinished()));

    ui->lineEdit->installEventFilter(this);
}

StringEdit::~StringEdit() {
    delete ui;
}

void StringEdit::setData(const Variant &data) {
    ui->lineEdit->setText(data.toString().data());
}

Variant StringEdit::data() const {
    return TString(ui->lineEdit->text().toStdString());
}

bool StringEdit::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) {
        QLineEdit *line = static_cast<QLineEdit *>(obj);
        QTimer::singleShot(0, line, SLOT(selectAll()));
    }
    return QObject::eventFilter(obj, event);
}
