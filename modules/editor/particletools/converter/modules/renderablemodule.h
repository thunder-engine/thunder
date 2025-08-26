#ifndef RENDREABLEMODULE_H
#define RENDREABLEMODULE_H

#include "effectmodule.h"
#include "effectrootnode.h"

class RenderableModule : public EffectModule {
    A_OBJECT(RenderableModule, EffectModule, Modificator)

    A_PROPERTIES(
        A_PROPERTYEX(Material *, material, RenderableModule::material, RenderableModule::setMaterial, "editor=Asset")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    RenderableModule();

    virtual int type() const { return -1; }

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Material *material() const;
    void setMaterial(Material *material);

protected:
    Mesh *m_mesh;

    Material *m_material;

};

#endif // RENDREABLEMODULE_H
