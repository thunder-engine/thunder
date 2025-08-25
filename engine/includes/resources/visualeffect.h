#ifndef VISUALEFFECT_H
#define VISUALEFFECT_H

#include <resource.h>

#include <material.h>
#include <mesh.h>

class ENGINE_EXPORT VisualEffect : public Resource {
    A_OBJECT(VisualEffect, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    struct Argument {
        int32_t space;

        int32_t size;

        int32_t offset;
    };

    typedef std::vector<float> FloatData;

    struct Operator {
        int32_t op;

        int32_t resultSpace;

        int32_t resultOffset;

        int32_t resultSize;

        std::vector<Argument> arguments;

        FloatData constData;
    };

    struct Buffers {
        FloatData system;
        FloatData emitter;
        FloatData particles;

        std::vector<FloatData> render;

        int instances = 0;
    };

    struct Renderable {
        Material::SurfaceType type;

        Mesh *mesh;

        Material *material;
    };

    enum SystemAttributes {
        DeltaTime = 0
    };

    enum EmitterAttributes {
        EmitterAge = 0,
        SpawnCounter,
        Transform
    };

public:
    VisualEffect();

    void update(Buffers &buffers);

    int capacity() const;
    void setCapacity(int capacity);

    int renderablesCount();

    const Renderable *renderable(int index) const;

    bool local() const;
    void setLocal(bool local);

    bool gpu() const;
    void setGpu(bool gpu);

    bool continous() const;
    void setContinous(bool continuous);

    int systemStride() const;
    int emitterStride() const;
    int particleStride() const;

    AABBox bound() const;

    void loadUserData(const VariantMap &data) override;

protected:
    std::vector<Operator> m_emitterSpawnOperations;
    std::vector<Operator> m_emitterUpdateOperations;
    std::vector<Operator> m_particleSpawnOperations;
    std::vector<Operator> m_particleUpdateOperations;
    std::vector<Operator> m_renderOperations;

    std::vector<Renderable> m_renderables;

    AABBox m_aabb;

    int m_capacity;

    int m_systemStride;

    int m_emitterStride;

    int m_particleStride;

    bool m_gpu;

    bool m_local;

    bool m_continous;

private:
    void apply(std::vector<Operator> &operations, Buffers &buffers, int particle = 0, int render = 0, int stride = 0) const;

    void loadOperations(const VariantList &list, std::vector<Operator> &operations);

    void loadRenderables(const VariantList &list);

};

#endif // VISUALEFFECT_H
