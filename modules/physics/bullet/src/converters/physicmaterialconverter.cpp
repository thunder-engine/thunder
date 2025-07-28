#include "converters/physicmaterialconverter.h"

#include <QFile>

#include <json.h>
#include <bson.h>

#define DATA "Data"

#define FORMAT_VERSION 1

PhysicMaterialImportSettings::PhysicMaterialImportSettings() {
    setType(MetaType::type<PhysicMaterial *>());
    setVersion(FORMAT_VERSION);
}

TString PhysicMaterialImportSettings::defaultIconPath(const TString &) const {
    return ":/Style/styles/dark/images/fixture.svg";
}

AssetConverterSettings *PhysicMaterialConverter::createSettings() {
    return new PhysicMaterialImportSettings();
}

AssetConverter::ReturnCode PhysicMaterialConverter::convertFile(AssetConverterSettings *settings) {
    QFile src(settings->source().data());
    if(src.open(QIODevice::ReadOnly)) {
        PhysicMaterial material;
        VariantMap map = Json::load(src.readAll().toStdString()).toMap();
        src.close();

        material.setFriction(map["Friction"].toFloat());
        material.setRestitution(map["Restitution"].toFloat());
        material.setDensity(map["Density"].toFloat());

        QFile file(settings->absoluteDestination().data());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(&material));
            file.write(reinterpret_cast<const char *>(data.data()), data.size());
            file.close();
            return Success;
        }
    }

    return InternalError;
}
