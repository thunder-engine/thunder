#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include <editor/assetconverter.h>
#include <resources/text.h>

class TextConverterSettings : public AssetConverterSettings {
public:
    TextConverterSettings();

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class TextConverter : public AssetConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"txt", "json", "html", "htm", "xml"}; }
    ReturnCode convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // TEXTCONVERTER_H
