#include "renderablemodule.h"

#include "effectgraph.h"

RenderableModule::RenderableModule() :
        m_mesh(nullptr),
        m_material(nullptr) {

    m_stage = Stage::Render;
}

Mesh *RenderableModule::mesh() const {
    return m_mesh;
}

void RenderableModule::setMesh(Mesh *mesh) {
    m_mesh = mesh;
}

Material *RenderableModule::material() const {
    return m_material;
}

void RenderableModule::setMaterial(Material *material) {
    m_material = material;
}
