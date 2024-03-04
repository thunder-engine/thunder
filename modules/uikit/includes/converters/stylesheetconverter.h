#ifndef STYLESHEETCONVERTER_H
#define STYLESHEETCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/stylesheet.h>

class StyleSheetConverterSettings : public AssetConverterSettings {
public:
    StyleSheetConverterSettings();

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class StyleSheetConverter : public AssetConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"css"}; }
    ReturnCode convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // STYLESHEETCONVERTER_H
