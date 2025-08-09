#include "converters/fontconverter.h"

#include <bson.h>
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

        File file(settings->absoluteDestination());
        if(file.open(File::WriteOnly)) {
            file.write(Bson::save( Engine::toVariant(font) ));
            file.close();
            return Success;
        }
    }
    return InternalError;
}

AssetConverterSettings *FontConverter::createSettings() {
    return new FontImportSettings();
}
