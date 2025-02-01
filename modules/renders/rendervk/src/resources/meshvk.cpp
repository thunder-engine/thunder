#include "resources/meshvk.h"

#include <cstring>

#include "commandbuffervk.h"

#include "wrappervk.h"

MeshVk::MeshVk() :
        m_indicesBuffer(VK_NULL_HANDLE),
        m_verticesBuffer(VK_NULL_HANDLE),
        m_memoryIndicesBuffer(VK_NULL_HANDLE),
        m_memoryVerticesBuffer(VK_NULL_HANDLE),
        m_indicesSize(0),
        m_verticesSize(0),
        m_uvSize(0),
        m_colorsSize(0),
        m_normalsSize(0),
        m_tangentsSize(0),
        m_bonesSize(0),
        m_weightsSize(0) {

}

void MeshVk::bind(VkCommandBuffer buffer) {
    switch(state()) {
        case ToBeUpdated: {
            updateGpu();

            setState(Ready);
        } break;
        case Unloading: {
            destroyGpu();

            switchState(ToBeDeleted);
            return;
        }
        default: break;
    }

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(buffer, 0, 1, &m_verticesBuffer, &offset);
    offset += m_verticesSize;

    if(m_uvSize > 0) {
        vkCmdBindVertexBuffers(buffer, 1, 1, &m_verticesBuffer, &offset);
        offset += m_uvSize;
    }
    if(m_colorsSize > 0) {
        vkCmdBindVertexBuffers(buffer, 2, 1, &m_verticesBuffer, &offset);
        offset += m_colorsSize;
    }
    if(m_normalsSize > 0) {
        vkCmdBindVertexBuffers(buffer, 3, 1, &m_verticesBuffer, &offset);
        offset += m_normalsSize;
    }
    if(m_tangentsSize > 0) {
        vkCmdBindVertexBuffers(buffer, 4, 1, &m_verticesBuffer, &offset);
        offset += m_tangentsSize;
    }
    if(m_weightsSize > 0) {
        vkCmdBindVertexBuffers(buffer, 5, 1, &m_verticesBuffer, &offset);
        offset += m_weightsSize;
    }
    if(m_bonesSize > 0) {
        vkCmdBindVertexBuffers(buffer, 6, 1, &m_verticesBuffer, &offset);
    }

    vkCmdBindIndexBuffer(buffer, m_indicesBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void MeshVk::switchState(State state) {
    switch(state) {
        case Unloading: {
            destroyGpu();
        } break;
        default: Mesh::switchState(state); break;
    }
}

void MeshVk::updateGpu() {
    VkDevice device = WrapperVk::device();

    {
        auto v = indices();
        if(!v.empty()) {
            size_t size = sizeof(v[0]) * v.size();

            if(size > m_indicesSize) {
                WrapperVk::destroyBuffer(m_indicesBuffer);
                WrapperVk::freeMemory(m_memoryIndicesBuffer);

                WrapperVk::createBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, m_indicesBuffer);
                WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_indicesBuffer, m_memoryIndicesBuffer);
            }

            m_indicesSize = size;

            void *dst = nullptr;
            vkMapMemory(device, m_memoryIndicesBuffer, 0, m_indicesSize, 0, &dst);
                memcpy(dst, v.data(), m_indicesSize);
            vkUnmapMemory(device, m_memoryIndicesBuffer);
        }
    }

    if(!vertices().empty()) {
        uint32_t vCount = vertices().size();
        uint32_t size = sizeof(Vector3) * vCount;

        if(!uv0().empty()) size += sizeof(Vector2) * vCount;
        if(!colors().empty()) size += sizeof(Vector4) * vCount;
        if(!normals().empty()) size += sizeof(Vector3) * vCount;
        if(!tangents().empty()) size += sizeof(Vector3) * vCount;
        if(!weights().empty()) size += sizeof(Vector4) * vCount;
        if(!bones().empty()) size += sizeof(Vector4) * vCount;

        if(size > (m_verticesSize + m_uvSize + m_colorsSize +
                   m_normalsSize + m_tangentsSize + m_weightsSize + m_bonesSize)) {

            WrapperVk::destroyBuffer(m_verticesBuffer);
            WrapperVk::freeMemory(m_memoryVerticesBuffer);

            WrapperVk::createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, m_verticesBuffer);
            WrapperVk::allocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_verticesBuffer, m_memoryVerticesBuffer);
        }

        uint8_t *dst = nullptr;
        vkMapMemory(device, m_memoryVerticesBuffer, 0, size, 0, reinterpret_cast<void **>(&dst));
            m_verticesSize = sizeof(Vector3) * vCount;
            memcpy(dst, vertices().data(), m_verticesSize);
            dst += m_verticesSize;

            if(!uv0().empty()) {
                m_uvSize = sizeof(Vector2) * vCount;
                memcpy(dst, uv0().data(), m_uvSize);
                dst += m_uvSize;
            }
            if(!colors().empty()) {
                m_colorsSize = sizeof(Vector4) * vCount;
                memcpy(dst, colors().data(), m_colorsSize);
                dst += m_colorsSize;
            }
            if(!normals().empty()) {
                m_normalsSize = sizeof(Vector3) * vCount;
                memcpy(dst, normals().data(), m_normalsSize);
                dst += m_normalsSize;
            }
            if(!tangents().empty()) {
                m_tangentsSize = sizeof(Vector3) * vCount;
                memcpy(dst, tangents().data(), m_tangentsSize);
                dst += m_tangentsSize;
            }
            if(!weights().empty()) {
                m_weightsSize = sizeof(Vector4) * vCount;
                memcpy(dst, weights().data(), m_weightsSize);
                dst += m_weightsSize;
            }
            if(!bones().empty()) {
                m_bonesSize = sizeof(Vector4) * vCount;
                memcpy(dst, bones().data(), m_bonesSize);
            }
        vkUnmapMemory(device, m_memoryVerticesBuffer);
    }
}

void MeshVk::destroyGpu() {
    WrapperVk::destroyBuffer(m_indicesBuffer);
    WrapperVk::destroyBuffer(m_verticesBuffer);

    m_indicesBuffer = nullptr;
    m_verticesBuffer = nullptr;

    WrapperVk::freeMemory(m_memoryIndicesBuffer);
    WrapperVk::freeMemory(m_memoryVerticesBuffer);

    m_memoryIndicesBuffer = nullptr;
    m_memoryVerticesBuffer = nullptr;

    m_indicesSize = 0;
    m_verticesSize = 0;
    m_uvSize = 0;
    m_colorsSize = 0;
    m_normalsSize = 0;
    m_tangentsSize = 0;
    m_weightsSize = 0;
    m_bonesSize = 0;
}
