#ifndef MESHMODULE_H
#define MESHMODULE_H

#include "renderablemodule.h"
#include "effectrootnode.h"

class MeshParticle : public RenderableModule {
    A_OBJECT(MeshParticle, RenderableModule, Modificator)

    A_PROPERTIES(
        A_PROPERTYEX(Mesh *, mesh, MeshParticle::mesh, MeshParticle::setMesh, "editor=Asset")
    )
    A_NOMETHODS()
    A_NOENUMS()

public:
    MeshParticle();

private:
    int type() const override { return Material::Static; }

    void setRoot(EffectRootNode *effect) override;

    void getOperations(std::vector<OperationData> &operations) const override;

};

#endif // MESHMODULE_H
