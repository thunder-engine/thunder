#include "sprite.h"

#include "engine.h"
#include "texture.h"

#include "atlas.h"

#include <cstring>

#define DATA    "Data"
#define MESHES  "Meshes"

typedef deque<Texture *> Textures;
typedef unordered_map<int, Mesh *> Meshes;

class SpritePrivate {
public:
    SpritePrivate() :
        m_texture(nullptr),
        m_root(new AtlasNode) {

    }
    Meshes m_meshes;

    Texture *m_texture;

    Textures m_sources;

    AtlasNode *m_root;
};

/*!
    \class Sprite
    \brief Represents 2D sprite.
    \inmodule Resources

    Sprites usually used in games to display environment and characters in 2D games.
    This class also supports sprite sheets to contain several images in one container to simplify animation or handle tile maps.
*/

Sprite::Sprite() :
        p_ptr(new SpritePrivate) {

    p_ptr->m_texture = Engine::objectCreate<Texture>();
    p_ptr->m_texture->setFiltering(Texture::Bilinear);
    resize(2048, 2048);
}

Sprite::~Sprite() {
    PROFILE_FUNCTION();

    clearAtlas();

    delete p_ptr;
    p_ptr = nullptr;
}
/*!
    \internal
*/
void Sprite::clearAtlas() {
    PROFILE_FUNCTION();

    for(auto it : p_ptr->m_meshes) {
        Engine::unloadResource(it.second);
    }
    p_ptr->m_meshes.clear();

    for(auto it : p_ptr->m_sources) {
        Engine::unloadResource(it);
    }
    p_ptr->m_sources.clear();
}
/*!
    Adds new sub \a texture as element to current sprite sheet.
    All elements will be packed to a single sprite sheet texture using Sprite::pack() method.
    Returns the id of the new element.

    \sa pack()
*/
int Sprite::addElement(Texture *texture) {
    PROFILE_FUNCTION();

    p_ptr->m_sources.push_back(texture);

    Lod lod;
    lod.setVertices({Vector3(0.0f, 0.0f, 0.0f),
                     Vector3(0.0f, 1.0f, 0.0f),
                     Vector3(1.0f, 1.0f, 0.0f),
                     Vector3(1.0f, 0.0f, 0.0f) });

    lod.setIndices({0, 1, 2, 0, 2, 3});

    Mesh *mesh = Engine::objectCreate<Mesh>();
    mesh->addLod(&lod);

    int index = (p_ptr->m_sources.size() - 1);
    p_ptr->m_meshes[index] = mesh;

    return index;
}
/*!
    Packs all added elements int to a single sprite sheet.
    Parameter \a padding can be used to delimit elements.

    \sa addElement()
*/
void Sprite::pack(int padding) {
    PROFILE_FUNCTION();

    for(size_t i = 0; i < p_ptr->m_sources.size(); i++) {
        Texture *it = p_ptr->m_sources[i];
        Mesh *m = mesh(i);
        Lod *lod = m->lod(0);

        int32_t width  = (it->width() + padding * 2);
        int32_t height = (it->height() + padding * 2);

        AtlasNode *n = p_ptr->m_root->insert(width, height);
        if(n && lod) {
            n->fill = true;
            int32_t w = n->w - padding * 2;
            int32_t h = n->h - padding * 2;

            Vector4 uv;
            uv.x = n->x / static_cast<float>(p_ptr->m_root->w);
            uv.y = (n->y + padding) / static_cast<float>(p_ptr->m_root->h);
            uv.z = uv.x + w / static_cast<float>(p_ptr->m_root->w);
            uv.w = uv.y + h / static_cast<float>(p_ptr->m_root->h);

            int8_t *src = &(it->surface(0)[0])[0];
            int8_t *dst = &(p_ptr->m_texture->surface(0)[0])[0];
            for(int32_t y = 0; y < h; y++) {
                memcpy(&dst[(y + n->y + padding) * p_ptr->m_root->w + n->x], &src[y * w], w);
            }

            lod->setUv0({Vector2(uv.x, uv.y),
                         Vector2(uv.z, uv.y),
                         Vector2(uv.z, uv.w),
                         Vector2(uv.x, uv.w)});
        } else {
            resize(p_ptr->m_root->w * 2, p_ptr->m_root->h * 2);
            pack(padding);
        }
    }

    p_ptr->m_texture->setDirty();
}

/*!
    \internal
    Changes current size of the sprite sheet and sets resorce state to ResourceState::ToBeUpdated.
*/
void Sprite::resize(int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    if(p_ptr->m_root) {
        delete p_ptr->m_root;
        p_ptr->m_root = new AtlasNode;
    }
    p_ptr->m_root->w = width;
    p_ptr->m_root->h = height;

    p_ptr->m_texture->resize(width, height);
}
/*!
    \internal
*/
void Sprite::loadUserData(const VariantMap &data) {
    clearAtlas();
    {
        auto it = data.find(DATA);
        if(it != data.end()) {
            Object *object = ObjectSystem::toObject(it->second);
            Texture *texture = dynamic_cast<Texture *>(object);
            if(texture) {
                Engine::unloadResource(p_ptr->m_texture);
                p_ptr->m_texture = texture;
            }
        }
    }
    {
        auto it = data.find(MESHES);
        if(it != data.end()) {
            for(auto &mesh : it->second.toList()) {
                VariantList array = mesh.toList();
                Object *object = ObjectSystem::toObject(array.back());
                Mesh *m = dynamic_cast<Mesh *>(object);
                if(m) {
                    int key = array.front().toInt();
                    setMesh(key, m);
                }
            }
        }
    }
}
/*!
    \internal
*/
VariantMap Sprite::saveUserData() const {
    VariantMap result;

    Variant data = ObjectSystem::toVariant(p_ptr->m_texture);
    if(data.isValid()) {
        result[DATA] = data;
    }

    if(!p_ptr->m_meshes.empty()) {
        VariantList meshes;

        for(auto it : p_ptr->m_meshes) {
            VariantList mesh;
            mesh.push_back(it.first);
            mesh.push_back(ObjectSystem::toVariant(it.second));
            meshes.push_back(mesh);
        }
        result[MESHES] = meshes;
    }

    return result;
}
/*!
    Returns a mesh which represents the sprite with \a key.
*/
Mesh *Sprite::mesh(int key) const {
    PROFILE_FUNCTION();

    auto it = p_ptr->m_meshes.find(key);
    if(it != p_ptr->m_meshes.end()) {
        return it->second;
    }
    return nullptr;
}
/*!
    Sets a new \a mesh for the sprite with \a key.
    The old mesh will be deleted and no longer available.
*/
void Sprite::setMesh(int key, Mesh *mesh) {
    PROFILE_FUNCTION();

    if(p_ptr->m_meshes[key]) {
        Engine::unloadResource(p_ptr->m_meshes[key]);
    }
    p_ptr->m_meshes[key] = mesh;
}
/*!
    Returns a sprite sheet texture.
*/
Texture *Sprite::texture() const {
    PROFILE_FUNCTION();

    return p_ptr->m_texture;
}
/*!
    Sets a new sprite sheet \a texture.
*/
void Sprite::setTexture(Texture *texture) {
    PROFILE_FUNCTION();

    if(p_ptr->m_texture) {
        Engine::unloadResource(p_ptr->m_texture);
    }
    p_ptr->m_texture = texture;
}
