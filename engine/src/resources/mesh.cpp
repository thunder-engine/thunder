#include <resources/mesh.h>

#include <resources/material.h>

#include <file.h>
#include <log.h>

#include <cstring>
#include <cfloat>

#define HEADER      "Header"
#define DATA        "Data"
#define DEFAULTMESH ".embedded/DefaultMesh.mtl"

/*!
    \class Lod
    \brief This class contains all necessary data of Level Of Detail for the Mesh.
    \inmodule Resources
*/

Lod::Lod() :
    m_Material(nullptr) {

}

bool Lod::operator== (const Lod &right) const {
    return (m_Material == right.m_Material) &&
           (m_Indices == right.m_Indices) &&
           (m_Colors == right.m_Colors) &&
           (m_Weights == right.m_Weights) &&
           (m_Bones == right.m_Bones) &&
           (m_Vertices == right.m_Vertices) &&
           (m_Normals == right.m_Normals) &&
           (m_Tangents == right.m_Tangents) &&
           (m_Uv0 == right.m_Uv0) &&
           (m_Uv1 == right.m_Uv1);
}

/*!
    Returns a material for the particular Lod.
*/
Material *Lod::material() const {
    return m_Material;
}
/*!
    Sets a \a material for the particular Lod.
*/
void Lod::setMaterial(Material *material) {
    m_Material = material;
}
/*!
    Returns an array of mesh indices for the particular Lod.
*/
IndexVector &Lod::indices() {
    return m_Indices;
}
/*!
    Sets an array of mesh \a indices for the particular Lod.
*/
void Lod::setIndices(const IndexVector &indices) {
    m_Indices = indices;
}
/*!
    Returns an array of colors for vertices for the particular Lod.
*/
Vector4Vector &Lod::colors() {
    return m_Colors;
}
/*!
    Sets an array of \a colors for vertices for the particular Lod.
*/
void Lod::setColors(const Vector4Vector &colors) {
    m_Colors = colors;
}
/*!
    Returns an array of bone weights for the particular Lod.
*/
Vector4Vector &Lod::weights() {
    return m_Weights;
}
/*!
    Sets an array of bone \a weights for the particular Lod.
*/
void Lod::setWeights(const Vector4Vector &weights) {
    m_Weights = weights;
}
/*!
    Returns an array of bones for vertices for the particular Lod.
*/
Vector4Vector &Lod::bones() {
    return m_Bones;
}
/*!
    Sets an array of \a bones for vertices for the particular Lod.
*/
void Lod::setBones(const Vector4Vector &bones) {
    m_Bones = bones;
}
/*!
    Returns an array of mesh vertices for the particular Lod.
*/
Vector3Vector &Lod::vertices() {
    return m_Vertices;
}
/*!
    Sets an array of mesh \a vertices for the particular Lod.
*/
void Lod::setVertices(const Vector3Vector &vertices) {
    m_Vertices = vertices;
}
/*!
    Returns an array of mesh normals for the particular Lod.
*/
Vector3Vector &Lod::normals() {
    return m_Normals;
}
/*!
    Sets an array of mesh \a normals for the particular Lod.
*/
void Lod::setNormals(const Vector3Vector &normals) {
    m_Normals = normals;
}
/*!
    Returns an array of mesh tangents for the particular Lod.
*/
Vector3Vector &Lod::tangents() {
    return m_Tangents;
}
/*!
    Sets an array of mesh \a tangents for the particular Lod.
*/
void Lod::setTangents(const Vector3Vector &tangents) {
    m_Tangents = tangents;
}
/*!
    Returns an array of mesh uv0 (base) texture coordinates for the particular Lod.
*/
Vector2Vector &Lod::uv0() {
    return m_Uv0;
}
/*!
    Sets an array of mesh \a uv0 (base) texture coordinates for the particular Lod.
*/
void Lod::setUv0(const Vector2Vector &uv0) {
    m_Uv0 = uv0;
}
/*!
    Returns an array of mesh uv1 texture coordinates for the particular Lod.
*/
Vector2Vector &Lod::uv1() {
    return m_Uv1;
}
/*!
    Sets an array of mesh \a uv1 texture coordinates for the particular Lod.
*/
void Lod::setUv1(const Vector2Vector &uv1) {
    m_Uv1 = uv1;
}

class MeshPrivate {
public:
    MeshPrivate() :
            m_Dynamic(false),
            m_Flags(0),
            m_Topology(Mesh::Triangles) {

    }

    bool m_Dynamic;

    uint8_t m_Flags;

    int m_Topology;

    LodQueue m_Lods;

