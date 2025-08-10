#include "converters/fontconverter.h"

#include <file.h>

namespace {
    const char *gData("Data");
}

#define FORMAT_VERSION 1

FontImportSettings::FontImportSettings() {
    setType(MetaType::type<Font *>());
    setVersion(FORMAT_VERSION);
}

TString FontImportSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/font.svg";
}

AssetConverter::ReturnCode FontConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        Font *font = Engine::loadResource<Font>(settings->destination());
        if(font == nullptr) {
            font = Engine::objectCreate<Font>();
        }

        VariantMap map;
        map[gData] = src.readAll();
        src.close();

        font->loadUserData(map);

        return settings->saveBinary(font);
    }
    return InternalError;
}

AssetConverterSettings *FontConverter::createSettings() {
    return new FontImportSettings();
}
