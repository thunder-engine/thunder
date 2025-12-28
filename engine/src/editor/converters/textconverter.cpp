#include "converters/textconverter.h"

#include <file.h>

#include "resources/text.h"

#define FORMAT_VERSION 1

TextConverterSettings::TextConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList TextConverterSettings::typeNames() const {
    return { MetaType::name<Text>() };
}

void TextConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/text.svg");
    }
}

AssetConverter::ReturnCode TextConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        Text *text = Engine::loadResource<Text>(settings->destination());
        if(text == nullptr) {
            text = Engine::objectCreate<Text>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(text->uuid() != uuid) {
            Engine::replaceUUID(text, uuid);
        }

        TString content(src.readAll());
        src.close();
        if(!content.isEmpty()) {
            text->setSize(content.size());
            memcpy(text->data(), content.data(), content.size());
        }

        return settings->saveBinary(Engine::toVariant(text), settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *TextConverter::createSettings() {
    return new TextConverterSettings();
}