    AABBox m_Box;
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
    p_ptr->m_Lods.clear();
}
/*!
    Returns true in case of mesh can by changed at the runtime; otherwise returns false.
*/
bool Mesh::isDynamic() const {
    return p_ptr->m_Dynamic;
}
/*!
    Marks mesh as dynamic that means it's can be changed at the runtime.
*/
void Mesh::makeDynamic() {
    p_ptr->m_Dynamic = true;
}
/*!
    \internal
*/
void Mesh::loadUserData(const VariantMap &data) {
    clear();

    auto it = data.find(HEADER);
    if(it != data.end()) {
        VariantList header = (*it).second.value<VariantList>();

        auto i = header.begin();
        p_ptr->m_Flags = (*i).toInt();
    }

    auto mesh = data.find(DATA);
    if(mesh != data.end()) {
        Vector3 min( FLT_MAX);
        Vector3 max(-FLT_MAX);

        VariantList surface = (*mesh).second.value<VariantList>();
        auto x = surface.begin();
        p_ptr->m_Topology = static_cast<Mesh::TriangleTopology>((*x).toInt());
        x++;
        while(x != surface.end()) {
            Lod l;

            VariantList lod = (*x).value<VariantList>();
            auto y = lod.begin();
            string path = (*y).toString();
            l.m_Material = Engine::loadResource<Material>(path.empty() ? DEFAULTMESH : path);
            y++;

            uint32_t vCount = (*y).toInt();
            y++;

            uint32_t tCount = (*y).toInt();
            y++;

            ByteArray data;
            { // Required field
                data = (*y).toByteArray();
                y++;
                l.m_Vertices.resize(vCount);
                memcpy(&l.m_Vertices[0],  &data[0], sizeof(Vector3) * vCount);
                for(uint32_t i = 0; i < vCount; i++) {
                    min.x = MIN(min.x, l.m_Vertices[i].x);
                    min.y = MIN(min.y, l.m_Vertices[i].y);
                    min.z = MIN(min.z, l.m_Vertices[i].z);

                    max.x = MAX(max.x, l.m_Vertices[i].x);
                    max.y = MAX(max.y, l.m_Vertices[i].y);
                    max.z = MAX(max.z, l.m_Vertices[i].z);
                }
            }
            { // Required field
                data = (*y).toByteArray();
                y++;
                l.m_Indices.resize(tCount * 3);
                memcpy(&l.m_Indices[0], &data[0], sizeof(uint32_t) * tCount * 3);
            }
            if(p_ptr->m_Flags & MeshAttributes::Color) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_Colors.resize(vCount);
                memcpy(&l.m_Colors[0], &data[0], sizeof(Vector4) * vCount);
            }
            if(p_ptr->m_Flags & MeshAttributes::Uv0) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_Uv0.resize(vCount);
                memcpy(&l.m_Uv0[0], &data[0], sizeof(Vector2) * vCount);
            }
            if(p_ptr->m_Flags & MeshAttributes::Uv1) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_Uv1.resize(vCount);
                memcpy(&l.m_Uv1[0], &data[0], sizeof(Vector2) * vCount);
            }
            if(p_ptr->m_Flags & MeshAttributes::Normals) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_Normals.resize(vCount);
                memcpy(&l.m_Normals[0], &data[0], sizeof(Vector3) * vCount);
            }
            if(p_ptr->m_Flags & MeshAttributes::Tangents) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_Tangents.resize(vCount);
                memcpy(&l.m_Tangents[0], &data[0],  sizeof(Vector3) * vCount);
            }
            if(p_ptr->m_Flags & MeshAttributes::Skinned) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.m_Weights.resize(vCount);
                memcpy(&l.m_Weights[0], &data[0],  sizeof(Vector4) * vCount);

                data = (*y).toByteArray();
                y++;
                l.m_Bones.resize(vCount);
                memcpy(&l.m_Bones[0], &data[0],  sizeof(Vector4) * vCount);
            }
            p_ptr->m_Lods.push_back(l);

            x++;
        }
        p_ptr->m_Box.setBox(min, max);
    }
    switchState(ToBeUpdated);
}
/*!
    \internal
*/
VariantMap Mesh::saveUserData() const {
    VariantMap result;

    int32_t flag = flags();

    VariantList header;
    header.push_back(flag);
    result[HEADER]  = header;

    VariantList surface;
    surface.push_back(topology());

    for(size_t index = 0; index < p_ptr->m_Lods.size(); index++) {
        Lod *l = lod(index);

        VariantList lod;
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

        if(flag & Color) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector4) * vCount);
            memcpy(&buffer[0], &l->colors()[0], sizeof(Vector4) * vCount);
            lod.push_back(buffer);
        }
        if(flag & Uv0) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector2) * vCount);
            memcpy(&buffer[0], &l->uv0()[0], sizeof(Vector2) * vCount);
            lod.push_back(buffer);
        }
        if(flag & Uv1) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector2) * vCount);
            memcpy(&buffer[0], &l->uv1()[0], sizeof(Vector2) * vCount);
            lod.push_back(buffer);
        }

        if(flag & Normals) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector3) * vCount);
            memcpy(&buffer[0], &l->normals()[0], sizeof(Vector3) * vCount);
            lod.push_back(buffer);
        }
        if(flag & Tangents) { // Optional field
            ByteArray buffer;
            buffer.resize(sizeof(Vector3) * vCount);
            memcpy(&buffer[0], &l->tangents()[0], sizeof(Vector3) * vCount);
            lod.push_back(buffer);
        }
        if(flag & Skinned) { // Optional field
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
    Returns the number of Levels Of Details
*/
int Mesh::lodsCount() const {
    return p_ptr->m_Lods.size();
}
/*!
    Returns bounding box for the Mesh.
*/
AABBox Mesh::bound() const {
    return p_ptr->m_Box;
}
/*!
    Sets new bounding \a box for the Mesh.
*/
void Mesh::setBound(AABBox box) {
    p_ptr->m_Box = box;
}
/*!
    Returns poligon topology for the mesh.
    For more details please see the Mesh::TriangleTopology enum.
*/
int Mesh::topology() const {
    return p_ptr->m_Topology;
}
/*!
    Sets poligon \a topology for the mesh.
    For more details please see the Mesh::TriangleTopology enum.
*/
void Mesh::setTopology(int topology) {
    p_ptr->m_Topology = topology;
}
/*!
    Returns vertex attributes flags.
    For more details please see the Mesh::Attributes enum.
*/
int Mesh::flags() const {
    return p_ptr->m_Flags;
}
/*!
    Sets vertex attributes \a flags.
    For more details please see the Mesh::Attributes enum.
*/
void Mesh::setFlags(int flags) {
    p_ptr->m_Flags = flags;
}
/*!
    Adds the new \a lod data for the Mesh.
    Retuns index of new lod.
*/
int Mesh::addLod(Lod *lod) {
    if(lod) {
        p_ptr->m_Lods.push_back(*lod);
        recalcBounds();
        switchState(ToBeUpdated);
        return p_ptr->m_Lods.size() - 1;
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
            p_ptr->m_Lods[lod] = *data;
            recalcBounds();
            switchState(ToBeUpdated);
        }
    } else {
        addLod(data);
    }
}
/*!
    Merges current with provided \a mesh.
    In the case of the \a transform, the matrix is not nullptr it will be applied to \a mesh before merging.
*/
void Mesh::batchMesh(Mesh *mesh, Matrix4 *transform) {
    if(mesh) {
        for(size_t i = 0; i < mesh->p_ptr->m_Lods.size(); i++) {
            Lod lod = mesh->p_ptr->m_Lods[i];
            if(transform) {
                for(auto &v : lod.vertices()) {
                    v = *transform * v;
                }

                Matrix3 rotation = transform->rotation();
                for(auto &n : lod.normals()) {
                    n = rotation * n;
                }
                for(auto &t : lod.tangents()) {
                    t = rotation * t;
                }
            }

            if(i < p_ptr->m_Lods.size()) {
                Lod &current = p_ptr->m_Lods[i];
                // Indices
                auto &curIndex = current.indices();
                uint32_t size = current.vertices().size();
                auto &srcIndex = lod.indices();
                for(auto &it : srcIndex) {
                    it += size;
                }
                curIndex.insert(curIndex.end(), srcIndex.begin(), srcIndex.end());
                // Vertex attributes
                current.vertices().insert(current.vertices().end(), lod.vertices().begin(), lod.vertices().end());
                current.tangents().insert(current.tangents().end(), lod.tangents().begin(), lod.tangents().end());
                current.normals().insert(current.normals().end(), lod.normals().begin(), lod.normals().end());
                current.colors().insert(current.colors().end(), lod.colors().begin(), lod.colors().end());
                current.uv0().insert(current.uv0().end(), lod.uv0().begin(), lod.uv0().end());
                current.uv1().insert(current.uv1().end(), lod.uv1().begin(), lod.uv1().end());
            } else {
                p_ptr->m_Lods.push_back(lod);
            }
        }
        recalcBounds();
    }
}
/*!
    Generates bound box according new geometry.
*/
void Mesh::recalcBounds() {
    Vector3 min( FLT_MAX);
    Vector3 max(-FLT_MAX);

    for(auto l : p_ptr->m_Lods) {
        for(uint32_t i = 0; i < l.vertices().size(); i++) {
            min.x = MIN(min.x, l.m_Vertices[i].x);
            min.y = MIN(min.y, l.m_Vertices[i].y);
            min.z = MIN(min.z, l.m_Vertices[i].z);

            max.x = MAX(max.x, l.m_Vertices[i].x);
            max.y = MAX(max.y, l.m_Vertices[i].y);
            max.z = MAX(max.z, l.m_Vertices[i].z);
        }
    }

    p_ptr->m_Box.setBox(min, max);
}
/*!
    Returns Lod data for the \a lod index if exists; othewise returns nullptr.
*/
Lod *Mesh::lod(int lod) const {
    if(lod < lodsCount()) {
        return &p_ptr->m_Lods[lod];
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
