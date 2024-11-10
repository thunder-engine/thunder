#ifndef PARTICLEEFFECT_H
#define PARTICLEEFFECT_H

#include <resource.h>

#include <material.h>
#include <mesh.h>

class ENGINE_EXPORT ParticleEffect : public Resource {
    A_REGISTER(ParticleEffect, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()
    A_NOENUMS()

public:
    struct Operator {
        int32_t op;

        int32_t resultSpace;

        int32_t resultOffset;

        int32_t resultSize;

        int32_t mode;

        int32_t argSpace;

        int32_t argOffset;

        int32_t argSize;

        std::vector<float> argData;
    };

public:
    ParticleEffect();

    void update(std::vector<float> &emitter, std::vector<float> &particles);

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

    inline int attributeStride() const;

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

    int m_attributeStride;

    bool m_gpu;

    bool m_local;

    bool m_continous;

private:
    void apply(std::vector<Operator> &operations, std::vector<float> &emitter, std::vector<float> &particle, bool spawn) const;

    void loadOperations(const VariantList &list, std::vector<Operator> &operations);

};

#endif // PARTICLEEFFECT_H
