#ifndef TILEMAPRENDER_H
#define TILEMAPRENDER_H

#include <renderable.h>

#include <tilemap.h>
#include <material.h>

class Mesh;
class MaterialInstance;

class ENGINE_EXPORT TileMapRender : public Renderable {
    A_REGISTER(TileMapRender, Renderable, Components/2D);

    A_PROPERTIES(
        A_PROPERTYEX(TileMap *, tileMap, TileMapRender::tileMap, TileMapRender::setTileMap, "editor=Asset"),
        A_PROPERTYEX(Material *, material, TileMapRender::material, TileMapRender::setMaterial, "editor=Asset"),
        A_PROPERTY(int, layer, TileMapRender::layer, TileMapRender::setLayer)
    )
    A_NOMETHODS()

public:
    TileMapRender();
    ~TileMapRender();

    TileMap *tileMap() const;
    void setTileMap(TileMap *map);

    void setMaterial(Material *material) override;

    int layer() const;
    void setLayer(int layer);

private:
    AABBox localBound() const override;

    void draw(CommandBuffer &buffer, uint32_t layer) override;

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    void composeComponent() override;

    int priority() const override;

private:
    TileMap *m_tileMap;

    int m_layer;

};

#endif // TILEMAPRENDER_H
