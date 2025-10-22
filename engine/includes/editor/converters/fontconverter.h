#ifndef FONTCONVERTER_H
#define FONTCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/font.h>

class FontImportSettings : public AssetConverterSettings {
public:
    FontImportSettings();

private:
    StringList typeNames() const override;

};

class FontConverter : public AssetConverter {
    void init() override;

    StringList suffixes() const override { return {"ttf", "otf"}; }

    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;
};

#endif // FONTCONVERTER_H
