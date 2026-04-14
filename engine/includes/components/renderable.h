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
    struct Group {
        MaterialInstance *instance = nullptr;

        Mesh *mesh = nullptr;

        uint32_t subMesh = 0;

        uint32_t hash = 0;

        ByteArray buffer;
    };
    typedef std::list<Group> GroupList;

    typedef std::list<Renderable *> RenderList;

public:
    Renderable();
    ~Renderable();

    virtual AABBox bound();
    virtual bool isCulled(const Frustum &frustum);

    Material *material() const;
    virtual void setMaterial(Material *material);

    int32_t materialsCount() const;
    virtual MaterialInstance *materialInstance(int index);

    static void filterByLayer(const RenderList &in, GroupList &out, int layer);

    static void group(const GroupList &in, GroupList &out);

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
