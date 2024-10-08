#ifndef UICONVERTER_H
#define UICONVERTER_H

#include <editor/assetconverter.h>
#include <resources/uidocument.h>

class UiConverterSettings : public AssetConverterSettings {
public:
    UiConverterSettings();

private:
    QString defaultIcon(QString) const override;

};

class UiConverter : public AssetConverter {
    QStringList suffixes() const override { return {"ui"}; }
    ReturnCode convertFile(AssetConverterSettings *s) override;
    AssetConverterSettings *createSettings() override;
};

#endif // UICONVERTER_H
