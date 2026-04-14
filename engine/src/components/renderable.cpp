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
AABBox Renderable::bound() {
    AABBox bb = localBound();
    Transform *t = transform();
    if(t) {
        uint32_t hash = t->hash();
        if(hash != m_transformHash || m_localBox != bb) {
            m_localBox = bb;
            m_worldBox = m_localBox * t->worldTransform();
            m_transformHash = hash;
        }
    }
    return m_worldBox;
}
/*!
    Returns true if current renderable fails \a frustum culling test; otherwise returns true;
*/
bool Renderable::isCulled(const Frustum &frustum) {
    AABBox bb(bound());

    return !(bb.extent.x < 0.0f || frustum.contains(bb));
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
MaterialInstance *Renderable::materialInstance(int index) {
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
AABBox Renderable::localBound() {
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
/*!
    Filters \a out an \a in renderable components by it's material \a layer.
*/
void Renderable::filterByLayer(const RenderList &in, GroupList &out, int layer) {
    for(auto it : in) {
        for(int i = 0; i < it->materialsCount(); i++) {
            MaterialInstance *instance = it->materialInstance(i);
            if(instance && instance->material()->layers() & layer) {
                Mesh *mesh = it->meshToDraw(i);
                if(mesh) {
                    uint32_t hash = instance->hash();
                    Mathf::hashCombine(hash, mesh->uuid());

                    out.push_back({instance, mesh, it->subMesh(i), hash});
                }
            }
        }
    }

    out.sort([](const Group &left, const Group &right) {
        int p1 = left.instance->finalPriority();
        int p2 = right.instance->finalPriority();
        if(p1 == p2) {
            return left.hash < right.hash;
        }
        return p1 < p2;
    });
}
/*!
    Groups elements from \a in list into \a out rendering instances.
*/
void Renderable::group(const GroupList &in, GroupList &out) {
    Group last;

    for(auto &it : in) {
        if(last.hash != it.hash || (last.instance != nullptr && last.instance->material() != it.instance->material())) {
            if(last.instance != nullptr) {
                out.push_back(last);
            }

            last = it;
            auto &buffer = it.instance->rawUniformBuffer();
            last.buffer.insert(last.buffer.begin(), buffer.begin(), buffer.begin() + it.instance->instanceSize());
        } else {
            auto &buffer = it.instance->rawUniformBuffer();
            last.buffer.insert(last.buffer.end(), buffer.begin(), buffer.begin() + it.instance->instanceSize());
        }
    }

    // do the last insert
    if(last.instance != nullptr) {
        out.push_back(last);
    }
}
