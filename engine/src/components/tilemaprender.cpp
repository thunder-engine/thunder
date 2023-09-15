#include "components/tilemaprender.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/tilemap.h"
#include "resources/tileset.h"
#include "resources/sprite.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"

#include "mesh.h"

namespace {
const char *gTileMap = "TileMap";
const char *gMaterial = "Material";
}

/*!
    \class TileMapRender
    \brief The tile map renderer is used to render the tile map.
    \inmodule Engine

*/

TileMapRender::TileMapRender() :
    m_tileMap(nullptr),
    m_material(nullptr),
    m_layer(0) {

}
/*!
    \internal
*/
void TileMapRender::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(m_tileMap && m_material && layer & a->layers() && a->transform()) {
        buffer.setObjectId(a->uuid());
        buffer.setMaterialId(m_material->material()->uuid());
        buffer.setColor(Vector4(1.0f));

        buffer.drawMesh(a->transform()->worldTransform(), m_tileMap->tileMesh(), 0, layer, m_material);
    }
}
/*!
    \internal
*/
AABBox TileMapRender::localBound() const {
    if(m_tileMap) {
        return m_tileMap->tileMesh()->bound();
    }
    return Renderable::localBound();
}

TileMap *TileMapRender::tileMap() const {
    return m_tileMap;
}

void TileMapRender::setTileMap(TileMap *map) {
    m_tileMap = map;

    if(m_tileMap) {
        TileSet *tileSet = m_tileMap->tileSet();
        if(tileSet) {
            Texture *texture = tileSet->spriteSheet() ? tileSet->spriteSheet()->texture() : nullptr;
            if(m_material && texture) {
                m_material->setTexture("texture0", texture);
            }
        }
    }
}
/*!
    Returns an instantiated Material assigned to MeshRender.
*/
Material *TileMapRender::material() const {
    if(m_material) {
        return m_material->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void TileMapRender::setMaterial(Material *material) {
    if(m_material) {
        delete m_material;
        m_material = nullptr;
    }
    if(material) {
        m_material = material->createInstance();

        if(m_tileMap) {
            TileSet *tileSet = m_tileMap->tileSet();
            if(tileSet) {
                Texture *texture = tileSet->spriteSheet() ? tileSet->spriteSheet()->texture() : nullptr;
                if(m_material && texture) {
                    m_material->setTexture("texture0", texture);
                }
            }
        }
    }
}
/*!
    Returns the order layer for the tile map.
*/
int TileMapRender::layer() const {
    return m_layer;
}
/*!
    Sets the order \a layer for the tile map.
*/
void TileMapRender::setLayer(int layer) {
    m_layer = layer;
}
/*!
    \internal
*/
void TileMapRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(gMaterial);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(gTileMap);
        if(it != data.end()) {
            setTileMap(Engine::loadResource<TileMap>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap TileMapRender::saveUserData() const {
    VariantMap result = Component::saveUserData();
    {
        string ref = Engine::reference(material());
        if(!ref.empty()) {
            result[gMaterial] = ref;
        }
    }
    {
        string ref = Engine::reference(tileMap());
        if(!ref.empty()) {
            result[gTileMap] = ref;
        }
    }

    return result;
}
/*!
    \internal
*/
void TileMapRender::composeComponent() {
    //setMaterial(Engine::loadResource<Material>(".embedded/DefaultSprite.mtl"));
    setMaterial(Engine::loadResource<Material>("Materials/DefaultSprite.mtl"));
}
/*!
    \internal
*/
int TileMapRender::priority() const {
    return m_layer;
}
