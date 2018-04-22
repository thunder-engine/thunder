#include "components/aspritegl.h"

#include "resources/amaterialgl.h"

#include "agl.h"
#include "commandbuffergl.h"

#include <components/actor.h>

ASpriteGL::ASpriteGL() :
        Sprite() {
    m_pPlane    = Engine::loadResource<Mesh>(".embedded/plane.fbx");
}

void ASpriteGL::draw(ICommandBuffer &buffer, int8_t layer) {
    Actor &a    = actor();
    if(layer & (ICommandBuffer::RAYCAST | ICommandBuffer::DEFAULT | ICommandBuffer::TRANSLUCENT | ICommandBuffer::SHADOWCAST | ICommandBuffer::UI)) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(a.uuid());
        }
        if(m_Material) {
            m_Material->setTexture("texture0", m_Texture);
        }
        buffer.drawMesh(a.worldTransform(), m_pPlane, 0, layer, m_Material);
        buffer.setColor(Vector4(1.0f));
    }
}
