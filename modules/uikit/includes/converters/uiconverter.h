#ifndef UICONVERTER_H
#define UICONVERTER_H

#include <editor/assetconverter.h>

class UiConverterSettings : public AssetConverterSettings {
public:
    UiConverterSettings();

private:
    QString defaultIconPath(const QString &) const override;

};

class UiConverter : public AssetConverter {
    QStringList suffixes() const override { return {"ui"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;

    QString templatePath() const override;
};

#endif // UICONVERTER_H
