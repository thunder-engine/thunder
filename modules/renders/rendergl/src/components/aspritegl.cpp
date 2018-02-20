#include "components/aspritegl.h"

#include "resources/amaterialgl.h"

#include "agl.h"

#include <components/actor.h>

ASpriteGL::ASpriteGL() :
        Sprite() {

}

void ASpriteGL::draw(APipeline &pipeline, int8_t layer) {
    Actor &a    = actor();
    if(layer & (IRenderSystem::RAYCAST | IRenderSystem::DEFAULT | IRenderSystem::TRANSLUCENT | IRenderSystem::SHADOWCAST | IRenderSystem::UI)) {
        if(layer & IRenderSystem::RAYCAST) {
            pipeline.setColor(pipeline.idCode(a.uuid()));
        }
        pipeline.drawQuad(a.worldTransform(), layer, m_Material, m_Texture);
        pipeline.resetColor();
    }
}
