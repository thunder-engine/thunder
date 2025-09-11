#include "converters/textconverter.h"

#include <file.h>

#include "resources/text.h"

#define FORMAT_VERSION 1

TextConverterSettings::TextConverterSettings() {
    setVersion(FORMAT_VERSION);
}

TString TextConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/text.svg";
}

StringList TextConverterSettings::typeNames() const {
    return { "Text" };
}

AssetConverter::ReturnCode TextConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        Text *text = Engine::loadResource<Text>(settings->destination());
        if(text == nullptr) {
            text = Engine::objectCreate<Text>(settings->destination());
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
