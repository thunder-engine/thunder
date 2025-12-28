#include "converters/controlschemeconverter.h"

#include <json.h>
#include <file.h>

namespace {
    const char *gData("Data");
}

#define FORMAT_VERSION 1

ControlScehemeConverterSettings::ControlScehemeConverterSettings() {
    setVersion(FORMAT_VERSION);
}

StringList ControlScehemeConverterSettings::typeNames() const {
    return { MetaType::name<ControlScheme>() };
}

void ControlSchemeConverter::init() {
    AssetConverter::init();
}

AssetConverter::ReturnCode ControlSchemeConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        ControlScheme *scheme = Engine::loadResource<ControlScheme>(settings->destination());
        if(scheme == nullptr) {
            scheme = Engine::objectCreate<ControlScheme>(settings->destination());
        }

        uint32_t uuid = settings->info().id;
        if(uuid == 0) {
            uuid = Engine::generateUUID();
            settings->info().id = uuid;
        }

        if(scheme->uuid() != uuid) {
            Engine::replaceUUID(scheme, uuid);
        }

        VariantMap map;
        map[gData] = Json::load(src.readAll());
        scheme->loadUserData(map);

        src.close();

        return settings->saveBinary(Engine::toVariant(scheme), settings->absoluteDestination());
    }
    return InternalError;
}

AssetConverterSettings *ControlSchemeConverter::createSettings() {
    return new ControlScehemeConverterSettings();
}
