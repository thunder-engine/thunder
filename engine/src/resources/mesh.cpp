#include <resources/mesh.h>

#include <file.h>
#include <log.h>

#define MESH        "Mesh"
#define DEFAULTMESH ".embedded/DefaultMesh.mtl"

Mesh::Mesh() :
        m_Animated(false) {

}

Mesh::~Mesh() {

}

void Mesh::loadUserData(const AVariantMap &data) {
    auto mesh = data.find(MESH);
    if(mesh == data.end()) {
        return;
    }
    for(auto surfaces : (*mesh).second.value<AVariantList>()) {
        // Surface
        Surface s;
        Vector3 obb[2];

        AVariantList surface = surfaces.value<AVariantList>();
        auto x  = surface.begin();
        s.collision     = (*x).toBool();
        x++;
        while(x != surface.end()) {
            Lod l;

            AVariantList lod  = (*x).value<AVariantList>();
            auto y      = lod.begin();
            string path = (*y).toString();
            l.material  = Engine::loadResource<Material>(path.empty() ? DEFAULTMESH : path);
            y++;

            uint32_t vCount = (*y).toInt();
            y++;

            uint32_t tCount = (*y).toInt();
            y++;

            AByteArray data = (*y).toByteArray();
            y++;

            uint32_t offset = 0;
            l.vertices      = VertexVector(vCount, Vertex());

            for(int vertex = 0; vertex < vCount; vertex++) {
                Vertex *v   = &l.vertices[vertex];

                v->weight   = Vector4();
                v->index    = Vector4();

                // Reading mesh
                uint8_t wCount  = 0;
                if(m_Animated) { // Animated mesh
                    memcpy(&wCount, &data[offset], sizeof(char));
                    offset += sizeof(char);
                    for(int k = 0; (k < wCount) && (k < 4); k++) {
                        unsigned short bone;
                        memcpy(&bone, &data[offset],  sizeof(short));
                        offset += sizeof(short);
                        memcpy(&v->weight[k], &data[offset],  sizeof(float));
                        offset += sizeof(float);
                    }
                }

                Vector3 xyz;
                memcpy(&xyz,    &data[offset], sizeof(Vector3));
                offset += sizeof(Vector3);
                v->xyz      = Vector4(xyz.x, xyz.y, xyz.z, wCount);

                memcpy(&v->t,   &data[offset], sizeof(Vector3));
                offset += sizeof(Vector3);
                memcpy(&v->n,   &data[offset], sizeof(Vector3));
                offset += sizeof(Vector3);

                memcpy(&v->uv0, &data[offset], sizeof(Vector2));
                offset += sizeof(Vector2);

                obb[0].x = MIN(obb[0].x, v->xyz.x);
                obb[0].y = MIN(obb[0].y, v->xyz.y);
                obb[0].z = MIN(obb[0].z, v->xyz.z);

                obb[1].x = MAX(obb[1].x, v->xyz.x);
                obb[1].y = MAX(obb[1].y, v->xyz.y);
                obb[1].z = MAX(obb[1].z, v->xyz.z);
            }
            l.indices   = IndexVector(tCount * 3, 0);
            memcpy(&l.indices[0],   &data[offset],  sizeof(int) * l.indices.size());
            offset += sizeof(int) * l.indices.size();

            s.lods.push_back(l);

            x++;
        }

        s.obb.setBox(obb[0], obb[1]);

        m_Surfaces.push_back(s);
    }

    if(m_Animated) { // Animated mesh
        // Reading skeletal arcitecture
/*
        short parent;
        short jCount;

        file->_fread(&jCount, sizeof(short), 1, fp);

        bind = joint_vector(jCount);
        for(int j = 0; j < jCount; j++) {
            file->_fread(&parent, sizeof(short), 1, fp);

            joint_data *joint   = &bind[j];
            if(parent == -1)
                joint->parent   = 0;
            else
                joint->parent   = &bind[parent];

            joint->iparent      = parent;

            file->_fread(joint->name,          32, 1, fp);
            file->_fread(&joint->proxy,        sizeof(char), 1, fp);
            file->_fread(&joint->emitter,      sizeof(char), 1, fp);
        }
        // Reading bind pose
        for(int j = 0; j < jCount; j++) {
            joint_data *joint   = &bind[j];

            //file->_fread(&joint->quaternion,   sizeof(Quaternion ),    1, fp);
            file->_fread(&joint->vector,       sizeof(Vector3),      1, fp);
            // Matrix calculation
            Matrix4 translate;
            translate  = translate.translate(joint->vector);

            //joint->rotation    = joint->quaternion.toMatrix();
            joint->transform   = translate * joint->rotation;
        }
*/
    }
}

