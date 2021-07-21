#ifndef MESHVK_H
#define MESHVK_H

#include <list>

#include <resources/mesh.h>
#include <amath.h>

#include <vulkan/vulkan.h>

class CommandBufferVk;

class MeshVk : public Mesh {
    A_OVERRIDE(MeshVk, Mesh, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

public:
    typedef IndexVector BufferVector;

public:
    MeshVk();

    void bind(VkCommandBuffer buffer, uint32_t lod);

protected:
    void updateVbo();

    void destroyVbo();

    void buildVbo(VkDevice device, VkBuffer &buffer, VkDeviceMemory &memory, void *data, VkDeviceSize size, VkBufferUsageFlagBits flags) const;

public:
    vector<VkBuffer> m_buffers;
    vector<VkDeviceMemory> m_memoryBuffers;

};

#endif // MESHVK_H
