#include "components/renderable.h"

#include <amath.h>

#include "mesh.h"
#include "material.h"

#include "components/transform.h"

#include "systems/rendersystem.h"

/*!
    \class Renderable
    \brief Base class for every object which can be drawn on the screen.
    \inmodule Components

    \note This class must be a superclass only and shouldn't be created manually.
*/

Renderable::Renderable() :
        m_transformHash(0),
        m_surfaceType(Material::Static) {

}

Renderable::~Renderable() {
    static_cast<RenderSystem *>(system())->removeRenderable(this);

    for(auto it : m_materials) {
        delete it;
    }
}
/*!
    Returns a bound box of the renderable object.
*/
AABBox Renderable::bound() const {
    AABBox bb = localBound();
    Transform *t = transform();
    uint32_t hash = t->hash();
    if(hash != m_transformHash || m_localBox != bb) {
        m_localBox = bb;
        m_worldBox = m_localBox * t->worldTransform();
        m_transformHash = hash;
    }
    return m_worldBox;
}
/*!
    Returns a mesh which will be drawn for the particular material \a instance.
*/
Mesh *Renderable::meshToDraw(int instance) {
    return nullptr;
}
/*!
    Returns a sub mesh index which will be drawn for the particular material \a instance.
*/
uint32_t Renderable::subMesh(int instance) const {
    return instance;
}
/*!
    Returns a first instantiated Material assigned to this Renderable.
*/
Material *Renderable::material() const {
    if(!m_materials.empty()) {
        return m_materials.front()->material();
    }
    return nullptr;
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void Renderable::setMaterial(Material *material) {
    if(!m_materials.empty()) {
        delete m_materials[0];
        m_materials[0] = nullptr;
    }

    if(material) {
        MaterialInstance *instance = material->createInstance(static_cast<Material::SurfaceType>(m_surfaceType));
        if(m_materials.empty()) {
            m_materials.push_back(instance);
        } else {
            m_materials[0] = instance;
        }
    }
}
int32_t Renderable::materialsCount() const {
    return m_materials.size();
}
/*!
    Returns a Material instance with \a index assigned to this Renderable.
*/
MaterialInstance *Renderable::materialInstance(int index) const {
    if(m_materials.size() > index) {
        return m_materials[index];
    }
    return nullptr;
}
/*!
    Creates a new instances for the list \a materials and assigns it.
*/
void Renderable::setMaterialsList(const std::list<Material *> &materials) {
    for(auto it : m_materials) {
        delete it;
    }

    m_materials.resize(materials.size());

    uint32_t index = 0;
    for(auto it : materials) {
        if(it) {
            m_materials[index] = it->createInstance(static_cast<Material::SurfaceType>(m_surfaceType));
        }

        index++;
    }
}
/*!
    \internal
*/
AABBox Renderable::localBound() const {
    return AABBox();
}
/*!
    \internal
*/
void Renderable::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    RenderSystem *render = static_cast<RenderSystem *>(system);
    render->addRenderable(this);
}
