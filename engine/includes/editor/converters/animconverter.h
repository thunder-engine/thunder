#ifndef ANIMCONVERTER_H
#define ANIMCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/animationclip.h>

class AnimImportSettings : public AssetConverterSettings {
public:
    AnimImportSettings();

private:
    bool isReadOnly() const override;

    QString defaultIconPath(const QString &) const override;
};

class AnimConverter : public AssetConverter {
    StringList suffixes() const override { return {"anim"}; }

    ReturnCode convertFile(AssetConverterSettings *s) override;

    AssetConverterSettings *createSettings() override;

    TString templatePath() const override { return ":/Templates/Animation.anim"; }

private:
    Variant readJson(const std::string &data, AssetConverterSettings *settings);
    void toVersion1(Variant &variant);
    void toVersion3(Variant &variant);
};

#endif // ANIMCONVERTER_H
