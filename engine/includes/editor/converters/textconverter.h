#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include <editor/assetconverter.h>

class TextConverterSettings : public AssetConverterSettings {
public:
    TextConverterSettings();

private:
    TString defaultIconPath(const TString &) const override;

    StringList typeNames() const override;

};

class TextConverter : public AssetConverter {
    StringList suffixes() const override { return {"txt", "json", "html", "htm", "xml"}; }

    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

};

#endif // TEXTCONVERTER_H
