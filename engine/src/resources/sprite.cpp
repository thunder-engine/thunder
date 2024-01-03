#include "sprite.h"

#include "engine.h"
#include "texture.h"

#include "atlas.h"

#include <cstring>

namespace  {
    const char *gData = "Data";
    const char *gMeshes = "Meshes";
}

static hash<string> hash_str;

/*!
    \class Sprite
    \brief Represents 2D sprite.
    \inmodule Resources

    Sprites usually used in games to display environment and characters in 2D games.
    This class also supports sprite sheets to contain several images in one container to simplify animation or handle tile maps.
*/

Sprite::Sprite() :
        m_texture(Engine::objectCreate<Texture>()),
        m_root(new AtlasNode) {

    m_texture->setFiltering(Texture::Bilinear);

    resize(2048, 2048);
}

Sprite::~Sprite() {
    PROFILE_FUNCTION();

    clearAtlas();
}
/*!
    \internal
*/
void Sprite::clearAtlas() {
    PROFILE_FUNCTION();

    for(auto it : m_meshes) {
        Engine::unloadResource(it.second);
    }
    m_meshes.clear();

    for(auto it : m_sources) {
        Engine::unloadResource(it);
    }
    m_sources.clear();
}
/*!
    Adds new sub \a texture as element to current sprite sheet.
    All elements will be packed to a single sprite sheet texture using Sprite::pack() method.
    Returns the id of the new element.
    Optionally developer is able to provide a \a name of element.
    In this case method will return a hash of provided name.

    \sa pack()
*/
int Sprite::addElement(Texture *texture, const string &name) {
    PROFILE_FUNCTION();

    m_sources.push_back(texture);

    Mesh *mesh = Engine::objectCreate<Mesh>();
    mesh->setVertices({Vector3(0.0f, 0.0f, 0.0f),
                       Vector3(0.0f, 1.0f, 0.0f),
                       Vector3(1.0f, 1.0f, 0.0f),
                       Vector3(1.0f, 0.0f, 0.0f) });

    mesh->setIndices({0, 1, 2, 0, 2, 3});

    int index = (m_sources.size() - 1);
    if(!name.empty()) {
        index = hash_str(name);
    }
    m_meshes[index] = mesh;

    return index;
}
/*!
    Packs all added elements int to a single sprite sheet.
    Parameter \a padding can be used to delimit elements.

    \sa addElement()
*/
void Sprite::pack(int padding) {
    PROFILE_FUNCTION();

    if(m_root) {
        delete m_root;
    }

    m_root = new AtlasNode;
    m_root->w = m_texture->width();
    m_root->h = m_texture->height();

    for(size_t i = 0; i < m_sources.size(); i++) {
        Texture *it = m_sources[i];
        Mesh *m = mesh(i);

        int32_t width  = (it->width() + padding * 2);
        int32_t height = (it->height() + padding * 2);

        AtlasNode *n = m_root->insert(width, height);
        if(n && m) {
            n->fill = true;
            int32_t w = n->w - padding * 2;
            int32_t h = n->h - padding * 2;

            Vector4 uv;
            uv.x = n->x / static_cast<float>(m_root->w);
            uv.y = (n->y + padding) / static_cast<float>(m_root->h);
            uv.z = uv.x + w / static_cast<float>(m_root->w);
            uv.w = uv.y + h / static_cast<float>(m_root->h);

            int8_t *src = &(it->surface(0)[0])[0];
            int8_t *dst = &(m_texture->surface(0)[0])[0];
            for(int32_t y = 0; y < h; y++) {
                memcpy(&dst[(y + n->y + padding) * m_root->w + n->x], &src[y * w], w);
            }

            m->setUv0({Vector2(uv.x, uv.y),
                       Vector2(uv.z, uv.y),
                       Vector2(uv.z, uv.w),
                       Vector2(uv.x, uv.w)});
        } else {
            resize(m_root->w * 2, m_root->h * 2);
            pack(padding);
        }
    }

    m_texture->setDirty();
}

/*!
    \internal
    Changes current size of the sprite sheet and sets resorce state to ResourceState::ToBeUpdated.
*/
void Sprite::resize(int32_t width, int32_t height) {
    PROFILE_FUNCTION();

    m_texture->resize(width, height);
}
/*!
    \internal
*/
void Sprite::loadUserData(const VariantMap &data) {
    clearAtlas();
    {
        auto it = data.find(gData);
        if(it != data.end()) {
            Object *object = ObjectSystem::toObject(it->second);
            Texture *texture = dynamic_cast<Texture *>(object);
            if(texture) {
                Engine::unloadResource(m_texture);
                m_texture = texture;
            }
        }
    }
    {
        auto it = data.find(gMeshes);
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

    Variant data = ObjectSystem::toVariant(m_texture);
    if(data.isValid()) {
        result[gData] = data;
    }

    if(!m_meshes.empty()) {
        VariantList meshes;

        for(auto it : m_meshes) {
            VariantList mesh;
            mesh.push_back(it.first);
            mesh.push_back(ObjectSystem::toVariant(it.second));
            meshes.push_back(mesh);
        }
        result[gMeshes] = meshes;
    }

    return result;
}
/*!
    Returns a mesh which represents the sprite with \a key.
*/
Mesh *Sprite::mesh(int key) const {
    PROFILE_FUNCTION();

    auto it = m_meshes.find(key);
    if(it != m_meshes.end()) {
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

    if(m_meshes[key]) {
        Engine::unloadResource(m_meshes[key]);
    }
    m_meshes[key] = mesh;
}
/*!
    Returns a sprite sheet texture.
*/
Texture *Sprite::texture() const {
    PROFILE_FUNCTION();

    return m_texture;
}
/*!
    Sets a new sprite sheet \a texture.
*/
void Sprite::setTexture(Texture *texture) {
    PROFILE_FUNCTION();

    if(m_texture) {
        Engine::unloadResource(m_texture);
    }
    m_texture = texture;
}
