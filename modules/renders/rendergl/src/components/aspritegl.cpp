#include "components/aspritegl.h"

#include "resources/amaterialgl.h"

#include "agl.h"

#include <components/actor.h>

#define OVERRIDE "texture0"

ASpriteGL::ASpriteGL() :
        Sprite() {

    glGenVertexArrays(1, &m_VAO);
}

void ASpriteGL::draw(APipeline &pipeline, int8_t layer) {
    Actor &a    = actor();
    if(layer & (RAYCAST | DEFAULT | TRANSLUCENT | SHADOWCAST | UI)) {
        if(layer & IDrawObjectGL::RAYCAST) {
            pipeline.setColor(pipeline.idCode(a.uuid()));
        }
        AMaterialGL *material   = dynamic_cast<AMaterialGL *>(m_Material);
        if(material) {
            Texture *t  = material->texture(OVERRIDE);
            material->overrideTexture(OVERRIDE, m_Texture);
            //glBindVertexArray(m_VAO);
            if(material->bind(pipeline, layer, AMaterialGL::Static)) {
                pipeline.drawQuad();
                material->unbind(layer);
            }
            //glBindVertexArray(0);
            material->overrideTexture(OVERRIDE, t);
        }
    }
}
