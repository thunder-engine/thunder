#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include "converters/converter.h"
#include "resources/text.h"

class TextConverter : public IConverter {
public:
    QStringList suffixes() const { return {"txt", "json", "html", "htm", "xml"}; }
    uint32_t contentType() const { return ContentText; }
    uint32_t type() const { return MetaType::type<Text *>(); }
    uint8_t convertFile(IConverterSettings *s);
};

#endif // TEXTCONVERTER_H
