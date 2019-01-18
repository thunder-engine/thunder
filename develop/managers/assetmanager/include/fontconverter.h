#ifndef FONTCONVERTER_H
#define FONTCONVERTER_H

#include "converters/converter.h"
#include "resources/font.h"

class FontConverter : public IConverter {
public:
    FontConverter               () {}

    QStringList suffixes() const { return {"ttf", "otf"}; }
    uint32_t                    contentType                 () const { return ContentFont; }
    uint32_t                    type                        () const { return MetaType::type<Font *>(); }
    uint8_t                     convertFile                 (IConverterSettings *);

};

#endif // FONTCONVERTER_H
