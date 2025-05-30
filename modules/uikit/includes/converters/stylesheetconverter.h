#ifndef STYLESHEETCONVERTER_H
#define STYLESHEETCONVERTER_H

#include <editor/assetconverter.h>

class StyleSheetConverterSettings : public AssetConverterSettings {
public:
    StyleSheetConverterSettings();

private:
    QString defaultIconPath(const QString &) const override;

};

class StyleSheetConverter : public AssetConverter {
    QStringList suffixes() const override { return {"css"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;

    QString templatePath() const override;
};

#endif // STYLESHEETCONVERTER_H
