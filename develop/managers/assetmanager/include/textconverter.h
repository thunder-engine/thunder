#ifndef TEXTCONVERTER_H
#define TEXTCONVERTER_H

#include "baseconvertersettings.h"

#include "resources/text.h"

class TextConverter : public IConverter {
public:
    TextConverter               () {}

    string                      format                      () const { return "txt;json;html;htm;xml;vert;frag"; }
    IConverter::ContentTypes    type                        () const { return IConverter::ContentText; }
    uint8_t                     convertFile                 (IConverterSettings *s);
};

#endif // TEXTCONVERTER_H
