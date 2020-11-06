#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include <editor/converter.h>
#include <resources/text.h>

class TextConverter : public IConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"txt", "json", "html", "htm", "xml"}; }
    uint8_t convertFile(IConverterSettings *s) Q_DECL_OVERRIDE;
    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // TEXTCONVERTER_H
