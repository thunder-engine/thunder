#ifndef PREFABCONVERTER_H
#define PREFABCONVERTER_H

#include <editor/assetconverter.h>

#include <resources/prefab.h>

class PrefabConverterSettings : public AssetConverterSettings {
public:
    PrefabConverterSettings();

private:
    StringList typeNames() const override;

    bool isReadOnly() const override;

};

class PrefabConverter : public AssetConverter {
public:
    void makePrefab(Actor *actor, AssetConverterSettings *settings);

    AssetConverterSettings *createSettings() override;

private:
    void init() override;

    StringList suffixes() const override { return {"fab"}; }

    ReturnCode convertFile(AssetConverterSettings *) override;

    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override;

    TString templatePath() const override;

    void createFromTemplate(const TString &destination) override;

protected:
    Variant readJson(const TString &data, AssetConverterSettings *);

    virtual bool toVersion1(Variant &variant);
    virtual bool toVersion2(Variant &variant);
    virtual bool toVersion3(Variant &variant);
    virtual bool toVersion4(Variant &variant);
    virtual bool toVersion5(Variant &variant);
};

#endif // PREFABCONVERTER_H
