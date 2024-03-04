#ifndef UICONVERTER_H
#define UICONVERTER_H

#include <editor/assetconverter.h>
#include <resources/uidocument.h>

class UiConverterSettings : public AssetConverterSettings {
public:
    UiConverterSettings();

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

};

class UiConverter : public AssetConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"ui"}; }
    ReturnCode convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // UICONVERTER_H
