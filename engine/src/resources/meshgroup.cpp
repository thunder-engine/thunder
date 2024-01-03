#include "resources/meshgroup.h"

#include "resources/mesh.h"

#include "systems/resourcesystem.h"

namespace  {
    const char *gData = "Data";
}

/*!
    \class MeshGroup
    \brief A class that allows creating or modifying meshes at the runtime.
    \inmodule Resources
*/

MeshGroup::MeshGroup() {

}

MeshGroup::~MeshGroup() {
    m_lods.clear();
}
/*!
    \internal
*/
void MeshGroup::loadUserData(const VariantMap &data) {
    m_lods.clear();

    auto meshData = data.find(gData);
    if(meshData != data.end()) {
        VariantList mesh = (*meshData).second.value<VariantList>();
        for(auto &it : mesh) {
            m_lods.push_back(Engine::loadResource<Mesh>(it.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap MeshGroup::saveUserData() const {
    VariantMap result;

    VariantList lods;
    for(size_t index = 0; index < m_lods.size(); index++) {
        lods.push_back(Engine::resourceSystem()->reference(m_lods[index]));
    }
    result[gData] = lods;

    return result;
}
/*!
    Returns the number of Levels Of Details
*/
int MeshGroup::lodsCount() const {
    return m_lods.size();
}
/*!
    Adds the new \a lod data for the MeshGroup.
    Retuns index of new lod.
*/
int MeshGroup::addLod(Mesh *lod) {
    if(lod) {
        m_lods.push_back(lod);
        return m_lods.size() - 1;
    }
    return -1;
}
/*!
    Sets the new \a data for the particular \a lod.
    This method can replace the existing data.
*/
void MeshGroup::setLod(int lod, Mesh *data) {
    if(lod < lodsCount()) {
        if(data) {
            m_lods[lod] = data;
        }
    } else {
        addLod(data);
    }
}
/*!
    Returns Mesh for the \a lod index if exists; othewise returns nullptr.
*/
Mesh *MeshGroup::lod(int lod) {
    if(lod < lodsCount()) {
        return m_lods[lod];
    }
    return nullptr;
}
