#include "converters/stylesheetconverter.h"

#include <resources/stylesheet.h>

#define FORMAT_VERSION 1

StyleSheetConverterSettings::StyleSheetConverterSettings() {
    setType(MetaType::type<StyleSheet *>());
    setVersion(FORMAT_VERSION);
}

TString StyleSheetConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/css.svg";
}

AssetConverter::ReturnCode StyleSheetConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        StyleSheet *style = Engine::loadResource<StyleSheet>(settings->destination());
        if(style == nullptr) {
            style = Engine::objectCreate<StyleSheet>();
        }

        TString array(src.readAll());
        src.close();
        if(!array.isEmpty()) {
            style->setData(array);
        }

        return settings->saveBinary(Engine::toVariant(style));
    }

    return InternalError;
}

AssetConverterSettings *StyleSheetConverter::createSettings() {
    return new StyleSheetConverterSettings();
}

TString StyleSheetConverter::templatePath() const {
    return ":/templates/StyleSheet.css";
}
