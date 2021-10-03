#ifndef PREFABCONVERTER_H
#define PREFABCONVERTER_H

#include <editor/assetconverter.h>

#include <resources/prefab.h>

class PrefabConverterSettings : public AssetConverterSettings {
public:
    PrefabConverterSettings();
};

class PrefabConverter : public AssetConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"fab"}; }

    uint8_t convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const QString &guid) const Q_DECL_OVERRIDE;

protected:
    Variant readJson(const string &data, AssetConverterSettings *);
    void injectResource(Variant &origin, Resource *resource);

    void toVersion1(Variant &variant);
};

#endif // PREFABCONVERTER_H
