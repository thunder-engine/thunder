#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "prefabconverter.h"

class MapConverterSettings : public AssetConverterSettings {
public:
    MapConverterSettings();
private:
    QString typeName() const Q_DECL_OVERRIDE;

    bool isReadOnly() const Q_DECL_OVERRIDE;
};

class MapConverter : public PrefabConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return { "map" }; }
    uint8_t convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE;

    bool toVersion3(Variant &variant) Q_DECL_OVERRIDE;
};

#endif // MAPCONVERTER_H
