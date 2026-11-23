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
#include "converters/uiconverter.h"

#include <resources/uidocument.h>

#define FORMAT_VERSION 1

UiConverterSettings::UiConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList UiConverterSettings::typeNames() const {
    return { MetaType::name<UiDocument>() };
}

void UiConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/ui.svg");
    }
}

AssetConverter::ReturnCode UiConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::Read)) {
        UiDocument *document = Engine::loadResource<UiDocument>(settings->destination());
        if(document == nullptr) {
            document = Engine::objectCreate<UiDocument>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(document->uuid() != uuid) {
            Engine::replaceUUID(document, uuid);
        }

        TString data(src.readAll());
        src.close();
        if(!data.isEmpty()) {
            document->setData(data);
        }

        return settings->saveBinary(document, settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *UiConverter::createSettings() {
    return new UiConverterSettings();
}

TString UiConverter::templatePath() const {
    return ":/templates/UIDocument.ui";
}
