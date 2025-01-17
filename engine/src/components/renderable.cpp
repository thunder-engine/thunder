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
    Returns a mesh wich will be drawn.
*/
Mesh *Renderable::meshToDraw() const {
    return nullptr;
}
/*!
    Returns the prority value used to sort renadarble components before drawing.
    Lower values are rendered first and higher are rendered last.
*/
int Renderable::priority() const {
    return 0;
}
/*!
    Returns instance hash.
    Renerables with the same hash will be grouped together to be drawn as a single GPU draw call.
*/
uint32_t Renderable::instanceHash(int index) const {
    uint32_t result = m_materials[index]->hash();
    Mesh *mesh = meshToDraw();
    if(mesh) {
        Mathf::hashCombine(result, mesh->uuid());
    }

    return result;
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
/*!
    Returns a first Material instance assigned to this Renderable.
*/
MaterialInstance *Renderable::materialInstance() const {
    if(!m_materials.empty()) {
        return m_materials.front();
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
