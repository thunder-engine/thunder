#include "components/renderable.h"

#include "components/transform.h"

#include "systems/rendersystem.h"

#include "material.h"

namespace {
    const char *gMaterial = "Material";
}

/*!
    \class Renderable
    \brief Base class for every object which can be drawn on the screen.
    \inmodule Engine

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
    int32_t hash = t->hash();
    if(hash != m_transformHash || m_localBox != bb) {
        m_localBox = bb;
        m_worldBox = m_localBox * t->worldTransform();
        m_transformHash = hash;
    }
    return m_worldBox;
}
/*!
    \internal
*/
void Renderable::draw(CommandBuffer &buffer, uint32_t layer) {
    A_UNUSED(buffer);
    A_UNUSED(layer);
}
/*!
    Returns the prority value used to sort renadarble components before drawing.
    Lower values are rendered first and higher are rendered last.
*/
int Renderable::priority() const {
    return 0;
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
    Creates a new instances for the list \a material and assigns it.
*/
void Renderable::setMaterials(const list<Material *> &materials) {
    for(auto it : m_materials) {
        delete it;
    }

    m_materials.resize(materials.size());

    uint32_t index = 0;
    for(auto it : materials) {
        MaterialInstance *instance = it->createInstance(static_cast<Material::SurfaceType>(m_surfaceType));
        m_materials[index] = instance;

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
void Renderable::loadUserData(const VariantMap &data) {
    auto it = data.find(gMaterial);
    if(it != data.end()) {
        list<Material *> materials;

        for(auto &mat : (*it).second.toList()) {
            materials.push_back(Engine::loadResource<Material>(mat.toString()));
        }

        setMaterial(materials.front());
    }
}
/*!
    \internal
*/
VariantMap Renderable::saveUserData() const {
    VariantMap result(Component::saveUserData());

    VariantList materials;
    for(auto it : m_materials) {
        materials.push_back(Engine::reference(it->material()));
    }

    result[gMaterial] = materials;

    return result;
}
/*!
    \internal
*/
void Renderable::setSystem(ObjectSystem *system) {
    Object::setSystem(system);

    RenderSystem *render = static_cast<RenderSystem *>(system);
    render->addRenderable(this);
}
