#ifndef MATERIALCONVERTER_H
#define MATERIALCONVERTER_H

#include "converters/converter.h"
#include "resources/material.h"

class MaterialConverter : public IConverter {
public:
    MaterialConverter           () {}

    string                      format                      () const { return "mtl"; }
    uint32_t                    contentType                 () const { return ContentMaterial; }
    uint32_t                    type                        () const { return MetaType::type<Material *>(); }
    uint8_t                     convertFile                 (IConverterSettings *);

};

#endif // MATERIALCONVERTER_H
