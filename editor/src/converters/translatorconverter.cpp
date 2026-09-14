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
#include "converters/translatorconverter.h"

#include <QFile>

#define FORMAT_VERSION 1

TranslatorConverterSettings::TranslatorConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList TranslatorConverterSettings::typeNames() const {
    return { MetaType::name<Translator>() };
}

void TranslatorConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/l10n.svg");
    }
}

AssetConverter::ReturnCode TranslatorConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QFile::ReadOnly)) {
        Translator *loc = Engine::loadResource<Translator>(settings->destination());
        if(loc == nullptr) {
            loc = Engine::objectCreate<Translator>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(loc->uuid() != uuid) {
            Engine::replaceUUID(loc, uuid);
        }

        while(!src.atEnd()) {
            QByteArray line = src.readLine();
            auto split = line.split(';');
            loc->setPair(split.first().constData(), split.last().constData());
        }
        src.close();

        return settings->saveBinary(loc, settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *TranslatorConverter::createSettings() {
    return new TranslatorConverterSettings();
}
