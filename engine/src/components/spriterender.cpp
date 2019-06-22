#include "components/spriterender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "commandbuffer.h"

#define MATERIAL    "Material"
#define BASEMAP     "BaseMap"

#define OVERRIDE "uni.texture0"

class SpriteRenderPrivate {
public:
    SpriteRenderPrivate() {
        m_Texture   = nullptr;
        m_pMaterial = nullptr;
        m_pMesh     = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");
    }

    Texture *m_Texture;

    MaterialInstance *m_pMaterial;

    Mesh *m_pMesh;
};
/*!
    \class SpriteRender
    \brief Draws a sprite for the 2D graphics.
    \inmodule Engine

    The SpriteRender component allows you to display images as sprites to use in both 2D and 3D scenes.
*/

SpriteRender::SpriteRender() :
        p_ptr(new SpriteRenderPrivate) {

}

SpriteRender::~SpriteRender() {
    delete p_ptr;
}
/*!
    \internal
*/
void SpriteRender::draw(ICommandBuffer &buffer, uint32_t layer) {
    Actor *a    = actor();
    if(p_ptr->m_pMesh && layer & a->layers()) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a->uuid()));
        }

        buffer.drawMesh(a->transform()->worldTransform(), p_ptr->m_pMesh, layer, p_ptr->m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    \internal
*/
AABBox SpriteRender::bound() const {
    if(p_ptr->m_pMesh) {
        return p_ptr->m_pMesh->bound();
    }
    return Renderable::bound();
}
/*!
    Returns an instantiated Material assigned to SpriteRender.
*/
Material *SpriteRender::material() const {
    if(p_ptr->m_pMaterial) {
        return p_ptr->m_pMaterial->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void SpriteRender::setMaterial(Material *material) {
    if(p_ptr->m_pMaterial) {
        delete p_ptr->m_pMaterial;
        p_ptr->m_pMaterial = nullptr;
    }

    if(material) {
        p_ptr->m_pMaterial = material->createInstance();
    }
}
/*!
    Returns a sprite texture.
*/
Texture *SpriteRender::texture() const {
    return p_ptr->m_Texture;
}
/*!
    Replaces current \a texture with a new one.
*/
void SpriteRender::setTexture(Texture *texture) {
    p_ptr->m_Texture   = texture;
    if(p_ptr->m_pMaterial) {
        p_ptr->m_pMaterial->setTexture(OVERRIDE, p_ptr->m_Texture);
    }
}
/*!
    Returns a sprite mesh which uses to render a sprite.
*/
Mesh *SpriteRender::mesh() const {
    return p_ptr->m_pMesh;
}
/*!
    \internal
*/
void SpriteRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(MATERIAL);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(BASEMAP);
        if(it != data.end()) {
            setTexture(Engine::loadResource<Texture>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap SpriteRender::saveUserData() const {
    VariantMap result   = Component::saveUserData();
    {
        Material *m = material();
        string ref  = Engine::reference(m);
        if(!ref.empty()) {
            result[MATERIAL]    = ref;
        }
    }
    {
        Texture *t  = texture();
        string ref  = Engine::reference(t);
        if(!ref.empty()) {
            result[BASEMAP]    = ref;
        }
    }
    return result;
}
