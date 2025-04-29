#ifndef PREFABCONVERTER_H
#define PREFABCONVERTER_H

#include <editor/assetconverter.h>

#include <resources/prefab.h>

class PrefabConverterSettings : public AssetConverterSettings {
public:
    PrefabConverterSettings();

private:
    QStringList typeNames() const override;

    bool isReadOnly() const override;

    QString defaultIconPath(const QString &) const override;
};

class PrefabConverter : public AssetConverter {
    QStringList suffixes() const override { return {"fab"}; }

    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const override;

    QString templatePath() const Q_DECL_OVERRIDE;

protected:
    Variant readJson(const std::string &data, AssetConverterSettings *);

    virtual bool toVersion1(Variant &variant);
    virtual bool toVersion2(Variant &variant);
    virtual bool toVersion3(Variant &variant);
    virtual bool toVersion4(Variant &variant);
    virtual bool toVersion5(Variant &variant);
};

#endif // PREFABCONVERTER_H
