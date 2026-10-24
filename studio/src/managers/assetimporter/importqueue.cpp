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
#include "importqueue.h"

#include <algorithm>

#include <editor/projectsettings.h>
#include <editor/assetmanager.h>

ImportQueue::ImportQueue() :
        m_started(false),
        m_building(false),
        m_totalImports(0),
        m_processedImports(0) {
    setValue(0.0f);
}

void ImportQueue::pollImport() {
    AssetManager *manager = Editor::assets();
    if(!m_started) {
        if(manager->pendingImportCount() == 0) {
            return;
        }
        startImport();
    }

    if(m_building) {
        m_building = manager->runBuilders();
        if(!m_building) {
            finishImport();
        }
    } else if(manager->pendingImportCount() > 0) {
        manager->importNext();
        ++m_processedImports;
        m_totalImports = std::max(m_totalImports, m_processedImports + manager->pendingImportCount());
        setTo(static_cast<float>(m_totalImports));
        setValue(static_cast<float>(m_processedImports));
    } else if(manager->finishImport()) {
        m_building = manager->runBuilders();
        if(!m_building) {
            finishImport();
        }
    }
}

void ImportQueue::startImport() {
    if(m_started) {
        return;
    }

    m_started = true;
    m_building = false;
    m_totalImports = Editor::assets()->pendingImportCount();
    m_processedImports = 0;
    setFrom(0.0f);
    setTo(static_cast<float>(std::max(m_totalImports, 1)));
    setValue(0.0f);
    setEnabled(true);
    setVisible(true);
}

void ImportQueue::finishImport() {
    m_started = false;
    m_building = false;
    setEnabled(false);
    setVisible(false);
    importFinished();
}

void ImportQueue::importFinished() {
    emitSignal(_SIGNAL(importFinished()));
}
