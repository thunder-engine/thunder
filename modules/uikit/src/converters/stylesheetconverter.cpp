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
    if(src.open(File::ReadOnly)) {
        StyleSheet *style = Engine::loadResource<StyleSheet>(settings->destination());
        if(style == nullptr) {
            style = Engine::objectCreate<StyleSheet>(settings->destination());
        }

        TString array(src.readAll());
        src.close();
        if(!array.isEmpty()) {
            style->setData(array);
        }

        settings->info().id = style->uuid();

        return settings->saveBinary(Engine::toVariant(style), settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *StyleSheetConverter::createSettings() {
    return new StyleSheetConverterSettings();
}

TString StyleSheetConverter::templatePath() const {
    return ":/templates/StyleSheet.css";
}
