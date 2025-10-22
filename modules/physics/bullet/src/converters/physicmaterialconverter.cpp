#include "converters/physicmaterialconverter.h"

#include <json.h>

#include "resources/physicmaterial.h"

#define FORMAT_VERSION 1

PhysicMaterialImportSettings::PhysicMaterialImportSettings() {
    setVersion(FORMAT_VERSION);
}

StringList PhysicMaterialImportSettings::typeNames() const {
    return { MetaType::name<PhysicMaterial>() };
}

AssetConverterSettings *PhysicMaterialConverter::createSettings() {
    return new PhysicMaterialImportSettings();
}

void PhysicMaterialConverter::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/fixture.svg");
    }
}

AssetConverter::ReturnCode PhysicMaterialConverter::convertFile(AssetConverterSettings *settings) {
    File src(settings->source());
    if(src.open(File::ReadOnly)) {
        PhysicMaterial *material = Engine::loadResource<PhysicMaterial>(settings->destination());
        if(material == nullptr) {
            material = Engine::objectCreate<PhysicMaterial>(settings->destination());
        }

        VariantMap map = Json::load(src.readAll()).toMap();
        src.close();

        material->setFriction(map["Friction"].toFloat());
        material->setRestitution(map["Restitution"].toFloat());
        material->setDensity(map["Density"].toFloat());

        settings->info().id = material->uuid();

        return settings->saveBinary(Engine::toVariant(material), settings->absoluteDestination());
    }

    return InternalError;
}
