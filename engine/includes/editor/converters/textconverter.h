#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include <editor/assetconverter.h>

class TextConverterSettings : public AssetConverterSettings {
public:
    TextConverterSettings();

private:
    QString defaultIconPath(const QString &) const override;

};

class TextConverter : public AssetConverter {
    StringList suffixes() const override { return {"txt", "json", "html", "htm", "xml"}; }

    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

};

#endif // TEXTCONVERTER_H
