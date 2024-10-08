#ifndef FONTCONVERTER_H
#define FONTCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/font.h>

class FontImportSettings : public AssetConverterSettings {
public:
    FontImportSettings();

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;
};

class FontConverter : public AssetConverter {
    QStringList suffixes() const override { return {"ttf", "otf"}; }
    ReturnCode convertFile(AssetConverterSettings *) override;
    AssetConverterSettings *createSettings() override;
};

#endif // FONTCONVERTER_H
