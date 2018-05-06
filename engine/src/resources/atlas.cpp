#include "atlas.h"

Atlas::Atlas() {
    m_pTexture  = Engine::objectCreate<Texture>("", this);
}

Atlas::~Atlas() {
    clear();
}

void Atlas::clear() {
    for(auto it : m_Sources) {
        delete it;
    }
    m_Sources.clear();
    m_Elements.clear();
}

uint32_t Atlas::addElement(const Texture *texture) {
    m_Sources.push_back(texture);
    return (m_Sources.size() - 1);
}

void Atlas::pack(uint8_t padding) {
    m_pTexture->resize(128, 128);
    m_Elements  = m_pTexture->pack(m_Sources, padding);
}

Vector2Vector Atlas::shape(uint32_t index) const {
    if(index < m_Sources.size()) {
        return m_Sources[index]->shape();
    }
    return Vector2Vector();
}

Vector4 Atlas::uv(uint32_t index) const {
    if(index < m_Elements.size()) {
        return m_Elements[index];
    }
    return Vector4();
}

Texture *Atlas::texture() const {
    return m_pTexture;
}
