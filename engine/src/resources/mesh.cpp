#include <resources/mesh.h>

#include <resources/material.h>

#include <file.h>
#include <log.h>

#include <cstring>
#include <cfloat>

#define HEADER      "Header"
#define DATA        "Data"
#define DEFAULTMESH ".embedded/DefaultMesh.mtl"

class MeshPrivate {
public:
    MeshPrivate() :
            m_Dynamic(false),
            m_Flags(0),
            m_Mode(Mesh::MODE_TRIANGLES) {

    }

    bool m_Dynamic;

    uint8_t m_Flags;

    Mesh::Modes m_Mode;

    Mesh::LodQueue m_Lods;

    AABBox m_Box;
};

Mesh::Mesh() :
        p_ptr(new MeshPrivate) {

}

Mesh::~Mesh() {
    clear();

    delete p_ptr;
}
/*!
    \internal
*/
void Mesh::clear() {
    p_ptr->m_Lods.clear();
}

bool Mesh::isDynamic() const {
    return p_ptr->m_Dynamic;
}

void Mesh::makeDynamic() {
    p_ptr->m_Dynamic = true;
}

void Mesh::loadUserData(const VariantMap &data) {
    clear();

    auto it = data.find(HEADER);
    if(it != data.end()) {
        VariantList header  = (*it).second.value<VariantList>();

        auto i = header.begin();
        p_ptr->m_Flags = (*i).toInt();
    }

    auto mesh = data.find(DATA);
    if(mesh != data.end()) {
        Vector3 min( FLT_MAX);
        Vector3 max(-FLT_MAX);

        VariantList surface = (*mesh).second.value<VariantList>();
        auto x  = surface.begin();
        p_ptr->m_Mode = static_cast<Mesh::Modes>((*x).toInt());
        x++;
        while(x != surface.end()) {
            Lod l;

            VariantList lod = (*x).value<VariantList>();
            auto y      = lod.begin();
            string path = (*y).toString();
            l.material  = Engine::loadResource<Material>(path.empty() ? DEFAULTMESH : path);
            y++;

            uint32_t vCount = (*y).toInt();
            y++;

            uint32_t tCount = (*y).toInt();
            y++;

            ByteArray data;
            { // Required field
                data    = (*y).toByteArray();
                y++;
                l.vertices.resize(vCount);
                memcpy(&l.vertices[0],  &data[0], sizeof(Vector3) * vCount);
                for(uint32_t i = 0; i < vCount; i++) {
                    min.x = MIN(min.x, l.vertices[i].x);
                    min.y = MIN(min.y, l.vertices[i].y);
                    min.z = MIN(min.z, l.vertices[i].z);

                    max.x = MAX(max.x, l.vertices[i].x);
                    max.y = MAX(max.y, l.vertices[i].y);
                    max.z = MAX(max.z, l.vertices[i].z);
                }
            }
            { // Required field
                data    = (*y).toByteArray();
                y++;
                l.indices.resize(tCount * 3);
                memcpy(&l.indices[0], &data[0],  sizeof(uint32_t) * tCount * 3);
            }
            if(p_ptr->m_Flags & ATTRIBUTE_COLOR) { // Optional field
                data    = (*y).toByteArray();
                y++;
                l.colors.resize(vCount);
                memcpy(&l.colors[0], &data[0],  sizeof(Vector4) * vCount);
            }
            if(p_ptr->m_Flags & ATTRIBUTE_UV0) { // Optional field
                data    = (*y).toByteArray();
                y++;
                l.uv0.resize(vCount);
                memcpy(&l.uv0[0], &data[0],  sizeof(Vector2) * vCount);
            }
            if(p_ptr->m_Flags & ATTRIBUTE_UV1) { // Optional field
                data    = (*y).toByteArray();
                y++;
                l.uv1.resize(vCount);
                memcpy(&l.uv1[0], &data[0],  sizeof(Vector2) * vCount);
            }
            if(p_ptr->m_Flags & ATTRIBUTE_NORMALS) { // Optional field
                data    = (*y).toByteArray();
                y++;
                l.normals.resize(vCount);
                memcpy(&l.normals[0], &data[0],  sizeof(Vector3) * vCount);
            }
            if(p_ptr->m_Flags & ATTRIBUTE_TANGENTS) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.tangents.resize(vCount);
                memcpy(&l.tangents[0], &data[0],  sizeof(Vector3) * vCount);
            }
            if(p_ptr->m_Flags & ATTRIBUTE_SKINNED) { // Optional field
                data = (*y).toByteArray();
                y++;
                l.weights.resize(vCount);
                memcpy(&l.weights[0], &data[0],  sizeof(Vector4) * vCount);

                data = (*y).toByteArray();
                y++;
                l.bones.resize(vCount);
                memcpy(&l.bones[0], &data[0],  sizeof(Vector4) * vCount);
            }
            p_ptr->m_Lods.push_back(l);

            x++;
        }
        p_ptr->m_Box.setBox(min, max);
    }
    setState(ToBeUpdated);
}

Material *Mesh::material(uint32_t lod) const {
    if(lod < lodsCount()) {
        return p_ptr->m_Lods[lod].material;
    }
    return nullptr;
}

Vector3Vector Mesh::vertices(uint32_t lod) const {
    if(lod < lodsCount()) {
        return p_ptr->m_Lods[lod].vertices;
    }
    return Vector3Vector();
}

void Mesh::setVertices(uint32_t lod, const Vector3Vector &vertices) {
    if(lod < lodsCount()) {
        p_ptr->m_Lods[lod].vertices = vertices;
    }
}

Mesh::IndexVector Mesh::indices(uint32_t lod) const {
    if(lod < lodsCount()) {
        return p_ptr->m_Lods[lod].indices;
    }
    return Mesh::IndexVector();
}

uint32_t Mesh::lodsCount() const {
    return p_ptr->m_Lods.size();
}

uint32_t Mesh::vertexCount(uint32_t lod) const {
    if(lod < lodsCount()) {
        return p_ptr->m_Lods[lod].vertices.size();
    }
    return 0;
}

uint32_t Mesh::indexCount(uint32_t lod) const {
    if(lod < lodsCount()) {
        return p_ptr->m_Lods[lod].indices.size();
    }
    return 0;
}

AABBox Mesh::bound() const {
    return p_ptr->m_Box;
}

void Mesh::setBound(const AABBox &box) {
    p_ptr->m_Box = box;
}

Mesh::Modes Mesh::mode() const {
    return p_ptr->m_Mode;
}

void Mesh::setMode(Mesh::Modes mode) {
    p_ptr->m_Mode = mode;
}

uint8_t Mesh::flags() const {
    return p_ptr->m_Flags;
}

void Mesh::setFlags(uint8_t flags) {
    p_ptr->m_Flags = flags;
}

void Mesh::addLod(const Lod &lod) {
    p_ptr->m_Lods.push_back(lod);

    setState(ToBeUpdated);
}

void Mesh::setLod(uint32_t index, const Lod &lod) {
    if(index < lodsCount()) {
        p_ptr->m_Lods[index] = lod;
        setState(ToBeUpdated);
    } else {
        addLod(lod);
    }
}
/*!
    \internal
*/
Mesh::Lod *Mesh::getLod(uint32_t lod) const {
    if(lod < lodsCount()) {
        return &p_ptr->m_Lods[lod];
    }
    return nullptr;
}
