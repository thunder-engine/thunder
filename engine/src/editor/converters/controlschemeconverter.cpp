#include "converters/controlschemeconverter.h"

#include <bson.h>
#include <json.h>
#include <file.h>

namespace {
    const char *gData("Data");
}

ControlScehemeConverterSettings::ControlScehemeConverterSettings() {
    setType(MetaType::type<ControlScheme *>());
}

AssetConverter::ReturnCode ControlSchemeConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        ControlScheme *scheme = Engine::loadResource<ControlScheme>(settings->destination());
        if(scheme == nullptr) {
            scheme = Engine::objectCreate<ControlScheme>();
        }

        VariantMap map;
        map[gData] = Json::load(src.readAll());
        scheme->loadUserData(map);

        src.close();

        File file(settings->absoluteDestination());
        if(file.open(File::WriteOnly)) {
            file.write(Bson::save(Engine::toVariant(scheme)));
            file.close();

            return Success;
        }
    }
    return InternalError;
}

AssetConverterSettings *ControlSchemeConverter::createSettings() {
    return new ControlScehemeConverterSettings();
}
