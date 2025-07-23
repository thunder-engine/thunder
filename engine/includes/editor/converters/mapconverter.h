#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "prefabconverter.h"

class MapConverterSettings : public AssetConverterSettings {
public:
    MapConverterSettings();

private:
    QStringList typeNames() const override;

    bool isReadOnly() const override;

    QString defaultIconPath(const QString &) const override;
};

class MapConverter : public PrefabConverter {
    StringList suffixes() const override { return { "map" }; }

    AssetConverterSettings *createSettings() override;

    TString templatePath() const override;

    bool toVersion3(Variant &variant) override;
    bool toVersion4(Variant &variant) override;
    bool toVersion5(Variant &variant) override;
};

#endif // MAPCONVERTER_H
