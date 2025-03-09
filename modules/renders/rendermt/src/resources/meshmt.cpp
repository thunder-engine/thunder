#include "resources/meshmt.h"

#include "commandbuffermt.h"

MeshMt::MeshMt() :
        m_vertexBuffer(nullptr),
        m_indexBuffer(nullptr),
        m_vertexSize(0),
        m_uvSize(0),
        m_normalsSize(0),
        m_tangentsSize(0),
        m_bonesSize(0),
        m_weightsSize(0) {

}

MTL::Buffer *MeshMt::indexBuffer() {
    return m_indexBuffer;
}

void MeshMt::bind(MTL::RenderCommandEncoder *encoder, int uniformOffset) {
    switch(state()) {
        case ToBeUpdated: {
            update();

            switchState(Ready);
        } break;
        case Unloading: {
            if(m_vertexBuffer != nullptr) {
                m_vertexBuffer->release();
            }
            m_vertexBuffer = nullptr;

            if(m_indexBuffer != nullptr) {
                m_indexBuffer->release();
            }
            m_indexBuffer = nullptr;

            switchState(ToBeDeleted);
            return;
        }
        default: break;
    }

    int index = uniformOffset;
    size_t offset = 0;
    encoder->setVertexBuffer(m_vertexBuffer, offset, index++);
    offset += m_vertexSize;

    if(m_uvSize > 0) {
        encoder->setVertexBuffer(m_vertexBuffer, offset, index++);
        offset += m_uvSize;
    }

    if(m_colorSize > 0) {
        encoder->setVertexBuffer(m_vertexBuffer, offset, index++);
        offset += m_colorSize;
    }

    if(m_normalsSize > 0) {
        encoder->setVertexBuffer(m_vertexBuffer, offset, index++);
        offset += m_normalsSize;
    }

    if(m_tangentsSize > 0) {
        encoder->setVertexBuffer(m_vertexBuffer, offset, index++);
        offset += m_tangentsSize;
    }

    if(m_bonesSize > 0) {
        encoder->setVertexBuffer(m_vertexBuffer, offset, index++);
        offset += m_bonesSize;
    }

    if(m_weightsSize > 0) {
        encoder->setVertexBuffer(m_vertexBuffer, offset, index);
    }
}

void MeshMt::update() {
    if(!indices().empty()) {
        if(m_indexBuffer != nullptr) {
            m_indexBuffer->release();
        }

        size_t size = sizeof(uint32_t) * indices().size();

        m_indexBuffer = WrapperMt::device()->newBuffer(size, MTL::ResourceStorageModeManaged);
        memcpy(m_indexBuffer->contents(), indices().data(), size);

        m_indexBuffer->didModifyRange(NS::Range::Make(0, size));
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

        if(size > (m_vertexSize + m_uvSize + m_colorSize +
                    m_normalsSize + m_tangentsSize + m_weightsSize + m_bonesSize)) {

            if(m_vertexBuffer) {
                m_vertexBuffer->release();
            }

            m_vertexBuffer = WrapperMt::device()->newBuffer(size, MTL::ResourceStorageModeManaged);
        }

        uint8_t *ptr = reinterpret_cast<uint8_t *>(m_vertexBuffer->contents());

        size_t offset = 0;
        m_vertexSize = sizeof(Vector3) * vCount;
        memcpy(ptr, vertices().data(), m_vertexSize);
        offset += m_vertexSize;

        if(!uv0().empty()) {
            m_uvSize = sizeof(Vector2) * vCount;
            memcpy(&ptr[offset], uv0().data(), m_uvSize);
            offset += m_uvSize;
        }
        if(!colors().empty()) {
            m_colorSize = sizeof(Vector4) * vCount;
            memcpy(&ptr[offset], colors().data(), m_colorSize);
            offset += m_colorSize;
        }
        if(!normals().empty()) {
            m_normalsSize = sizeof(Vector3) * vCount;
            memcpy(&ptr[offset], normals().data(), m_normalsSize);
            offset += m_normalsSize;
        }
        if(!tangents().empty()) {
            m_tangentsSize = sizeof(Vector3) * vCount;
            memcpy(&ptr[offset], tangents().data(), m_tangentsSize);
            offset += m_tangentsSize;
        }
        if(!bones().empty()) {
            m_bonesSize = sizeof(Vector4) * vCount;
            memcpy(&ptr[offset], bones().data(), m_bonesSize);
            offset += m_bonesSize;
        }
        if(!weights().empty()) {
            m_weightsSize = sizeof(Vector4) * vCount;
            memcpy(&ptr[offset], weights().data(), m_weightsSize);
        }

        m_vertexBuffer->didModifyRange(NS::Range::Make(0, size));
    }
}
