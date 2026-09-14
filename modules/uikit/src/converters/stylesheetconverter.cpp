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
#include "converters/stylesheetconverter.h"

#include <resources/stylesheet.h>

#define FORMAT_VERSION 1

StyleSheetConverterSettings::StyleSheetConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList StyleSheetConverterSettings::typeNames() const {
    return { MetaType::name<StyleSheet>() };
}

void StyleSheetConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/css.svg");
    }
}

AssetConverter::ReturnCode StyleSheetConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::Read)) {
        StyleSheet *style = Engine::loadResource<StyleSheet>(settings->destination());
        if(style == nullptr) {
            style = Engine::objectCreate<StyleSheet>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(style->uuid() != uuid) {
            Engine::replaceUUID(style, uuid);
        }

        TString array(src.readAll());
        src.close();
        if(!array.isEmpty()) {
            style->setData(array);
        }

        return settings->saveBinary(style, settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *StyleSheetConverter::createSettings() {
    return new StyleSheetConverterSettings();
}

TString StyleSheetConverter::templatePath() const {
    return ":/templates/StyleSheet.css";
}
