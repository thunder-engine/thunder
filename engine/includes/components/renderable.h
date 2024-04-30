#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "nativebehaviour.h"

#include <amath.h>

class CommandBuffer;
class PipelineContext;

class Mesh;
class Material;
class MaterialInstance;

class ENGINE_EXPORT Renderable : public NativeBehaviour {
    A_REGISTER(Renderable, NativeBehaviour, General)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(AABBox, Renderable::bound)
    )

public:
    Renderable();
    ~Renderable();

    virtual AABBox bound() const;

    virtual int priority() const;

    Material *material() const;
    virtual void setMaterial(Material *material);

protected:
    virtual Mesh *meshToDraw();

    virtual AABBox localBound() const;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    virtual void setMaterialsList(const list<Material *> &materials);

private:
    void setSystem(ObjectSystem *system) override;

protected:
    friend class PipelineContext;

    vector<MaterialInstance *> m_materials;

    uint32_t m_surfaceType;

private:
    mutable AABBox m_localBox;
    mutable AABBox m_worldBox;

    mutable int32_t m_transformHash;

};

typedef list<Renderable *> RenderList;

#endif // RENDERABLE_H
