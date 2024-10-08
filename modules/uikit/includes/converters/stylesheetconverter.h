#ifndef STYLESHEETCONVERTER_H
#define STYLESHEETCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/stylesheet.h>

class StyleSheetConverterSettings : public AssetConverterSettings {
public:
    StyleSheetConverterSettings();

private:
    QString defaultIcon(QString) const override;

};

class StyleSheetConverter : public AssetConverter {
    QStringList suffixes() const override { return {"css"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;
};

#endif // STYLESHEETCONVERTER_H
