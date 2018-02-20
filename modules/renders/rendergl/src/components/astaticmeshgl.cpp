#include "components/astaticmeshgl.h"

#include <atools.h>

#include "agl.h"

#include "resources/amaterialgl.h"

#include <analytics/profiler.h>
#include <components/actor.h>

void AStaticMeshGL::draw(APipeline &pipeline, int8_t layer) {
    Actor &a    = actor();
    if(m_pMesh && layer & (IRenderSystem::RAYCAST | IRenderSystem::DEFAULT | IRenderSystem::TRANSLUCENT | IRenderSystem::SHADOWCAST)) {
        if(layer & IRenderSystem::RAYCAST) {
            pipeline.setColor(pipeline.idCode(a.uuid()));
        }

        for(uint32_t s = 0; s < m_pMesh->surfacesCount(); s++) {
            MaterialInstance *material   = (s < m_Materials.size()) ? m_Materials[s] : nullptr;
            pipeline.drawMesh(a.worldTransform(), m_pMesh, s, layer, material);
        }
        pipeline.resetColor();
    }
}
