#include "components/staticmesh.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"

#include "commandbuffer.h"

void StaticMesh::draw(ICommandBuffer &buffer, int8_t layer) {
    Actor &a    = actor();
    if(m_pMesh && layer & (ICommandBuffer::RAYCAST | ICommandBuffer::DEFAULT | ICommandBuffer::TRANSLUCENT | ICommandBuffer::SHADOWCAST)) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a.uuid()));
        }

        for(uint32_t s = 0; s < m_pMesh->surfacesCount(); s++) {
            MaterialInstance *material   = (s < m_Materials.size()) ? m_Materials[s] : nullptr;
            buffer.drawMesh(a.transform()->worldTransform(), m_pMesh, s, layer, material);
        }
        buffer.setColor(Vector4(1.0f));
    }
}
