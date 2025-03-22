#ifndef MESHVK_H
#define MESHVK_H

#include <vulkan/vulkan.h>

#include <resources/mesh.h>

class MeshVk : public Mesh {
    A_OBJECT_OVERRIDE(MeshVk, Mesh, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    MeshVk();

    void bind(VkCommandBuffer buffer);

protected:
    void switchState(State state) override;

    void updateGpu();

    void destroyGpu();

public:
    VkBuffer m_indicesBuffer;
    VkBuffer m_verticesBuffer;

    VkDeviceMemory m_memoryIndicesBuffer;
    VkDeviceMemory m_memoryVerticesBuffer;

    size_t m_indicesSize;
    size_t m_verticesSize;
    size_t m_uvSize;
    size_t m_colorsSize;
    size_t m_normalsSize;
    size_t m_tangentsSize;
    size_t m_bonesSize;
    size_t m_weightsSize;

};

#endif // MESHVK_H
