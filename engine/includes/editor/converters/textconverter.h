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
    QStringList suffixes() const override { return {"txt", "json", "html", "htm", "xml"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;
};

#endif // TEXTCONVERTER_H
