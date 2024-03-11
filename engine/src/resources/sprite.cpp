#include "sprite.h"

#include "engine.h"
#include "texture.h"
#include "utils/atlas.h"

#include <cstring>

namespace  {
    const char *gPages = "Pages";
    const char *gShapes = "Shapes";
}

/*!
    \class Sprite
    \brief Represents 2D sprite.
    \inmodule Resources

    Sprites usually used in games to display environment and characters in 2D games.
    This class also supports sprite sheets to contain several images in one container to simplify animation or handle tile maps.
*/

Sprite::Sprite() {

}

Sprite::~Sprite() {
    PROFILE_FUNCTION();

    for(auto it : m_shapes) {
        Engine::unloadResource(it.second.first);
    }
    m_shapes.clear();
}
/*!
    Adds new sub \a texture as element to current sprite sheet.
    All elements will be packed to a single sprite sheet texture using Sprite::pack() method.
    Returns the id of the new element.
    Optionally developer is able to provide a \a name of element.
    In this case method will return a hash of provided name.

    \sa pack()
*/
int Sprite::addElement(Texture *texture) {
    PROFILE_FUNCTION();

    m_sources.push_back(texture);

    Mesh *mesh = Engine::objectCreate<Mesh>();
    mesh->makeDynamic();
    mesh->setVertices({Vector3(0.0f, 0.0f, 0.0f),
                       Vector3(0.0f, texture->height(), 0.0f),
                       Vector3(texture->width(), texture->height(), 0.0f),
                       Vector3(texture->width(), 0.0f, 0.0f) });

    mesh->setIndices({0, 1, 2, 0, 2, 3});

    int index = (m_sources.size() - 1);
    m_shapes[index].first = mesh;

    return index;
}
/*!
    Packs all added elements int to a sprite sheets.
    Parameter \a padding can be used to delimit elements.

    \sa addElement()
*/
void Sprite::packSheets(int padding) {
    PROFILE_FUNCTION();

    uint32_t atlasWidth = 1;
    uint32_t atlasHeight = 1;

    vector<AtlasNode *> nodes;
    nodes.resize(m_sources.size());

    AtlasNode root;

    if(m_pages.empty()) {
        Texture *texture = Engine::objectCreate<Texture>();
        texture->setFiltering(Texture::Bilinear);
        m_pages.push_back(texture);
    }

    while(true) {
        root.w = atlasWidth;
        root.h = atlasHeight;

        uint32_t i;
        for(i = 0; i < m_sources.size(); i++) {
            Texture *texture = m_sources[i];

            int32_t width  = (texture->width() + padding * 2);
            int32_t height = (texture->height() + padding * 2);

            AtlasNode *node = root.insert(width, height);
            if(node) {
                node->fill = true;

                nodes[i] = node;
            } else {
                atlasWidth *= 2;
                atlasHeight *= 2;

                if(root.left) {
                    delete root.left;
                    root.left = nullptr;
                }

                if(root.right) {
                    delete root.right;
                    root.right = nullptr;
                }

                break;
            }
        }

        if(i == m_sources.size()) {
            break;
        }
    }

    for(auto it : m_pages) {
        it->resize(atlasWidth, atlasHeight);
        for(uint32_t i = 0; i < nodes.size(); i++) {
            AtlasNode *node = nodes[i];

            int32_t w = node->w - padding * 2;
            int32_t h = node->h - padding * 2;

            int8_t *src = &(m_sources[i]->surface(0)[0])[0];
            int8_t *dst = &(it->surface(0)[0])[0];
            for(int32_t y = 0; y < h; y++) {
                memcpy(&dst[(y + node->y + padding) * atlasWidth + node->x], &src[y * w], w);
            }

            Mesh *mesh = shape(i);
            if(mesh) {
                Vector4 uvFrame;
                uvFrame.x = node->x / static_cast<float>(atlasWidth);
                uvFrame.y = (node->y + padding) / static_cast<float>(atlasHeight);
                uvFrame.z = uvFrame.x + w / static_cast<float>(atlasWidth);
                uvFrame.w = uvFrame.y + h / static_cast<float>(atlasHeight);

                mesh->setUv0({Vector2(uvFrame.x, uvFrame.y),
                              Vector2(uvFrame.z, uvFrame.y),
                              Vector2(uvFrame.z, uvFrame.w),
                              Vector2(uvFrame.x, uvFrame.w)});

                mesh->recalcBounds();
            }
        }

        it->setDirty();
    }
}
/*!
    \internal
*/
void Sprite::loadUserData(const VariantMap &data) {
    clear();
    {
        auto it = data.find(gPages);
        if(it != data.end()) {
            for(auto &page : it->second.toList()) {
                string ref = page.toString();
                m_pages.push_back(Engine::loadResource<Texture>(ref));
            }
        }
    }
    {
        auto it = data.find(gShapes);
        if(it != data.end()) {
            for(auto &mesh : it->second.toList()) {
                VariantList array = mesh.toList();
                auto arrayIt = array.begin();
                int32_t key = arrayIt->toInt();
                ++arrayIt;
                int32_t pageId = arrayIt->toInt();
                ++arrayIt;
                Mesh *m = Engine::loadResource<Mesh>(arrayIt->toString());
                if(m) {
                    m_shapes[key] = make_pair(m, pageId);
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

    VariantList pages;
    for(auto &it : m_pages) {
        string ref = Engine::reference(it);
        if(!ref.empty()) {
            pages.push_back(ref);
        }
    }
    if(!pages.empty()) {
        result[gPages] = pages;
    }

    VariantList shapes;
    for(auto &it : m_shapes) {
        string ref = Engine::reference(it.second.first);
        if(!ref.empty()) {
            VariantList fields;

            fields.push_back(it.first);
            fields.push_back(it.second.second);
            fields.push_back(ref);

            shapes.push_back(fields);
        }


    }
    if(!shapes.empty()) {
        result[gShapes] = shapes;
    }

    return result;
}
/*!
    Returns a mesh which represents the sprite with \a key.
*/
Mesh *Sprite::shape(int key) const {
    PROFILE_FUNCTION();

    auto it = m_shapes.find(key);
    if(it != m_shapes.end()) {
        return it->second.first;
    }
    return nullptr;
}
/*!
    Sets a new \a mesh for the sprite with \a key.
    The old mesh will be deleted and no longer available.
*/
void Sprite::setShape(int key, Mesh *mesh) {
    PROFILE_FUNCTION();

    m_shapes[key].first = mesh;
}
/*!
    Returns a sprite sheet texture.
*/
Texture *Sprite::page(int key) const {
    PROFILE_FUNCTION();

    int index = 0;
    auto it = m_shapes.find(key);
    if(it != m_shapes.end()) {
        index = it->second.second;
    }

    return (index < m_pages.size()) ? m_pages[index] : nullptr;
}
/*!
    Adds a new sprite sheet \a texture.
*/
void Sprite::addPage(Texture *texture) {
    PROFILE_FUNCTION();

    m_pages.push_back(texture);
}
/*!
    \internal
*/
void Sprite::clear() {
    PROFILE_FUNCTION();

    for(auto it : m_sources) {
        Engine::unloadResource(it);
    }
    m_sources.clear();
}
