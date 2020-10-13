#ifndef PREFABCONVERTER_H
#define PREFABCONVERTER_H

#include "converters/converter.h"

#include "resources/prefab.h"

class PrefabConverter : public IConverter {
public:
    QStringList suffixes () const { return {"fab"}; }
    uint32_t contentType () const { return ContentPrefab; }
    uint32_t type () const { return MetaType::type<Prefab *>(); }
    uint8_t convertFile (IConverterSettings *);
    IConverterSettings *createSettings() const;

protected:
    Variant readJson(const string &data, IConverterSettings *);
    void toVersion1(Variant &variant);
};

#endif // PREFABCONVERTER_H
