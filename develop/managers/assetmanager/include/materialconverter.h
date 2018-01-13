#ifndef MATERIALCONVERTER_H
#define MATERIALCONVERTER_H

#include "baseconvertersettings.h"

class MaterialConverter : public IConverter {
public:
    MaterialConverter           () {}

    string                      format                      () const { return "mtl"; }
    IConverter::ContentTypes    type                        () const { return IConverter::ContentMaterial; }
    uint8_t                     convertFile                 (IConverterSettings *);

};

#endif // MATERIALCONVERTER_H
