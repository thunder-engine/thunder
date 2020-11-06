#ifndef FONTCONVERTER_H
#define FONTCONVERTER_H

#include <editor/converter.h>
#include <resources/font.h>

class FontConverter : public IConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"ttf", "otf"}; }
    uint8_t convertFile(IConverterSettings *) Q_DECL_OVERRIDE;
    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // FONTCONVERTER_H
