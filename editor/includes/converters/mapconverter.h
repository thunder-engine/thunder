#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "prefabconverter.h"

class MapConverterSettings : public AssetConverterSettings {
public:
    MapConverterSettings();

private:
    StringList typeNames() const override;

    bool isReadOnly() const override;

};

class MapConverter : public PrefabConverter {
    void init() override;

    StringList suffixes() const override { return { "map" }; }

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override { return nullptr; }

    TString templatePath() const override;

    bool toVersion3(Variant &variant) override;
    bool toVersion4(Variant &variant) override;
    bool toVersion5(Variant &variant) override;
};

#endif // MAPCONVERTER_H
