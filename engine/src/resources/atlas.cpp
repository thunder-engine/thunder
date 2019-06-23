#include "atlas.h"

#include "engine.h"
#include "texture.h"

class AtlasPrivate {
public:
    AtlasPrivate() {
        m_pTexture = nullptr;
    }
    Vector4Vector m_Elements;

    Texture *m_pTexture;

    Texture::Textures m_Sources;
};

Atlas::Atlas() :
        p_ptr(new AtlasPrivate) {

    p_ptr->m_pTexture  = Engine::objectCreate<Texture>("", this);
}

Atlas::~Atlas() {
    clear();

    delete p_ptr;
}

void Atlas::clear() {
    p_ptr->m_Sources.clear();
    p_ptr->m_Elements.clear();
}

uint32_t Atlas::addElement(const Texture *texture) {
    p_ptr->m_Sources.push_back(texture);
    return (p_ptr->m_Sources.size() - 1);
}

void Atlas::pack(uint8_t padding) {
    p_ptr->m_pTexture->resize(128, 128);
    p_ptr->m_Elements  = p_ptr->m_pTexture->pack(p_ptr->m_Sources, padding);
}

Vector2Vector Atlas::shape(uint32_t index) const {
    if(index < p_ptr->m_Sources.size()) {
        return p_ptr->m_Sources[index]->shape();
    }
    return Vector2Vector();
}

Vector4 Atlas::uv(uint32_t index) const {
    if(index < p_ptr->m_Elements.size()) {
        return p_ptr->m_Elements[index];
    }
    return Vector4();
}

Texture *Atlas::texture() const {
    return p_ptr->m_pTexture;
}
