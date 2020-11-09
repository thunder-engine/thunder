#ifndef MAPCONVERTER_H
#define MAPCONVERTER_H

#include "prefabconverter.h"

class MapConverterSettings : public IConverterSettings {
public:
    MapConverterSettings();
private:
    QString typeName() const Q_DECL_OVERRIDE;
};

class MapConverter : public PrefabConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return { "map" }; }
    uint8_t convertFile(IConverterSettings *) Q_DECL_OVERRIDE;

    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;
};

#endif // MAPCONVERTER_H
