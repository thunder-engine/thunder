#include "resources/mesh.h"

#include "resources/material.h"

#include "systems/resourcesystem.h"

#include <cstring>
#include <cfloat>

namespace  {
    const char *gData = "Data";
}

enum MeshAttributes {
    Color    = (1<<0),
    Uv0      = (1<<1),
    Uv1      = (1<<2),
    Normals  = (1<<3),
    Tangents = (1<<4),
    Skinned  = (1<<5),
};

/*!
    \class Mesh
    \brief This class contains all necessary data for the Mesh.
    \inmodule Resources
*/

Mesh::Mesh() :
        m_dynamic(false)  {

}

bool Mesh::operator== (const Mesh &right) const {
    return (m_indices == right.m_indices) &&
           (m_colors == right.m_colors) &&
           (m_weights == right.m_weights) &&
           (m_bones == right.m_bones) &&
           (m_vertices == right.m_vertices) &&
           (m_normals == right.m_normals) &&
           (m_tangents == right.m_tangents) &&
           (m_uv0 == right.m_uv0) &&
           (m_uv1 == right.m_uv1);
}

/*!
    Returns true in case of mesh can by changed at the runtime; otherwise returns false.
*/
bool Mesh::isDynamic() const {
    return m_dynamic;
}
/*!
    Marks mesh as dynamic that means it's can be changed at the runtime.
*/
void Mesh::makeDynamic() {
    m_dynamic = true;
}
/*!
    Returns false if mesh structure is empty; otherwise returns true.
*/
bool Mesh::isEmpty() const {
    return m_indices.empty() || m_vertices.empty();
}
/*!
    Removes all mesh data.
*/
void Mesh::clear() {
    m_indices.clear();
    m_offsets.clear();
    m_vertices.clear();
    m_uv0.clear();
    m_uv1.clear();
    m_colors.clear();
    m_normals.clear();
    m_tangents.clear();
    m_bones.clear();
    m_weights.clear();
}
/*!
    Returns a default material for the \a sub mesh.
*/
Material *Mesh::defaultMaterial(int sub) const {
    if(sub < m_defaultMaterials.size()) {
        return m_defaultMaterials[sub];
    }
    return nullptr;
}
/*!
    Sets a default \a material for the \a sub mesh.
*/
void Mesh::setDefaultMaterial(Material *material, int sub) {
    if(sub < m_defaultMaterials.size()) {
        m_defaultMaterials[sub] = material;
    } else {
        m_defaultMaterials.push_back(material);
    }
}
/*!
    Returns an array of mesh indices for the particular Mesh.
*/
IndexVector &Mesh::indices() {
    return m_indices;
}
/*!
    Sets an array of mesh \a indices for the particular Mesh.
*/
void Mesh::setIndices(const IndexVector &indices) {
    m_indices = indices;
}
/*!
    Returns an array of colors for vertices for the particular Mesh.
*/
Vector4Vector &Mesh::colors() {
    return m_colors;
}
/*!
    Sets an array of \a colors for vertices for the particular Mesh.
*/
void Mesh::setColors(const Vector4Vector &colors) {
    m_colors = colors;
}
/*!
    Returns an array of bone weights for the particular Mesh.
*/
Vector4Vector &Mesh::weights() {
    return m_weights;
}
/*!
    Sets an array of bone \a weights for the particular Lod.
*/
void Mesh::setWeights(const Vector4Vector &weights) {
    m_weights = weights;
}
/*!
    Returns an array of bones for vertices for the particular Lod.
*/
Vector4Vector &Mesh::bones() {
    return m_bones;
}
/*!
    Sets an array of \a bones for vertices for the particular Lod.
*/
void Mesh::setBones(const Vector4Vector &bones) {
    m_bones = bones;
}
/*!
    Returns an array of mesh vertices for the particular Lod.
*/
Vector3Vector &Mesh::vertices() {
    return m_vertices;
}
/*!
    Sets an array of mesh \a vertices for the particular Lod.
*/
void Mesh::setVertices(const Vector3Vector &vertices) {
    m_vertices = vertices;
}
/*!
    Returns an array of mesh normals for the particular Lod.
*/
Vector3Vector &Mesh::normals() {
    return m_normals;
}
/*!
    Sets an array of mesh \a normals for the particular Lod.
*/
void Mesh::setNormals(const Vector3Vector &normals) {
    m_normals = normals;
}
/*!
    Returns an array of mesh tangents for the particular Lod.
*/
Vector3Vector &Mesh::tangents() {
    return m_tangents;
}
/*!
    Sets an array of mesh \a tangents for the particular Lod.
*/
void Mesh::setTangents(const Vector3Vector &tangents) {
    m_tangents = tangents;
}
/*!
    Returns an array of mesh uv0 (base) texture coordinates for the particular Lod.
*/
Vector2Vector &Mesh::uv0() {
    return m_uv0;
}
/*!
    Sets an array of mesh \a uv0 (base) texture coordinates for the particular Lod.
*/
void Mesh::setUv0(const Vector2Vector &uv0) {
    m_uv0 = uv0;
}
/*!
    Returns an array of mesh uv1 texture coordinates for the particular Lod.
*/
Vector2Vector &Mesh::uv1() {
    return m_uv1;
}
/*!
    Sets an array of mesh \a uv1 texture coordinates for the particular Lod.
*/
void Mesh::setUv1(const Vector2Vector &uv1) {
    m_uv1 = uv1;
}
/*!
    Returns bounding box for the Mesh.
*/
AABBox Mesh::bound() const {
    return m_box;
}
/*!
    Sets new bounding \a box for the Mesh.
*/
void Mesh::setBound(const AABBox &box) {
    m_box = box;
    switchState(ToBeUpdated);
}
/*!
    Returns the number of sub-meshes inside the Mesh.
*/
int Mesh::subMeshCount() const {
    return m_offsets.size();
}
/*!
    Sets a base vertex \a offset for the \a sub mesh.
*/
void Mesh::setSubMesh(int offset, int sub) {
    if(sub < m_offsets.size()) {
        m_offsets[sub] = offset;
        return;
    }
    m_offsets.push_back(offset);
}
/*!
    Returns starting point index for the \a sub mesh.
*/
int Mesh::indexStart(int sub) const {
    if(sub < m_offsets.size()) {
        return m_offsets[sub];
    }
    return 0;
}
/*!
    Returns index count for the \a sub mesh.
*/
int Mesh::indexCount(int sub) const {
    if(sub < static_cast<int32_t>(m_offsets.size()) - 1) {
        return m_offsets[sub+1] - m_offsets[sub];
    }
    return m_indices.size() - m_offsets[sub];
}
/*!
    Recalculates the normals of the Mesh from the triangles and vertices.
*/
void Mesh::recalcNormals() {
    m_normals.clear();
    m_normals.resize(m_vertices.size());
    for(uint32_t i = 0; i < m_indices.size(); i += 3) {
        uint32_t index1 = m_indices[i];
        uint32_t index2 = m_indices[i + 1];
        uint32_t index3 = m_indices[i + 2];

        Vector3 n = (m_vertices[index2] - m_vertices[index1]).cross(m_vertices[index3] - m_vertices[index1]);
        n.normalize();
        m_normals[index1] += n;
        m_normals[index2] += n;
        m_normals[index3] += n;
    }
    for(auto it : m_normals) {
        it.normalize();
    }

    switchState(ToBeUpdated);
}
/*!
    Generates bound box according new geometry.
*/
void Mesh::recalcBounds() {
    Vector3 min( FLT_MAX);
    Vector3 max(-FLT_MAX);

    for(uint32_t i = 0; i < m_vertices.size(); i++) {
        min.x = MIN(min.x, m_vertices[i].x);
        min.y = MIN(min.y, m_vertices[i].y);
        min.z = MIN(min.z, m_vertices[i].z);

        max.x = MAX(max.x, m_vertices[i].x);
        max.y = MAX(max.y, m_vertices[i].y);
        max.z = MAX(max.z, m_vertices[i].z);
    }

    m_box.setBox(min, max);

    if(m_offsets.empty()) {
        m_offsets.push_back(0);
    }

    switchState(ToBeUpdated);
}
/*!
    Merges current with provided \a mesh.
    In the case of the \a transform, the matrix is not nullptr it will be applied to \a mesh before merging.
*/
void Mesh::batchMesh(Mesh &mesh, const Matrix4 *transform) {
    auto vertexVector = mesh.vertices();
    auto normalVector = mesh.normals();
    auto tangentVector = mesh.tangents();
    if(transform) {
        for(auto &v : vertexVector) {
            v = *transform * v;
        }

        Matrix3 rotation = transform->rotation();
        for(auto &n : normalVector) {
            n = rotation * n;
        }
        for(auto &t : tangentVector) {
            t = rotation * t;
        }
    }

    // Indices
    size_t size = vertices().size();
    auto indexVector = mesh.indices();
    for(auto &it : indexVector) {
        it += size;
    }
    indices().insert(indices().end(), indexVector.begin(), indexVector.end());
    // Vertex attributes
    vertices().insert(vertices().end(), vertexVector.begin(), vertexVector.end());
    tangents().insert(tangents().end(), tangentVector.begin(), tangentVector.end());
    normals().insert(normals().end(), normalVector.begin(), normalVector.end());
    colors().insert(colors().end(), mesh.colors().begin(), mesh.colors().end());
    uv0().insert(uv0().end(), mesh.uv0().begin(), mesh.uv0().end());
    uv1().insert(uv1().end(), mesh.uv1().begin(), mesh.uv1().end());

    recalcBounds();
}
/*!
    \internal
*/
void Mesh::switchState(ResourceState state) {
    setState(state);
}
/*!
    \internal
*/
bool Mesh::isUnloadable() {
    return true;
}
/*!
    \internal
*/
void Mesh::loadUserData(const VariantMap &data) {
    auto meshData = data.find(gData);
    if(meshData != data.end()) {
        VariantList mesh = meshData->second.value<VariantList>();
        auto i = mesh.begin();

        int flags = (*i).toInt();

        i++;
        int sub = 0;
        for(auto &material : (*i).toList()) {
            setDefaultMaterial(Engine::loadResource<Material>(material.toString()), sub);
            sub++;
        }

        i++;
        uint32_t vCount = (*i).toInt();

        i++;
        uint32_t tCount = (*i).toInt();

        Vector3 min( FLT_MAX);
        Vector3 max(-FLT_MAX);

        ByteArray vertexData;
        // Positions (Required field)
        i++;
        vertexData = (*i).toByteArray();
        m_vertices.resize(vCount);
        memcpy(m_vertices.data(),  &vertexData[0], sizeof(Vector3) * vCount);
        for(uint32_t i = 0; i < vCount; i++) {
            min.x = MIN(min.x, m_vertices[i].x);
            min.y = MIN(min.y, m_vertices[i].y);
            min.z = MIN(min.z, m_vertices[i].z);

            max.x = MAX(max.x, m_vertices[i].x);
            max.y = MAX(max.y, m_vertices[i].y);
            max.z = MAX(max.z, m_vertices[i].z);
        }

        // Indices (Required field)
        i++;
        vertexData = (*i).toByteArray();
        m_indices.resize(tCount * 3);
        memcpy(m_indices.data(), vertexData.data(), sizeof(uint32_t) * tCount * 3);

        // Load attributes
        if(flags & MeshAttributes::Color) { // Optional field
            i++;
            vertexData = (*i).toByteArray();
            m_colors.resize(vCount);
            memcpy(m_colors.data(), vertexData.data(), sizeof(Vector4) * vCount);
        }
        if(flags & MeshAttributes::Uv0) { // Optional field
            i++;
            vertexData = (*i).toByteArray();
            m_uv0.resize(vCount);
            memcpy(m_uv0.data(), vertexData.data(), sizeof(Vector2) * vCount);
        }
        if(flags & MeshAttributes::Normals) { // Optional field
            i++;
            vertexData = (*i).toByteArray();
            m_normals.resize(vCount);
            memcpy(m_normals.data(), vertexData.data(), sizeof(Vector3) * vCount);
        }
        if(flags & MeshAttributes::Tangents) { // Optional field
            i++;
            vertexData = (*i).toByteArray();
            m_tangents.resize(vCount);
            memcpy(m_tangents.data(), vertexData.data(), sizeof(Vector3) * vCount);
        }
        if(flags & MeshAttributes::Skinned) { // Optional field
            i++;
            vertexData = (*i).toByteArray();
            m_weights.resize(vCount);
            memcpy(m_weights.data(), vertexData.data(), sizeof(Vector4) * vCount);

            i++;
            vertexData = (*i).toByteArray();
            m_bones.resize(vCount);
            memcpy(m_bones.data(), vertexData.data(), sizeof(Vector4) * vCount);
        }

        i++;
        // Load offsets
        m_offsets.clear();
        if(i != mesh.end()) {
            for(auto &offset : (*i).toList()) {
                m_offsets.push_back(offset.toInt());
            }
        }
        if(m_offsets.empty()) {
            m_offsets.push_back(0);
        }

        m_box.setBox(min, max);
    }
    switchState(ToBeUpdated);
}
/*!
    \internal
*/
VariantMap Mesh::saveUserData() const {
    VariantMap result;

    VariantList mesh;

    int flags = 0;
    flags = m_uv0.empty() ? flags : (flags | Uv0);
    flags = m_colors.empty() ? flags : (flags | Color);

    flags = m_normals.empty() ? flags : (flags | Normals);
    flags = m_tangents.empty() ? flags : (flags | Tangents);

    flags = m_weights.empty() ? flags : (flags | Skinned);

    mesh.push_back(flags);

    // Push materials
    VariantList materials;
    for(auto it : m_defaultMaterials) {
        materials.push_back(Engine::resourceSystem()->reference(it));
    }
    mesh.push_back(materials);

    // Push geometry
    size_t vCount = m_vertices.size();
    mesh.push_back(static_cast<int32_t>(vCount));
    mesh.push_back(static_cast<int32_t>(m_indices.size() / 3));

    { // Required field
        ByteArray buffer;
        buffer.resize(sizeof(Vector3) * vCount);
        memcpy(buffer.data(), m_vertices.data(), sizeof(Vector3) * vCount);
        mesh.push_back(buffer);
    }
    { // Required field
        ByteArray buffer;
        buffer.resize(sizeof(uint32_t) * m_indices.size());
        memcpy(buffer.data(), m_indices.data(), sizeof(uint32_t) * m_indices.size());
        mesh.push_back(buffer);
    }

    if(!m_colors.empty()) { // Optional field
        ByteArray buffer;
        buffer.resize(sizeof(Vector4) * vCount);
        memcpy(buffer.data(), m_colors.data(), sizeof(Vector4) * vCount);
        mesh.push_back(buffer);
    }
    if(!m_uv0.empty()) { // Optional field
        ByteArray buffer;
        buffer.resize(sizeof(Vector2) * vCount);
        memcpy(buffer.data(), m_uv0.data(), sizeof(Vector2) * vCount);
        mesh.push_back(buffer);
    }
    if(!m_normals.empty()) { // Optional field
        ByteArray buffer;
        buffer.resize(sizeof(Vector3) * vCount);
        memcpy(buffer.data(), m_normals.data(), sizeof(Vector3) * vCount);
        mesh.push_back(buffer);
    }
    if(!m_tangents.empty()) { // Optional field
        ByteArray buffer;
        buffer.resize(sizeof(Vector3) * vCount);
        memcpy(buffer.data(), m_tangents.data(), sizeof(Vector3) * vCount);
        mesh.push_back(buffer);
    }
    if(!m_weights.empty()) { // Optional field
        {
            ByteArray buffer;
            buffer.resize(sizeof(Vector4) * vCount);
            memcpy(buffer.data(), m_weights.data(), sizeof(Vector4) * vCount);
            mesh.push_back(buffer);
        }
        {
            ByteArray buffer;
            buffer.resize(sizeof(Vector4) * vCount);
            memcpy(buffer.data(), m_bones.data(), sizeof(Vector4) * vCount);
            mesh.push_back(buffer);
        }
    }

    // Save offsets
    VariantList offsets;
    for(auto it : m_offsets) {
        offsets.push_back(it);
    }
    mesh.push_back(offsets);

    result[gData] = mesh;

    return result;
}
