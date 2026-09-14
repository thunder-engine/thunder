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
#include "actions.h"
#include "ui_actions.h"

#include "../../propertyeditor.h"

#include <editor/asseteditor.h>

Actions::Actions(QWidget *parent) :
        PropertyEdit(parent),
        ui(new Ui::Actions),
        m_property(MetaProperty(nullptr)) {

    ui->setupUi(this);
}

Actions::~Actions() {
    delete ui;
}

void Actions::setObject(Object *object, const TString &name) {
    PropertyEdit::setObject(object, name);

    if(m_object == nullptr) {
        return;
    }
    const MetaObject *meta = m_object->metaObject();

    int index = meta->indexOfProperty("enabled");
    if(index > -1) {
        m_property = meta->property(index);
    }

    PropertyEditor *editor = findEditor(parentWidget());
    if(editor) {
        AssetEditor *assetEditor = editor->currentEditor();
        if(assetEditor) {
            for(auto it : assetEditor->propertiesActionWidgets(m_object, this)) {
                ui->horizontalLayout->addWidget(it);
            }
        }
    }
}

void Actions::onDataChanged(bool value) {
    if(m_property.isValid()) {
        m_property.write(m_object, value);
    }
}

bool Actions::isChecked() const {
    if(m_property.isValid()) {
        return m_property.read(m_object).toBool();
    }
    return false;
}

PropertyEditor *Actions::findEditor(QWidget *parent) const {
    PropertyEditor *editor = dynamic_cast<PropertyEditor *>(parent);
    if(editor) {
        return editor;
    }

    if(parent) {
        return findEditor(parent->parentWidget());
    }

    return nullptr;
}
