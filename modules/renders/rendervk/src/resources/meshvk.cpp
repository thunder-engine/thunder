#include "resources/meshvk.h"

#include "commandbuffervk.h"

#include "rendervksystem.h"

void MeshVk::bind(VkCommandBuffer buffer, uint32_t lod) {
    switch(state()) {
        case ToBeUpdated: {
            updateVbo();

            setState(Ready);
        } break;
        case Ready: break;
        case Suspend: {
            destroyVbo();

            setState(ToBeDeleted);
            return;
        }
        default: return;
    }

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(buffer, 0, 1, &m_buffers[1], &offset);

    uint8_t flag = flags();
    if(flag & Mesh::Uv0) {
        vkCmdBindVertexBuffers(buffer, 1, 1, &m_buffers[2], &offset);
    }
    if(flag & Mesh::Normals) {
        vkCmdBindVertexBuffers(buffer, 2, 1, &m_buffers[3], &offset);
    }
    if(flag & Mesh::Tangents) {
        vkCmdBindVertexBuffers(buffer, 3, 1, &m_buffers[4], &offset);
    }
    if(flag & Mesh::Color) {
        vkCmdBindVertexBuffers(buffer, 4, 1, &m_buffers[5], &offset);
    }

    VkPrimitiveTopology t;
    Mesh::TriangleTopology mode = static_cast<Mesh::TriangleTopology>(topology());
    if(mode > Mesh::Lines) {
        t = (mode == Mesh::TriangleStrip) ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    } else {
        t = (mode == Mesh::TriangleStrip) ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        vkCmdBindIndexBuffer(buffer, m_buffers[0], 0, VK_INDEX_TYPE_UINT32);
    }

    //vkCmdSetPrimitiveTopologyEXT(buffer, t);
}

void MeshVk::updateVbo() {
    VkDevice device = RenderVkSystem::currentDevice();

    Lod *l = lod(0);
    m_buffers.resize(6);
    m_memoryBuffers.resize(6);
    {
        auto v = l->indices();
        if(!v.empty()) {
            uploadVbo(device, m_buffers[0], m_memoryBuffers[0], v.data(), sizeof(v[0]) * v.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        }
    }
    {
        auto v = l->vertices();
        if(!v.empty()) {
            uploadVbo(device, m_buffers[1], m_memoryBuffers[1], v.data(), sizeof(v[0]) * v.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        }
    }
    {
        auto v = l->uv0();
        if(!v.empty()) {
            uploadVbo(device, m_buffers[2], m_memoryBuffers[2], v.data(), sizeof(v[0]) * v.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        }
    }
    {
        auto v = l->normals();
        if(!v.empty()) {
            uploadVbo(device, m_buffers[3], m_memoryBuffers[3], v.data(), sizeof(v[0]) * v.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        }
    }
    {
        auto v = l->tangents();
        if(!v.empty()) {
            uploadVbo(device, m_buffers[4], m_memoryBuffers[4], v.data(), sizeof(v[0]) * v.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        }
    }
    {
        auto v = l->colors();
        if(!v.empty()) {
            uploadVbo(device, m_buffers[5], m_memoryBuffers[5], v.data(), sizeof(v[0]) * v.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        }
    }

}

void MeshVk::destroyVbo() {
    VkDevice device = RenderVkSystem::currentDevice();

    for(auto &it : m_buffers) {
        vkDestroyBuffer(device, it, nullptr);
    }

    for(auto &it : m_memoryBuffers) {
        vkFreeMemory(device, it, nullptr);
    }
}

void MeshVk::uploadVbo(VkDevice device, VkBuffer &buffer, VkDeviceMemory &memory, void *data, VkDeviceSize size, VkBufferUsageFlagBits flags) const {
    CommandBufferVK::createBuffer(size, flags,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  buffer, memory);

    void *dst = nullptr;
    vkMapMemory(device, memory, 0, size, 0, &dst);
        memcpy(dst, data, (size_t)size);
    vkUnmapMemory(device, memory);
}