AVariantMap Mesh::saveUserData() const {
    AVariantMap result;

    AVariantList surfaces;
    for(auto s : m_Surfaces) {
        AVariantList surface;
        surface.push_back(s.collision);

        for(auto l : s.lods) {
            AVariantList lod;
            string path = Engine::reference(l.material);
            lod.push_back(path);
            lod.push_back((int)l.vertices.size());
            lod.push_back((int)l.indices.size() / 3);

            AByteArray buffer;
            buffer.resize(sizeof(Vertex) * l.vertices.size() +
                          sizeof(int) * l.indices.size());

            uint32_t offset = 0;
            for(int vertex = 0; vertex < l.vertices.size(); vertex++) {
                const Vertex *v = &l.vertices[vertex];
                if(m_Animated) {	// Animated mesh
                    char wCount = 4;
                    memcpy(&buffer[offset], &wCount, sizeof(char));
                    offset += sizeof(char);
                    for(int k = 0; k < wCount; k++) {
                        int bone    = v->index[k];
                        memcpy(&buffer[offset], &bone,  sizeof(short));
                        offset += sizeof(short);
                        memcpy(&buffer[offset], &v->weight.v[k],    sizeof(float));
                        offset += sizeof(float);
                    }
                }
                memcpy(&buffer[offset], &v->xyz,    sizeof(Vector3));
                offset += sizeof(Vector3);
                memcpy(&buffer[offset], &v->t,      sizeof(Vector3));
                offset += sizeof(Vector3);
                memcpy(&buffer[offset], &v->n,      sizeof(Vector3));
                offset += sizeof(Vector3);

                memcpy(&buffer[offset], &v->uv0,    sizeof(Vector2));
                offset += sizeof(Vector2);
            }

            memcpy(&buffer[offset],     &l.indices[0],  sizeof(int) * l.indices.size());
            offset += sizeof(int) * l.indices.size();

            lod.push_back(AByteArray(buffer.begin(), buffer.begin() + offset));
            surface.push_back(lod);
        }
        surfaces.push_back(surface);
    }
    result[MESH]    = surfaces;

    if(m_Animated) {
/*
        // Save bone structure
        short jCount    = bind.size();
        file->_fwrite(&jCount, sizeof(short), 1, fp);
        for(int j = 0; j < jCount; j++) {
            joint_data *joint   = &bind[j];
            file->_fwrite(&joint->iparent,	sizeof(short), 1, fp);
            // Save name of bone
            file->_fwrite(joint->name, 32, 1, fp);

            file->_fwrite(&joint->proxy,    sizeof(char), 1, fp);
            file->_fwrite(&joint->emitter,	sizeof(char), 1, fp);
        }
        // Save bind pose
        for(int j = 0; j < jCount; j++) {
            joint_data *joint   = &bind[j];
            //file->_fwrite(&joint->quaternion,  sizeof(Quaternion ),    1, fp);
            file->_fwrite(&joint->vector,      sizeof(Vector3),      1, fp);
        }
*/
    }
    return result;
}

bool Mesh::isAnimated() const {
    return m_Animated;
}
