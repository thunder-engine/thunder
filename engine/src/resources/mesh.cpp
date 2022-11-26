#include <resources/mesh.h>

#include <resources/material.h>

#include <file.h>
#include <log.h>

#include <cstring>
#include <cfloat>

#define DATA        "Data"
#define DEFAULTMESH ".embedded/DefaultMesh.mtl"

/*!
    \class Lod
    \brief This class contains all necessary data of Level Of Detail for the Mesh.
    \inmodule Resources
*/

Lod::Lod() :
    m_material(nullptr),
    m_flags(0),
    m_topology(Mesh::Triangles) {

}

bool Lod::operator== (const Lod &right) const {
    return (m_material == right.m_material) &&
           (m_indices == right.m_indices) &&
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
    Returns a material for the particular Lod.
*/
Material *Lod::material() const {
    return m_material;
}
/*!
    Sets a \a material for the particular Lod.
*/
void Lod::setMaterial(Material *material) {
    m_material = material;
}
/*!
    Returns an array of mesh indices for the particular Lod.
*/
IndexVector &Lod::indices() {
    return m_indices;
}
/*!
    Sets an array of mesh \a indices for the particular Lod.
*/
void Lod::setIndices(const IndexVector &indices) {
    m_indices = indices;
}
/*!
    Returns an array of colors for vertices for the particular Lod.
*/
Vector4Vector &Lod::colors() {
    return m_colors;
}
/*!
    Sets an array of \a colors for vertices for the particular Lod.
*/
void Lod::setColors(const Vector4Vector &colors) {
    m_colors = colors;
}
/*!
    Returns an array of bone weights for the particular Lod.
*/
Vector4Vector &Lod::weights() {
    return m_weights;
}
/*!
    Sets an array of bone \a weights for the particular Lod.
*/
void Lod::setWeights(const Vector4Vector &weights) {
    m_weights = weights;
}
/*!
    Returns an array of bones for vertices for the particular Lod.
*/
Vector4Vector &Lod::bones() {
    return m_bones;
}
/*!
    Sets an array of \a bones for vertices for the particular Lod.
*/
void Lod::setBones(const Vector4Vector &bones) {
    m_bones = bones;
}
/*!
    Returns an array of mesh vertices for the particular Lod.
*/
Vector3Vector &Lod::vertices() {
    return m_vertices;
}
/*!
    Sets an array of mesh \a vertices for the particular Lod.
*/
void Lod::setVertices(const Vector3Vector &vertices) {
    m_vertices = vertices;
}
/*!
    Returns an array of mesh normals for the particular Lod.
*/
Vector3Vector &Lod::normals() {
    return m_normals;
}
/*!
    Sets an array of mesh \a normals for the particular Lod.
*/
void Lod::setNormals(const Vector3Vector &normals) {
    m_normals = normals;
}
/*!
    Returns an array of mesh tangents for the particular Lod.
*/
Vector3Vector &Lod::tangents() {
    return m_tangents;
}
/*!
    Sets an array of mesh \a tangents for the particular Lod.
*/
void Lod::setTangents(const Vector3Vector &tangents) {
    m_tangents = tangents;
}
/*!
    Returns an array of mesh uv0 (base) texture coordinates for the particular Lod.
*/
Vector2Vector &Lod::uv0() {
    return m_uv0;
}
/*!
    Sets an array of mesh \a uv0 (base) texture coordinates for the particular Lod.
*/
void Lod::setUv0(const Vector2Vector &uv0) {
    m_uv0 = uv0;
}
/*!
    Returns an array of mesh uv1 texture coordinates for the particular Lod.
*/
Vector2Vector &Lod::uv1() {
    return m_uv1;
}
/*!
    Sets an array of mesh \a uv1 texture coordinates for the particular Lod.
*/
void Lod::setUv1(const Vector2Vector &uv1) {
    m_uv1 = uv1;
}
/*!
    Returns poligon topology for the mesh.
    For more details please see the Mesh::TriangleTopology enum.
*/
int Lod::topology() const {
    return m_topology;
}
/*!
    Sets poligon \a topology for the mesh.
    For more details please see the Mesh::TriangleTopology enum.
*/
void Lod::setTopology(int topology) {
    m_topology = topology;
}
/*!
    Returns vertex attributes flags.
    For more details please see the Mesh::Attributes enum.
*/
int Lod::flags() const {
    return m_flags;
}
/*!
    Sets vertex attributes \a flags.
    For more details please see the Mesh::Attributes enum.
*/
void Lod::setFlags(int flags) {
    m_flags = flags;
}
/*!
    Returns bounding box for the Mesh.
*/
AABBox Lod::bound() const {
    return m_box;
}
/*!
    Sets new bounding \a box for the Mesh.
*/
void Lod::setBound(const AABBox &box) {
    m_box = box;
}
/*!
    Recalculates the normals of the Mesh from the triangles and vertices.
*/
void Lod::recalcNormals() {
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
}
/*!
    Generates bound box according new geometry.
*/
void Lod::recalcBounds() {
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
}
/*!
    Merges current with provided \a mesh.
    In the case of the \a transform, the matrix is not nullptr it will be applied to \a mesh before merging.
*/
void Lod::batchMesh(Lod &mesh, Matrix4 *transform) {
    if(transform) {
        for(auto &v : mesh.vertices()) {
            v = *transform * v;
        }

        Matrix3 rotation = transform->rotation();
        for(auto &n : mesh.normals()) {
            n = rotation * n;
        }
        for(auto &t : mesh.tangents()) {
            t = rotation * t;
        }
    }

    // Indices
    uint32_t size = vertices().size();
    auto &srcIndex = mesh.indices();
    for(auto &it : srcIndex) {
        it += size;
    }
    indices().insert(indices().end(), srcIndex.begin(), srcIndex.end());
    // Vertex attributes
    vertices().insert(vertices().end(), mesh.vertices().begin(), mesh.vertices().end());
    tangents().insert(tangents().end(), mesh.tangents().begin(), mesh.tangents().end());
    normals().insert(normals().end(), mesh.normals().begin(), mesh.normals().end());
    colors().insert(colors().end(), mesh.colors().begin(), mesh.colors().end());
    uv0().insert(uv0().end(), mesh.uv0().begin(), mesh.uv0().end());
    uv1().insert(uv1().end(), mesh.uv1().begin(), mesh.uv1().end());

    recalcBounds();
}

class MeshPrivate {
public:
    MeshPrivate() :
            m_dynamic(false) {

    }

    bool m_dynamic;

    LodQueue m_lods;

};

/*!
    \class Mesh
    \brief A class that allows creating or modifying meshes at the runtime.
    \inmodule Resources
*/

/*!
    \enum Mesh::MeshAttributes

    \value Color \c The Lod structure contains color information for the vertices.
    \value Uv0 \c The Lod structure contains base texture coordinates for the vertices.
    \value Uv1 \c The Lod structure contains secondary texture coordinates for the vertices.
    \value Normals \c The Lod structure contains normal vectors for the vertices.
    \value Tangents \c The Lod structure contains tangent vectors for the vertices.
    \value Skinned \c The Mesh was marked as skinned which means Lod structure contains bones and weights information for the vertices.
*/

/*!
    \enum Mesh::TriangleTopology

    \value Triangles \c This mode means the indices array will be used to stitch vertices into triangles.
    \value Lines \c The The mesh will be rendered as set of lines. Indices array will be used.
    \value TriangleStrip \c A triangle strip is a series of connected triangles from the triangle mesh, sharing vertices. Indices array is not required.
    \value LineStrip \c The same as TriangleStrip but will be rendered as Lines.
    \value TriangleFan \c A set of connected triangles that share one central vertex. Indices array is not required.
*/

Mesh::Mesh() :
        p_ptr(new MeshPrivate) {

}

Mesh::~Mesh() {
    clear();

    delete p_ptr;
}
/*!
    Removes all attached Levels Of Detal
*/
void Mesh::clear() {
    p_ptr->m_lods.clear();
}
/*!
    Returns true in case of mesh can by changed at the runtime; otherwise returns false.
*/
bool Mesh::isDynamic() const {
    return p_ptr->m_dynamic;
}
/*!
    Marks mesh as dynamic that means it's can be changed at the runtime.
*/
void Mesh::makeDynamic() {
    p_ptr->m_dynamic = true;
}
/*!
    \internal
*/
void Mesh::loadUserData(const VariantMap &data) {
    clear();

    auto mesh = data.find(DATA);
    if(mesh != data.end()) {
        VariantList surface = (*mesh).second.value<VariantList>();
        auto x = surface.begin();
        while(x != surface.end()) {
            VariantList lod = (*x).value<VariantList>();
            auto y = lod.begin();

            Lod l;
            l.m_topology = static_cast<Mesh::TriangleTopology>((*y).toInt());
            y++;
            l.m_flags = (*y).toInt();
            y++;
            string path = (*y).toString();
            l.m_material = Engine::loadResource<Material>(path.empty() ? DEFAULTMESH : path);
            y++;

            uint32_t vCount = (*y).toInt();
            y++;

            uint32_t tCount = (*y).toInt();
            y++;

            Vector3 min( FLT_MAX);
            Vector3 max(-FLT_MAX);

            ByteArray data;
            { // Required field
                data = (*y).toByteArray();
                y++;
                l.m_vertices.resize(vCount);
                memcpy(&l.m_vertices[0],  &data[0], sizeof(Vector3) * vCount);
                for(uint32_t i = 0; i < vCount; i++) {
                    min.x = MIN(min.x, l.m_vertices[i].x);
                    min.y = MIN(min.y, l.m_vertices[i].y);
                    min.z = MIN(min.z, l.m_vertices[i].z);

                    max.x = MAX(max.x, l.m_vertices[i].x);
                    max.y = MAX(max.y, l.m_vertices[i].y);
                    max.z = MAX(max.z, l.m_vertices[i].z);
                }
            }
            { // Required field
                data = (*y).toByteArray();
                y++;
                l.m_indices.resize(tCount * 3);
                memcpy(&l.m_indices[0], &data[0], sizeof(uint32_t) * tCount * 3);
            }
            if(l.m_flags & MeshAttributes::Color) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_colors.resize(vCount);
                memcpy(&l.m_colors[0], &data[0], sizeof(Vector4) * vCount);
            }
            if(l.m_flags & MeshAttributes::Uv0) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_uv0.resize(vCount);
                memcpy(&l.m_uv0[0], &data[0], sizeof(Vector2) * vCount);
            }
            if(l.m_flags & MeshAttributes::Uv1) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_uv1.resize(vCount);
                memcpy(&l.m_uv1[0], &data[0], sizeof(Vector2) * vCount);
            }
            if(l.m_flags & MeshAttributes::Normals) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_normals.resize(vCount);
                memcpy(&l.m_normals[0], &data[0], sizeof(Vector3) * vCount);
            }
            if(l.m_flags & MeshAttributes::Tangents) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_tangents.resize(vCount);
                memcpy(&l.m_tangents[0], &data[0],  sizeof(Vector3) * vCount);
            }
            if(l.m_flags & MeshAttributes::Skinned) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_weights.resize(vCount);
                memcpy(&l.m_weights[0], &data[0],  sizeof(Vector4) * vCount);

                data = (*y).toByteArray();
                y++;
                l.m_bones.resize(vCount);
                memcpy(&l.m_bones[0], &data[0],  sizeof(Vector4) * vCount);
            }
            l.m_box.setBox(min, max);
            p_ptr->m_lods.push_back(l);

            x++;
        }
    }
    switchState(ToBeUpdated);
}
/*!
    \internal
*/
VariantMap Mesh::saveUserData() const {
    VariantMap result;

    VariantList surface;

    for(size_t index = 0; index < p_ptr->m_lods.size(); index++) {
        Lod *l = lod(index);

        VariantList lod;
        lod.push_back(l->topology());
        lod.push_back(l->flags());

        // Push material
        lod.push_back("{00000000-0402-0000-0000-000000000000}");

        uint32_t vCount = l->vertices().size();
        lod.push_back(static_cast<int32_t>(vCount));
        lod.push_back(static_cast<int32_t>(l->indices().size() / 3));

        { // Required field
            ByteArray buffer;
            buffer.resize(sizeof(Vector3) * vCount);
            memcpy(&buffer[0], &l->vertices()[0], sizeof(Vector3) * vCount);
            lod.push_back(buffer);
        }
        { // Required field
            ByteArray buffer;
            buffer.resize(sizeof(uint32_t) * l->indices().size());
            memcpy(&buffer[0], &l->indices()[0], sizeof(uint32_t) * l->indices().size());
            lod.push_back(buffer);
        }

        if(l->flags() & Color) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector4) * vCount);
            memcpy(&buffer[0], &l->colors()[0], sizeof(Vector4) * vCount);
            lod.push_back(buffer);
        }
        if(l->flags() & Uv0) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector2) * vCount);
            memcpy(&buffer[0], &l->uv0()[0], sizeof(Vector2) * vCount);
            lod.push_back(buffer);
        }
        if(l->flags() & Uv1) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector2) * vCount);
            memcpy(&buffer[0], &l->uv1()[0], sizeof(Vector2) * vCount);
            lod.push_back(buffer);
        }

        if(l->flags() & Normals) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector3) * vCount);
            memcpy(&buffer[0], &l->normals()[0], sizeof(Vector3) * vCount);
            lod.push_back(buffer);
        }
        if(l->flags() & Tangents) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector3) * vCount);
            memcpy(&buffer[0], &l->tangents()[0], sizeof(Vector3) * vCount);
            lod.push_back(buffer);
        }
        if(l->flags() & Skinned) { // Optional field
            {
                ByteArray buffer;
                buffer.resize(sizeof(Vector4) * vCount);
                memcpy(&buffer[0], &l->weights()[0], sizeof(Vector4) * vCount);
                lod.push_back(buffer);
            }
            {
                ByteArray buffer;
                buffer.resize(sizeof(Vector4) * vCount);
                memcpy(&buffer[0], &l->bones()[0], sizeof(Vector4) * vCount);
                lod.push_back(buffer);
            }
        }
        surface.push_back(lod);
    }
    result[DATA] = surface;

    return result;
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
    Returns the number of Levels Of Details
*/
int Mesh::lodsCount() const {
    return p_ptr->m_lods.size();
}
/*!
    Adds the new \a lod data for the Mesh.
    Retuns index of new lod.
*/
int Mesh::addLod(Lod *lod) {
    if(lod) {
        p_ptr->m_lods.push_back(*lod);
        switchState(ToBeUpdated);
        return p_ptr->m_lods.size() - 1;
    }
    return -1;
}
/*!
    Sets the new \a data for the particular \a lod.
    This method can replace the existing data.
*/
void Mesh::setLod(int lod, Lod *data) {
    if(lod < lodsCount()) {
        if(data) {
            p_ptr->m_lods[lod] = *data;
            switchState(ToBeUpdated);
        }
    } else {
        addLod(data);
    }
}
/*!
    Returns Lod data for the \a lod index if exists; othewise returns nullptr.
*/
Lod *Mesh::lod(int lod) const {
    if(lod < lodsCount()) {
        return &p_ptr->m_lods[lod];
    }
    return nullptr;
}
/*!
    \internal

    \warning Do not call this function manually
*/
void Mesh::registerSuper(ObjectSystem *system) {
    REGISTER_META_TYPE(Lod);
    Mesh::registerClassFactory(system);
}
