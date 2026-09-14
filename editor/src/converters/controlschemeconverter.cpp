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
#include "converters/controlschemeconverter.h"

#include <json.h>
#include <file.h>

namespace {
    const char *gData("Data");
}

#define FORMAT_VERSION 1

ControlScehemeConverterSettings::ControlScehemeConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList ControlScehemeConverterSettings::typeNames() const {
    return { MetaType::name<ControlScheme>() };
}

void ControlSchemeConverter::init() {
    AssetConverter::init();
}

AssetConverter::ReturnCode ControlSchemeConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::Read)) {
        ControlScheme *scheme = Engine::loadResource<ControlScheme>(settings->destination());
        if(scheme == nullptr) {
            scheme = Engine::objectCreate<ControlScheme>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(scheme->uuid() != uuid) {
            Engine::replaceUUID(scheme, uuid);
        }

        VariantMap map;
        map[gData] = Json::load(src.readAll());
        ResourceSystem::loadResourceData(scheme, map);

        src.close();

        return settings->saveBinary(scheme, settings->absoluteDestination());
    }
    return InternalError;
}

AssetConverterSettings *ControlSchemeConverter::createSettings() {
    return new ControlScehemeConverterSettings();
}
