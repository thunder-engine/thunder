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
    return { "Font" };
}

TString FontImportSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/font.svg";
}

AssetConverter::ReturnCode FontConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        Font *font = Engine::loadResource<Font>(settings->destination());
        if(font == nullptr) {
            font = Engine::objectCreate<Font>(settings->destination());
        }

        VariantMap map;
        map[gData] = src.readAll();
        src.close();

        font->loadUserData(map);

        return settings->saveBinary(Engine::toVariant(font), settings->absoluteDestination());
    }
    return InternalError;
}

AssetConverterSettings *FontConverter::createSettings() {
    return new FontImportSettings();
}
