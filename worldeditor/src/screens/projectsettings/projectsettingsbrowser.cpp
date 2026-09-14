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
#include "projectsettingsbrowser.h"
#include "ui_projectsettingsbrowser.h"

#include <QEvent>

#include <editor/projectsettings.h>

ProjectSettingsBrowser::ProjectSettingsBrowser(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ProjectSettingsBrowser) {

    ui->setupUi(this);

    connect(ui->projectWidget, &PropertyEditor::objectsChanged, this, &ProjectSettingsBrowser::onSettingsUpdated);
}

ProjectSettingsBrowser::~ProjectSettingsBrowser() {
    delete ui;
}

void ProjectSettingsBrowser::init() {
    ui->projectWidget->onObjectSelected(Editor::project());
}

void ProjectSettingsBrowser::onSettingsUpdated(const Object::ObjectList &list, const TString &property, Variant value) {
    Editor::project()->setProperty(property.data(), value);
}

void ProjectSettingsBrowser::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
