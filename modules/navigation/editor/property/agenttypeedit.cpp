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
#include "agenttypeedit.h"
#include "ui_agenttypeedit.h"

#include <navigationsystem.h>

AgentTypeEdit::AgentTypeEdit(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::AgentTypeEdit) {
    ui->setupUi(this);

    for(int i = 0; i < NavigationSystem::agentTypeCount(); ++i) {
        AgentType type = NavigationSystem::agentType(i);
        ui->comboBox->addItem(type.name.data());
    }

    connect(ui->comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int) {
        emit dataChanged();
        emit editFinished();
    });
}

AgentTypeEdit::~AgentTypeEdit() {
    delete ui;
}

Variant AgentTypeEdit::data() const {
    return ui->comboBox->currentIndex();
}

void AgentTypeEdit::setData(const Variant &data) {
    int index = data.toInt();
    if(index >= 0 && index < ui->comboBox->count()) {
        ui->comboBox->setCurrentIndex(index);
    }
}
