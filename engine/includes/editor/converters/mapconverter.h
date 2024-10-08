#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "prefabconverter.h"

class MapConverterSettings : public AssetConverterSettings {
public:
    MapConverterSettings();

private:
    QStringList typeNames() const override;

    bool isReadOnly() const override;

    QString defaultIcon(QString) const override;
};

class MapConverter : public PrefabConverter {
    QStringList suffixes() const override { return { "map" }; }

    AssetConverterSettings *createSettings() override;

    QString templatePath() const override;

    Resource *requestResource() override;

    bool toVersion3(Variant &variant) override;
    bool toVersion4(Variant &variant) override;
};

#endif // MAPCONVERTER_H
