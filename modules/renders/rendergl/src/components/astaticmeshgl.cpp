#include "components/astaticmeshgl.h"

#include <tools.h>

#include "agl.h"
#include "commandbuffergl.h"

#include "resources/amaterialgl.h"

#include <analytics/profiler.h>
#include <components/actor.h>

void AStaticMeshGL::draw(ICommandBuffer &buffer, int8_t layer) {
    Actor &a    = actor();
    if(m_pMesh && layer & (ICommandBuffer::RAYCAST | ICommandBuffer::DEFAULT | ICommandBuffer::TRANSLUCENT | ICommandBuffer::SHADOWCAST)) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(a.uuid());
        }

        for(uint32_t s = 0; s < m_pMesh->surfacesCount(); s++) {
            MaterialInstance *material   = (s < m_Materials.size()) ? m_Materials[s] : nullptr;
            buffer.drawMesh(a.worldTransform(), m_pMesh, s, layer, material);
        }
        buffer.setColor(Vector4(1.0f));
    }
}
