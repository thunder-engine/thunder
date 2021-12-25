#ifndef MEDIA_H
#define MEDIA_H

#include <module.h>

class Media : public Module {
public:
    Media(Engine *engine);

    ~Media();

    const char *metaInfo() const override;

    System *system(const char *name) override;
#ifdef NEXT_SHARED
    AssetConverter *assetConverter(const char *name) override;
#endif
protected:
    System *m_pSystem;
};
#ifdef NEXT_SHARED
extern "C" {
    MODULE_EXPORT Module *moduleCreate(Engine *engine);
}
#endif
#endif // MEDIA_H
