#ifndef VISUALEFFECT_H
#define VISUALEFFECT_H

#include <resource.h>

#include <material.h>
#include <mesh.h>

class ENGINE_EXPORT VisualEffect : public Resource {
    A_REGISTER(VisualEffect, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    struct Argument {
        int32_t space;

        int32_t size;

        int32_t offset;
    };

    struct Operator {
        int32_t op;

        int32_t resultSpace;

        int32_t resultOffset;

        int32_t resultSize;

        std::vector<Argument> arguments;

        std::vector<float> constData;
    };

    enum EmitterAttributes {
        EmitterAge = 0,
        DeltaTime,
        AliveParticles,
        SpawnRate,
        SpawnCounter,
        LastAttribute
    };

public:
    VisualEffect();

    void update(std::vector<float> &emitter, std::vector<float> &particles, std::vector<float> &render);

    int capacity() const;
    void setCapacity(int capacity);

    Mesh *mesh() const;
    void setMesh(Mesh *mesh);

    Material *material() const;
    void setMaterial(Material *material);

    float spawnRate() const;
    void setSpawnRate(float rate);

    bool local() const;
    void setLocal(bool local);

    bool gpu() const;
    void setGpu(bool gpu);

    bool continous() const;
    void setContinous(bool continuous);

    inline int particleStride() const;
    inline int renderableStride() const;

    AABBox bound() const;

    void loadUserData(const VariantMap &data) override;

protected:
    std::vector<Operator> m_spawnOperations;
    std::vector<Operator> m_updateOperations;
    std::vector<Operator> m_renderOperations;

    AABBox m_aabb;

    Mesh *m_mesh;

    Material *m_material;

    float m_spawnRate;

    int m_capacity;

    int m_particleStride;

    int m_renderableStride;

    bool m_gpu;

    bool m_local;

    bool m_continous;

private:
    void apply(std::vector<Operator> &operations, std::vector<float> &emitter, std::vector<float> &particle, std::vector<float> &render, int stage) const;

    void loadOperations(const VariantList &list, std::vector<Operator> &operations);

};

#endif // VISUALEFFECT_H
