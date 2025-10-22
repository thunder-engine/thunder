#ifndef STYLESHEETCONVERTER_H
#define STYLESHEETCONVERTER_H

#include <editor/assetconverter.h>

class StyleSheetConverterSettings : public AssetConverterSettings {
public:
    StyleSheetConverterSettings();

private:
    StringList typeNames() const override;

};

class StyleSheetConverter : public AssetConverter {
    void init() override;

    StringList suffixes() const override { return {"css"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;

    TString templatePath() const override;
};

#endif // STYLESHEETCONVERTER_H
