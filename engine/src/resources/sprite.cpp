#include "sprite.h"

#include "engine.h"
#include "texture.h"

#include "atlas.h"

#include <cstring>

#define HEADER  "Header"
#define DATA    "Data"

typedef deque<Texture *>  Textures;

class SpritePrivate {
public:
    SpritePrivate() :
        m_pTexture(nullptr),
        m_pRoot(new AtlasNode) {

    }
    Vector4Vector m_Elements;

    Texture *m_pTexture;

    Textures m_Sources;

    AtlasNode *m_pRoot;
};

Sprite::Sprite() :
        p_ptr(new SpritePrivate) {

    p_ptr->m_pTexture = Engine::objectCreate<Texture>("", this);
    p_ptr->m_pTexture->setFiltering(Texture::Bilinear);
    resize(1024, 1024);
}

Sprite::~Sprite() {
    PROFILE_FUNCTION();

    clearAtlas();

    delete p_ptr;
    p_ptr = nullptr;
}

void Sprite::clearAtlas() {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_Sources) {
        delete it;
    }
    p_ptr->m_Sources.clear();
    p_ptr->m_Elements.clear();

}

int Sprite::addElement(Texture *texture) {
    PROFILE_FUNCTION();

    p_ptr->m_Sources.push_back(texture);
    return (p_ptr->m_Sources.size() - 1);
}

void Sprite::pack(int padding) {
    PROFILE_FUNCTION();

    p_ptr->m_Elements.clear();
    for(auto it : p_ptr->m_Sources) {
        int32_t width  = (it->width() + padding * 2);
        int32_t height = (it->height() + padding * 2);

        AtlasNode *n = p_ptr->m_pRoot->insert(width, height);
        if(n) {
            n->fill = true;
            int32_t w = n->w - padding * 2;
            int32_t h = n->h - padding * 2;

            Vector4 res;
            res.x = n->x / static_cast<float>(p_ptr->m_pRoot->w);
            res.y = n->y / static_cast<float>(p_ptr->m_pRoot->h);
            res.z = res.x + w / static_cast<float>(p_ptr->m_pRoot->w);
            res.w = res.y + h / static_cast<float>(p_ptr->m_pRoot->h);

            int8_t *src = &(it->surface(0)[0])[0];
            int8_t *dst = &(p_ptr->m_pTexture->surface(0)[0])[0];
            for(int32_t y = padding; y < h; y++) {
                memcpy(&dst[(y + n->y) * p_ptr->m_pRoot->w + n->x], &src[y * w], w);
            }

            p_ptr->m_Elements.push_back(res);
        } else {
            resize(p_ptr->m_pRoot->w * 2, p_ptr->m_pRoot->h * 2);
            pack(padding);
        }
    }

    p_ptr->m_pTexture->setDirty();
}
/*!
    Changes current size of the atlas and sets resorce state to ResourceState::ToBeUpdated.
*/
void Sprite::resize(int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    if(p_ptr->m_pRoot) {
        delete p_ptr->m_pRoot;
        p_ptr->m_pRoot = new AtlasNode;
    }
    p_ptr->m_pRoot->w = width;
    p_ptr->m_pRoot->h = height;

    p_ptr->m_pTexture->resize(width, height);
}

void Sprite::loadUserData(const VariantMap &data) {
    {
        auto it = data.find(DATA);
        if(it != data.end()) {
            Object *object = ObjectSystem::toObject(it->second);
            Texture *texture = dynamic_cast<Texture *>(object);
            if(texture) {
                delete p_ptr->m_pTexture; // May lead to crash in case of m_pTexture had references
                p_ptr->m_pTexture = texture;
                p_ptr->m_pTexture->setParent(this);

            }
        }
    }

    setState(Ready);
}

VariantMap Sprite::saveUserData() const {
    VariantMap result;

    Variant data = ObjectSystem::toVariant(p_ptr->m_pTexture);
    if(data.isValid()) {
        result[DATA] = data;
    }

    return result;
}

Vector2Vector Sprite::shape(int index) const {
    PROFILE_FUNCTION();

    if(index < p_ptr->m_Sources.size()) {
        return p_ptr->m_Sources[index]->shape();
    }
    return Vector2Vector();
}

Vector4 Sprite::uv(int index) const {
    PROFILE_FUNCTION();

    if(index < p_ptr->m_Elements.size()) {
        return p_ptr->m_Elements[index];
    }
    return Vector4();
}

Texture *Sprite::texture() const {
    PROFILE_FUNCTION();

    return p_ptr->m_pTexture;
}

void Sprite::setTexture(Texture *texture) {
    PROFILE_FUNCTION();

    if(p_ptr->m_pTexture) {
        delete p_ptr->m_pTexture;
    }
    p_ptr->m_pTexture = texture;
    p_ptr->m_pTexture->setParent(this);

}
