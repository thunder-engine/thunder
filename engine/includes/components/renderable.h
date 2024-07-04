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
    virtual Mesh *meshToDraw() const;

    virtual AABBox localBound() const;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    virtual void setMaterialsList(const std::list<Material *> &materials);

private:
    uint32_t instanceHash(int index) const;

    void setSystem(ObjectSystem *system) override;

protected:
    friend class PipelineContext;

    std::vector<MaterialInstance *> m_materials;

    uint32_t m_surfaceType;

private:
    mutable AABBox m_localBox;
    mutable AABBox m_worldBox;

    mutable int32_t m_transformHash;

};

typedef std::list<Renderable *> RenderList;

#endif // RENDERABLE_H
