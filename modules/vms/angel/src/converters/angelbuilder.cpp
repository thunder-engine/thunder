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
#include "converters/angelbuilder.h"

#include <log.h>
#include <bson.h>
#include <file.h>
#include <url.h>

#include <angelscript.h>

#include <QFile>

#include <set>

#include "angelsystem.h"
#include "resources/angelscript.h"

#include <editor/projectsettings.h>
#include <editor/assetmanager.h>

#define DATA    "Data"
#define GET     "get_"

namespace {
    const char *g_assetPath("/AngelScript");
}

class CBytecodeStream : public asIBinaryStream {
public:
    explicit CBytecodeStream(ByteArray &ptr) :
        array(ptr) {

    }
    int Write(const void *ptr, asUINT size) {
        if(size == 0) {
            return 0;
        }
        uint32_t offset = array.size();
        array.resize(offset + size);
        memcpy(&array[offset], ptr, size);

        return static_cast<int>(size);
    }
    int Read(void *ptr, asUINT size) {
        A_UNUSED(ptr);
        A_UNUSED(size);
        return 0;
    }
protected:
    ByteArray &array;

};

StringList AngelScriptImportSettings::typeNames() const {
    return { "AngelScript" };
}

AngelBuilder::AngelBuilder(AngelSystem *system) :
        m_system(system),
        m_scriptEngine(asCreateScriptEngine()),
        m_rebuildPending(false) {

    m_scriptEngine->SetMessageCallback(asFUNCTION(messageCallback), nullptr, asCALL_CDECL);
}

AngelBuilder::~AngelBuilder() {
    m_scriptEngine->ShutDownAndRelease();
}

void AngelBuilder::init() {
    m_system->registerClasses(m_scriptEngine);
    //m_classModel->update(m_scriptEngine);

    ResourceSystem::ResourceInfo info;
    info.uuid = AssetConverterSettings::fixUuid(Editor::project()->projectId(), "AngelScript", 0);
    info.type = "AngelScript";
    Editor::assets()->registerAsset(Editor::project()->contentPath() + g_assetPath, info);

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/code.svg");
    }
}

AssetConverter::ReturnCode AngelBuilder::convertFile(AssetConverterSettings *settings) {
    m_rebuildPending = true;
    m_batchSettings.insert(settings);
    return Deferred;
}

AssetConverterSettings *AngelBuilder::createSettings() {
    return new AngelScriptImportSettings;
}

void AngelBuilder::onFileRemoved(const TString &) {
    m_rebuildPending = true;
}

void AngelBuilder::finalizeBatch() {
    if(!m_rebuildPending) {
        return;
    }
    m_rebuildPending = false;

    AssetManager *assetMgr = Editor::assets();
    ProjectSettings *project = Editor::project();
    TString persistentUUID = AssetConverterSettings::fixUuid(project->projectId(), "AngelScript", 0);

    std::set<TString> sources;
    for(const TString &path : File::list(project->contentPath())) {
        if(Url(path).completeSuffix().toLower() == "as") {
            sources.insert(path);
        }
    }

    if(sources.empty()) {
        File::remove(project->importPath() + "/" + persistentUUID);
        assetMgr->unregisterAsset(project->contentPath() + g_assetPath);
        m_system->unloadAll(false);
        assetMgr->buildSuccessful(true);
        m_batchSettings.clear();
        return;
    }

    asIScriptModule *mod = m_scriptEngine->GetModule("AngelBuilder", asGM_CREATE_IF_NOT_EXISTS);

    QFile base(":/Behaviour.txt");
    if(base.open(QFile::ReadOnly)) {
        TString code(base.readAll());
        mod->AddScriptSection("AngelData", code.data());
        base.close();
    }
    for(const TString &path : sources) {
        File file(path);
        if(file.open(File::Read)) {
            TString code(file.readAll());
            mod->AddScriptSection("AngelData", code.data());
            file.close();
        }
    }

    bool success = mod->Build() >= 0;
    if(!success) {
        aError() << "AngelScript module compilation failed.";
    }
    if(success) {
        TString destination = project->importPath() + "/" + persistentUUID;

        File dst(destination.data());
        if(dst.open(File::Write)) {
            AngelScript *serial = Engine::loadResource<AngelScript>(persistentUUID);
            if(serial == nullptr) {
                serial = Engine::objectCreate<AngelScript>(persistentUUID);
            }

            serial->m_array.clear();
            CBytecodeStream stream(serial->m_array);
            mod->SaveByteCode(&stream);

            dst.write(Bson::save(Engine::toVariant(serial)));
            dst.close();

            ResourceSystem::ResourceInfo info;
            info.uuid = persistentUUID;
            info.type = "AngelScript";
            assetMgr->registerAsset(project->contentPath() + g_assetPath, info);
        } else {
            aError() << "Unable to write compiled AngelScript module:" << destination;
            success = false;
        }

        if(success) {
            if(m_system->init()) {
                m_system->reload();
            } else {
                aError() << "Unable to initialize AngelScript runtime.";
                success = false;
            }
        }
    }

    mod->Discard();
    if(success) {
        for(AssetConverterSettings *settings : m_batchSettings) {
            settings->setCurrentVersion(settings->version());
            settings->saveSettings();
        }
    }
    m_batchSettings.clear();
    assetMgr->buildSuccessful(success);
}

void AngelBuilder::messageCallback(const asSMessageInfo *msg, void *param) {
    A_UNUSED(param);
    Log::LogTypes type = Log::INF;
    switch(msg->type) {
        case asMSGTYPE_ERROR: type = Log::ERR; break;
        case asMSGTYPE_WARNING: type = Log::WRN; break;
        case asMSGTYPE_INFORMATION: type = Log::INF; break;
        default: break;
    }
    Log(type) << msg->section << "(line:" << msg->row << "col:" << msg->col << "):" << msg->message;
}
