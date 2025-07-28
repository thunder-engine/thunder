#ifndef TRANSLATORCONVERTER_H
#define TRANSLATORCONVERTER_H

#include <editor/assetconverter.h>

#include <resources/translator.h>

class TranslatorConverterSettings : public AssetConverterSettings {
public:
    TranslatorConverterSettings();

private:
    TString defaultIconPath(const TString &) const override;
};

class TranslatorConverter : public AssetConverter {
public:
    StringList suffixes() const override { return {"loc"}; }

    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;
};

#endif // TRANSLATORCONVERTER_H
