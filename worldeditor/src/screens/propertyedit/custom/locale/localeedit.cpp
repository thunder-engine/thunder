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
#include "localeedit.h"
#include "ui_localeedit.h"

#include <url.h>

#include <QDirIterator>
#include <QLocale>

LocaleEdit::LocaleEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::LocaleEdit) {

    ui->setupUi(this);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(dataChanged()));

    QDirIterator it(":/Translations", QDirIterator::Subdirectories);
    while(it.hasNext()) {
        QLocale locale(Url(it.next().toStdString()).baseName().data());
        QString name = locale.nativeLanguageName();
        ui->comboBox->addItem(name.replace(0, 1, name[0].toUpper()), locale.bcp47Name());
    }
}

LocaleEdit::~LocaleEdit() {
    delete ui;
}

Variant LocaleEdit::data() const {
    return TString (ui->comboBox->currentData().toString().toStdString());
}

void LocaleEdit::setData(const Variant &data) {
    TString lang = data.toString();
    if(lang.isEmpty()) {
        lang = "en";
    }
    int index = ui->comboBox->findData(lang.data());
    if(index == -1) {
        return;
    }
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(index);
    ui->comboBox->blockSignals(false);
}
