#ifndef PREFABCONVERTER_H
#define PREFABCONVERTER_H

#include <editor/converter.h>

#include <resources/prefab.h>

class PrefabConverterSettings : public IConverterSettings {
public:
    PrefabConverterSettings();
};

class PrefabConverter : public IConverter {
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"fab"}; }

    uint8_t convertFile(IConverterSettings *) Q_DECL_OVERRIDE;
    IConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const QString &guid) const Q_DECL_OVERRIDE;

protected:
    Variant readJson(const string &data, IConverterSettings *);
    void injectResource(Variant &origin, Resource *resource);

    void toVersion1(Variant &variant);
};

#endif // PREFABCONVERTER_H
