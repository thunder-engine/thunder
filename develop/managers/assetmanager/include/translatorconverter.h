#ifndef TRANSLATORCONVERTER_H
#define TRANSLATORCONVERTER_H

#include "converters/converter.h"
#include "resources/translator.h"

class TranslatorConverter : public IConverter {
public:
    QStringList suffixes() const { return {"loc"}; }
    uint32_t contentType() const { return ContentLocalization; }
    uint32_t type() const { return MetaType::type<Translator *>(); }
    uint8_t convertFile(IConverterSettings *s);
};

#endif // TRANSLATORCONVERTER_H
