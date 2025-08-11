#include "converters/controlschemeconverter.h"

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

        return settings->saveBinary(Engine::toVariant(scheme));
    }
    return InternalError;
}

AssetConverterSettings *ControlSchemeConverter::createSettings() {
    return new ControlScehemeConverterSettings();
}
