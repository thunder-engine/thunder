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
#include "commitrevert.h"
#include "ui_commitrevert.h"

#include <editor/assetconverter.h>
#include <editor/assetmanager.h>

#include <message.h>

CommitRevert::CommitRevert(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CommitRevert),
        m_propertyObject(nullptr),
        m_proxy(new CommitRevertProxy) {

    ui->setupUi(this);

    m_proxy->setEditor(this);

    ui->commitButton->setProperty("green", true);
}

CommitRevert::~CommitRevert() {
    delete ui;
}

void CommitRevert::setObject(Object *object) {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings && settings != object) {
        checkImportSettings(settings);
        Object::disconnect(settings, _SIGNAL(updated()), m_proxy, _SLOT(onUpdated()));
    }

    settings = dynamic_cast<AssetConverterSettings *>(object);
    if(settings) {
        Object::connect(settings, _SIGNAL(updated()), m_proxy, _SLOT(onUpdated()));

        ui->commitButton->setEnabled(settings->isModified());
        ui->revertButton->setEnabled(settings->isModified());
    }

    m_propertyObject = object;
}

void CommitRevert::onSettingsUpdated() {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings) {
        ui->commitButton->setEnabled(settings->isModified());
        ui->revertButton->setEnabled(settings->isModified());
    }
}

void CommitRevert::on_commitButton_clicked() {
    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings && settings->isModified()) {
        settings->saveSettings();
        Editor::assets()->pushToImport(settings);
        Editor::assets()->reimport();
    }

    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);
}

void CommitRevert::on_revertButton_clicked() {
    ui->commitButton->setEnabled(false);
    ui->revertButton->setEnabled(false);

    AssetConverterSettings *settings = dynamic_cast<AssetConverterSettings *>(m_propertyObject);
    if(settings) {
        settings->loadSettings();
        emit reverted();
    }
}

void CommitRevert::checkImportSettings(AssetConverterSettings *settings) {
    if(settings->isModified()) {
        Message msgBox;
        msgBox.setIcon(Message::Question);
        msgBox.setText(tr("The import settings has been modified.").toStdString());
        msgBox.setInformativeText(tr("Do you want to save your changes?").toStdString());
        msgBox.setStandardButtons(Message::Yes | Message::No | Message::Cancel);
        msgBox.setDefaultButton(Message::Cancel);

        int result = msgBox.exec();
        if(result == Message::Cancel) {
            return;
        }
        if(result == Message::Yes) {
            settings->saveSettings();
            Editor::assets()->pushToImport(settings);
            Editor::assets()->reimport();
        }
        if(result == Message::No) {
            settings->loadSettings();
        }
    }
}
