#ifndef TRANSLATORCONVERTER_H
#define TRANSLATORCONVERTER_H

#include <editor/assetconverter.h>

#include <resources/translator.h>

class TranslatorConverter : public AssetConverter {
public:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"loc"}; }
    uint8_t convertFile(AssetConverterSettings *s) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // TRANSLATORCONVERTER_H
