#ifndef TRANSLATORCONVERTER_H
#define TRANSLATORCONVERTER_H

#include <editor/converter.h>

#include <resources/translator.h>

class TranslatorConverter : public IConverter {
public:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"loc"}; }
    uint8_t convertFile(IConverterSettings *s) Q_DECL_OVERRIDE;
    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // TRANSLATORCONVERTER_H
