#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "prefabconverter.h"

class MapConverter : public PrefabConverter {
public:
    QStringList suffixes () const { return { "map" }; }
    uint32_t contentType () const { return ContentMap; }
    uint32_t type () const { return MetaType::type<Actor *>(); }
    uint8_t convertFile (IConverterSettings *);
};

#endif // MAPCONVERTER_H
