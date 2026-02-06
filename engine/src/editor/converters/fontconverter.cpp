#include "converters/fontconverter.h"

#include <file.h>

namespace {
    const char *gData("Data");
}

#define FORMAT_VERSION 1

FontImportSettings::FontImportSettings() {
    setVersion(FORMAT_VERSION);
}

StringList FontImportSettings::typeNames() const {
    return { MetaType::name<Font>() };
}

void FontConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/font.svg");
    }
}

AssetConverter::ReturnCode FontConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        Font *font = Engine::loadResource<Font>(settings->destination());
        if(font == nullptr) {
            font = Engine::objectCreate<Font>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(font->uuid() != uuid) {
            Engine::replaceUUID(font, uuid);
        }

        VariantMap map;
        map[gData] = src.readAll();
        src.close();

        return settings->saveBinary(Engine::toVariant(font), settings->absoluteDestination());
    }
    return InternalError;
}

AssetConverterSettings *FontConverter::createSettings() {
    return new FontImportSettings();
}
