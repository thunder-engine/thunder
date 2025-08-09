#include "converters/uiconverter.h"

#include <bson.h>
#include <file.h>

#include <resources/uidocument.h>

#define FORMAT_VERSION 1

UiConverterSettings::UiConverterSettings() {
    setType(MetaType::type<UiDocument *>());
    setVersion(FORMAT_VERSION);
}

TString UiConverterSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/ui.svg";
}

AssetConverter::ReturnCode UiConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        UiDocument *document = Engine::loadResource<UiDocument>(settings->destination());
        if(document == nullptr) {
            document = Engine::objectCreate<UiDocument>();
        }

        TString data(src.readAll());
        src.close();
        if(!data.isEmpty()) {
            document->setData(data);
        }

        File file(settings->absoluteDestination());
        if(file.open(File::WriteOnly)) {
            file.write(Bson::save( Engine::toVariant(document) ));
            file.close();

            return Success;
        }
    }

    return InternalError;
}

AssetConverterSettings *UiConverter::createSettings() {
    return new UiConverterSettings();
}

TString UiConverter::templatePath() const {
    return ":/templates/UIDocument.ui";
}
