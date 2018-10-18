#include <resources/mesh.h>

#include <file.h>
#include <log.h>

#if __linux__
#include <cstring>
#endif

#define HEADER      "Header"
#define DATA        "Data"
#define DEFAULTMESH ".embedded/DefaultMesh.mtl"

Mesh::Mesh() :
        m_Dynamic(false),
        m_Flags(0) {

}

Mesh::~Mesh() {
    clear();
}

void Mesh::apply() {
    Vector3 aabb[2];
    for(Surface it : m_Surfaces) {
        Vector3 min;
        Vector3 max;
        it.aabb.box(min, max);
        aabb[0].x   = MIN(aabb[0].x, min.x);
        aabb[0].y   = MIN(aabb[0].y, min.y);
        aabb[0].z   = MIN(aabb[0].z, min.z);

        aabb[1].x   = MAX(aabb[1].x, max.x);
        aabb[1].y   = MAX(aabb[1].y, max.y);
        aabb[1].z   = MAX(aabb[1].z, max.z);
    }
    m_Box.setBox(aabb[0], aabb[1]);
}

void Mesh::clear() {
    m_Surfaces.clear();
}

void Mesh::makeDynamic() {
    m_Dynamic   = true;
}

void Mesh::loadUserData(const VariantMap &data) {
    clear();

    auto it = data.find(HEADER);
    if(it != data.end()) {
        VariantList header  = (*it).second.value<VariantList>();

        auto i      = header.begin();
        m_Flags     = (*i).toInt();
    }

    auto mesh = data.find(DATA);
    if(mesh != data.end()) {
        for(auto surfaces : (*mesh).second.value<VariantList>()) {
            // Surface
            Surface s;
            Vector3 bb[2];

            VariantList surface = surfaces.value<VariantList>();
            auto x  = surface.begin();
            s.collision = (*x).toBool();
            x++;
            s.mode      = (Mesh::Modes)(*x).toInt();
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
                    l.vertices  = vector<Vector3>(vCount);
                    memcpy(&l.vertices[0],  &data[0], sizeof(Vector3) * vCount);
                    for(uint32_t i = 0; i < vCount; i++) {
                        bb[0].x = MIN(bb[0].x, l.vertices[i].x);
                        bb[0].y = MIN(bb[0].y, l.vertices[i].y);
                        bb[0].z = MIN(bb[0].z, l.vertices[i].z);

                        bb[1].x = MAX(bb[1].x, l.vertices[i].x);
                        bb[1].y = MAX(bb[1].y, l.vertices[i].y);
                        bb[1].z = MAX(bb[1].z, l.vertices[i].z);
                    }
                }
                { // Required field
                    data    = (*y).toByteArray();
                    y++;
                    l.indices = IndexVector(tCount * 3);
                    memcpy(&l.indices[0], &data[0],  sizeof(uint32_t) * tCount * 3);
                }
                if(m_Flags & ATTRIBUTE_COLOR) { // Optional field
                    data    = (*y).toByteArray();
                    y++;
                    l.colors    = vector<Vector4>(vCount);
                    memcpy(&l.colors[0],    &data[0],  sizeof(Vector4) * vCount);
                }
                if(m_Flags & ATTRIBUTE_UV0) { // Optional field
                    data    = (*y).toByteArray();
                    y++;
                    l.uv0   = vector<Vector2>(vCount);
                    memcpy(&l.uv0[0],       &data[0],  sizeof(Vector2) * vCount);
                }
                if(m_Flags & ATTRIBUTE_UV1) { // Optional field
                    data    = (*y).toByteArray();
                    y++;
                    l.uv1   = vector<Vector2>(vCount);
                    memcpy(&l.uv1[0],       &data[0],  sizeof(Vector2) * vCount);
                }
                if(m_Flags & ATTRIBUTE_NORMALS) { // Optional field
                    data    = (*y).toByteArray();
                    y++;
                    l.normals   = vector<Vector3>(vCount);
                    memcpy(&l.normals[0],   &data[0],  sizeof(Vector3) * vCount);
                }
                if(m_Flags & ATTRIBUTE_TANGENTS) { // Optional field
                    data    = (*y).toByteArray();
                    y++;
                    l.tangents  = vector<Vector3>(vCount);
                    memcpy(&l.tangents[0],  &data[0],  sizeof(Vector3) * vCount);
                }
                if(m_Flags & ATTRIBUTE_ANIMATED) { // Optional field
                    data    = (*y).toByteArray();
                    y++;
                    l.weights   = vector<Vector4>(vCount);
                    memcpy(&l.weights[0],   &data[0],  sizeof(Vector4) * vCount);

                    data    = (*y).toByteArray();
                    y++;
                    l.bones = vector<Vector4>(vCount);
                    memcpy(&l.bones[0],   &data[0],  sizeof(Vector4) * vCount);
                }
                s.lods.push_back(l);

                x++;
            }
            s.aabb.setBox(bb[0], bb[1]);

            addSurface(s);
        }
    }

    apply();
}

Material *Mesh::material(uint32_t surface, uint32_t lod) const {
    if(lod < lodsCount(surface)) {
        return m_Surfaces[surface].lods[lod].material;
    }
    return nullptr;
}

Vector3Vector Mesh::vertices(uint32_t surface, uint32_t lod) const {
    if(lod < lodsCount(surface)) {
        return m_Surfaces[surface].lods[lod].vertices;
    }
    return Vector3Vector();
}

Mesh::IndexVector Mesh::indices(uint32_t surface, uint32_t lod) const {
    if(lod < lodsCount(surface)) {
        return m_Surfaces[surface].lods[lod].indices;
    }
    return Mesh::IndexVector();
}

uint32_t Mesh::surfacesCount() const {
    return m_Surfaces.size();
}

uint32_t Mesh::lodsCount(uint32_t surface) const {
    if(surface < surfacesCount()) {
        return m_Surfaces[surface].lods.size();
    }
    return 0;
}

uint32_t Mesh::vertexCount(uint32_t surface, uint32_t lod) const {
    if(lod < lodsCount(surface)) {
        return m_Surfaces[surface].lods[lod].vertices.size();
    }
    return 0;
}

uint32_t Mesh::indexCount(uint32_t surface, uint32_t lod) const {
    if(lod < lodsCount(surface)) {
        return m_Surfaces[surface].lods[lod].indices.size();
    }
    return 0;
}

AABBox Mesh::bound() const {
    return m_Box;
}

AABBox Mesh::bound(uint32_t surface) const {
    if(surface < surfacesCount()) {
        return m_Surfaces[surface].aabb;
    }
    return AABBox();
}

Mesh::Modes Mesh::mode(uint32_t surface) const {
    if(surface < surfacesCount()) {
        return m_Surfaces[surface].mode;
    }
    return MODE_TRIANGLES;
}

uint8_t Mesh::flags() const {
    return m_Flags;
}

void Mesh::setFlags(uint8_t flags) {
    m_Flags = flags;
}

void Mesh::addSurface(const Surface &surface) {
    m_Surfaces.push_back(surface);
}

void Mesh::setSurface(uint32_t index, Surface &surface) {
    if(index < surfacesCount()) {
        m_Surfaces[index]   = surface;
    } else {
        addSurface(surface);
    }
}
