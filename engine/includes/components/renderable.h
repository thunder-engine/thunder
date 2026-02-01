#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <nativebehaviour.h>

#include <amath.h>

class CommandBuffer;
class PipelineContext;

class Mesh;
class Material;
class MaterialInstance;

class ENGINE_EXPORT Renderable : public NativeBehaviour {
    A_OBJECT(Renderable, NativeBehaviour, General)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    Renderable();
    ~Renderable();

    virtual AABBox bound();

    Material *material() const;
    virtual void setMaterial(Material *material);

    int32_t materialsCount() const;
    virtual MaterialInstance *materialInstance(int index);

protected:
    virtual Mesh *meshToDraw(int instance);

    virtual uint32_t subMesh(int instance) const;

    virtual AABBox localBound();

    virtual void setMaterialsList(const std::list<Material *> &materials);

private:
    void setSystem(ObjectSystem *system) override;

protected:
    friend class PipelineContext;
    friend class PipelineTask;

    std::vector<MaterialInstance *> m_materials;

    uint32_t m_surfaceType;

private:
    mutable AABBox m_localBox;
    mutable AABBox m_worldBox;

    mutable uint32_t m_transformHash;

};

#endif // RENDERABLE_H
