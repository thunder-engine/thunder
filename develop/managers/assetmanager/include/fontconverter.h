#ifndef FONTCONVERTER_H
#define FONTCONVERTER_H

#include "baseconvertersettings.h"

class FontConverter : public IConverter {
public:
    FontConverter               () {}

    string                      format                      () const { return "ttf;otf"; }
    IConverter::ContentTypes    type                        () const { return IConverter::ContentFont; }
    uint8_t                     convertFile                 (IConverterSettings *);

};

#endif // FONTCONVERTER_H
