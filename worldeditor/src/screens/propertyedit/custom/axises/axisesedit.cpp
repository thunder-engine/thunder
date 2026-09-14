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
#include "axisesedit.h"
#include "ui_axisesedit.h"

AxisesEdit::AxisesEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::AxisesEdit) {
    ui->setupUi(this);

    ui->x->setProperty("checkred", true);
    ui->y->setProperty("checkgreen", true);
    ui->z->setProperty("checkblue", true);

    connect(ui->x, &QPushButton::toggled, this, &AxisesEdit::editFinished);
    connect(ui->y, &QPushButton::toggled, this, &AxisesEdit::editFinished);
    connect(ui->z, &QPushButton::toggled, this, &AxisesEdit::editFinished);
}

AxisesEdit::~AxisesEdit() {
    delete ui;
}

Variant AxisesEdit::data() const {
    int value = 0;
    if(ui->x->isChecked()) {
        value |= AXIS_X;
    }
    if(ui->y->isChecked()) {
        value |= AXIS_Y;
    }
    if(ui->z->isChecked()) {
        value |= AXIS_Z;
    }
    return value;
}

void AxisesEdit::setData(const Variant &data) {
    int value = data.toInt();
    ui->x->setChecked(value & AXIS_X);
    ui->y->setChecked(value & AXIS_Y);
    ui->z->setChecked(value & AXIS_Z);
}
