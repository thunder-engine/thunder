#include "converters/uiconverter.h"

#include <resources/uidocument.h>

#define FORMAT_VERSION 1

UiConverterSettings::UiConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList UiConverterSettings::typeNames() const {
    return { "UiDocument" };
}

TString UiConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/ui.svg";
}

AssetConverter::ReturnCode UiConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        UiDocument *document = Engine::loadResource<UiDocument>(settings->destination());
        if(document == nullptr) {
            document = Engine::objectCreate<UiDocument>(settings->destination());
        }

        TString data(src.readAll());
        src.close();
        if(!data.isEmpty()) {
            document->setData(data);
        }

        settings->info().id = document->uuid();

        return settings->saveBinary(Engine::toVariant(document), settings->absoluteDestination());
    }

    return InternalError;
}

AssetConverterSettings *UiConverter::createSettings() {
    return new UiConverterSettings();
}

TString UiConverter::templatePath() const {
    return ":/templates/UIDocument.ui";
}
