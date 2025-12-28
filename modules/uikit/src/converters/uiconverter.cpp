#include "converters/uiconverter.h"

#include <resources/uidocument.h>

#define FORMAT_VERSION 1

UiConverterSettings::UiConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList UiConverterSettings::typeNames() const {
    return { MetaType::name<UiDocument>() };
}

void UiConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/ui.svg");
    }
}

AssetConverter::ReturnCode UiConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        UiDocument *document = Engine::loadResource<UiDocument>(settings->destination());
        if(document == nullptr) {
            document = Engine::objectCreate<UiDocument>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(document->uuid() != uuid) {
            Engine::replaceUUID(document, uuid);
        }

        TString data(src.readAll());
        src.close();
        if(!data.isEmpty()) {
            document->setData(data);
        }

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
