#include "fbxconverter.h"

#include "resources/mesh.h"
#include "file.h"
#include "log.h"

#include <bson.h>

#include <cstring>

#include <QFileInfo>
#include <QTime>
#include <QVariantMap>
#include <QUuid>
#include <QDebug>

#include <ofbx.h>

#include "converters/converter.h"
#include "projectmanager.h"

#include "resources/material.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/meshrender.h"

#define HEADER  "Header"
#define DATA    "Data"

static Matrix3 gInvert;

FbxImportSettings::FbxImportSettings() :
        m_UseScale(false),
        m_Scale(1.0f),
        m_Colors(true),
        m_Normals(true),
        m_Animation(false) {

}

bool FbxImportSettings::colors() const {
    return m_Colors;
}
void FbxImportSettings::setColors(bool value) {
    if(m_Colors != value) {
        m_Colors = value;
        emit updated();
    }
}

bool FbxImportSettings::normals() const {
    return m_Normals;
}
void FbxImportSettings::setNormals(bool value) {
    if(m_Normals != value) {
        m_Normals = value;
        emit updated();
    }
}

bool FbxImportSettings::animation() const {
    return m_Animation;
}
void FbxImportSettings::setAnimation(bool value) {
    if(m_Animation != value) {
        m_Animation = value;
        emit updated();
    }
}

bool FbxImportSettings::useScale() const {
    return m_UseScale;
}
void FbxImportSettings::setUseScale(bool value) {
    if(m_UseScale != value) {
        m_UseScale = value;
        emit updated();
    }
}

float FbxImportSettings::customScale() const {
    return m_Scale;
}
void FbxImportSettings::setCustomScale(float value) {
    if(m_Scale != value) {
        m_Scale = value;
        emit updated();
    }
}

class MeshSerial : public Mesh {
public:
    VariantMap saveUserData() const {
        VariantMap result;

        int32_t flag = flags();

        VariantList header;
        header.push_back(flag);
        result[HEADER]  = header;

        VariantList surface;
        surface.push_back(mode());

        for(uint32_t index = 0; index < lodsCount(); index++) {
            Lod *l = getLod(index);

            VariantList lod;
            // Push material
            lod.push_back("{00000000-0402-0000-0000-000000000000}");
            uint32_t vCount = l->vertices.size();
            lod.push_back(static_cast<int32_t>(vCount));
            lod.push_back(static_cast<int32_t>(l->indices.size() / 3));

            { // Required field
                ByteArray buffer;
                buffer.resize(sizeof(Vector3) * vCount);
                memcpy(&buffer[0], &l->vertices[0], sizeof(Vector3) * vCount);
                lod.push_back(buffer);
            }
            { // Required field
                ByteArray buffer;
                buffer.resize(sizeof(uint32_t) * l->indices.size());
                memcpy(&buffer[0], &l->indices[0], sizeof(uint32_t) * l->indices.size());
                lod.push_back(buffer);
            }

            if(flag & ATTRIBUTE_COLOR) { // Optional field
                ByteArray buffer;
                buffer.resize(sizeof(Vector4) * vCount);
                memcpy(&buffer[0], &l->colors[0], sizeof(Vector4) * vCount);
                lod.push_back(buffer);
            }
            if(flag & ATTRIBUTE_UV0) { // Optional field
                ByteArray buffer;
                buffer.resize(sizeof(Vector2) * vCount);
                memcpy(&buffer[0], &l->uv0[0], sizeof(Vector2) * vCount);
                lod.push_back(buffer);
            }
            if(flag & ATTRIBUTE_UV1) { // Optional field
                ByteArray buffer;
                buffer.resize(sizeof(Vector2) * vCount);
                memcpy(&buffer[0], &l->uv1[0], sizeof(Vector2) * vCount);
                lod.push_back(buffer);
            }

            if(flag & ATTRIBUTE_NORMALS) { // Optional field
                ByteArray buffer;
                buffer.resize(sizeof(Vector3) * vCount);
                memcpy(&buffer[0], &l->normals[0], sizeof(Vector3) * vCount);
                lod.push_back(buffer);
            }
            if(flag & ATTRIBUTE_TANGENTS) { // Optional field
                ByteArray buffer;
                buffer.resize(sizeof(Vector3) * vCount);
                memcpy(&buffer[0], &l->tangents[0], sizeof(Vector3) * vCount);
                lod.push_back(buffer);
            }
            if(flag & ATTRIBUTE_ANIMATED) { // Optional field
                {
                    ByteArray buffer;
                    buffer.resize(sizeof(Vector4) * vCount);
                    memcpy(&buffer[0], &l->weights[0], sizeof(Vector4) * vCount);
                    lod.push_back(buffer);
                }
                {
                    ByteArray buffer;
                    buffer.resize(sizeof(Vector4) * vCount);
                    memcpy(&buffer[0], &l->bones[0], sizeof(Vector4) * vCount);
                    lod.push_back(buffer);
                }
            }
            surface.push_back(lod);
        }
        result[DATA] = surface;

        return result;
    }

    void setState(ResourceState state) {
        if(state == Suspend) {
            state = ToBeDeleted;
        }
        Resource::setState(state);
    }

protected:
    friend class FBXConverter;

};

FBXConverter::FBXConverter() {
    gInvert[0] =-1.0;
    gInvert[1] = 0.0;
    gInvert[2] = 0.0;

    gInvert[3] = 0.0;
    gInvert[4] = 0.0;
    gInvert[5] = 1.0;

    gInvert[6] = 0.0;
    gInvert[7] = 1.0;
    gInvert[8] = 0.0;

}

IConverterSettings *FBXConverter::createSettings() const {
    return new FbxImportSettings();
}

uint8_t FBXConverter::convertFile(IConverterSettings *settings) {
    QTime t;
    t.start();

    FbxImportSettings *s = static_cast<FbxImportSettings *>(settings);

    QFile fp(s->source());
    if(fp.open(QIODevice::ReadOnly)) {
        QByteArray data = fp.readAll();
        fp.close();

        ofbx::IScene *scene = ofbx::load(reinterpret_cast<const uint8_t *>(data.constData()),
                                         data.size(),
                                         uint64_t(ofbx::LoadFlags::TRIANGULATE));

        if(!s->useScale()) {
            const ofbx::GlobalSettings *global = scene->getGlobalSettings();
            s->setCustomScale(static_cast<float>(global->UnitScaleFactor) * 0.01f);
        }
        QStringList resources;

        list<Actor *> actors;
        int meshCount = scene->getMeshCount();
        for(int i = 0; i < meshCount; ++i) {
            const ofbx::Mesh *m = scene->getMesh(i);
            Actor *actor = importObject(m, s, resources);
            if(actor) {
                actors.push_back(actor);
            }
        }

        Actor *root;
        if(actors.size() == 1) {
            root = actors.front();
        } else {
            root = Engine::objectCreate<Actor>();
            for(auto it : actors) {
                it->setParent(root);
            }
            Engine::replaceUUID(root, qHash(settings->destination()));
            Engine::replaceUUID(root->transform(), qHash(QString(settings->destination()) + ".Transform"));
        }

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(root));
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
        }

        for(auto it : resources) {
            Engine::unloadResource(it.toStdString(), true);
        }
        Log(Log::INF) << "Mesh imported in:" << t.elapsed() << "msec";

        return 0;
    }
    return 1;
}

Actor *FBXConverter::importObject(const ofbx::Object *object, FbxImportSettings *settings, QStringList &list) {
    Actor *actor = Engine::objectCreate<Actor>();

    ofbx::Vec3 p = object->getLocalTranslation();
    Vector3 pos = gInvert * (Vector3(static_cast<areal>(p.x),
                                     static_cast<areal>(p.y),
                                     static_cast<areal>(p.z)) * settings->customScale());
    actor->transform()->setPosition(pos);

    ofbx::Vec3 r = object->getLocalRotation();
    Vector3 rot = gInvert * Vector3(static_cast<areal>(r.x),
                                    static_cast<areal>(r.y),
                                    static_cast<areal>(r.z));
    actor->transform()->setEuler(rot);

    ofbx::Vec3 s = object->getLocalScaling();
    Vector3 scl = Vector3(static_cast<areal>(s.x),
                          static_cast<areal>(s.y),
                          static_cast<areal>(s.z));
    actor->transform()->setScale(scl);

    QString path(object->name);
    actor->setName(qPrintable(path));

    MeshSerial *mesh = importMesh(static_cast<const ofbx::Mesh *>(object), settings->customScale());
    if(mesh) {
        if(mesh->flags() & Mesh::ATTRIBUTE_ANIMATED) {
            //importAnimation(node, scene);
        }

        QString uuid = settings->subItem(path);
        if(uuid.isEmpty()) {
            uuid = QUuid::createUuid().toString();
        }
        Engine::replaceUUID(actor, qHash(uuid));
        Engine::replaceUUID(actor->transform(), qHash(uuid + ".Transform"));
        settings->setSubItem(path, uuid, IConverter::ContentMesh);
        QFileInfo dst(settings->absoluteDestination());

        QFile file(dst.absolutePath() + "/" + uuid);
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(mesh));
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
        }
        Engine::setResource(mesh, uuid.toStdString());

        MeshRender *render = static_cast<MeshRender *>(actor->addComponent("MeshRender"));
        render->setMesh(mesh);
        Engine::replaceUUID(render, qHash(uuid + ".MeshRender"));

        list.push_back(uuid);

        return actor;
    }
    delete actor;

    return nullptr;
}

MeshSerial *FBXConverter::importMesh(const ofbx::Mesh *m, float scale) {
    MeshSerial *mesh = new MeshSerial;
    mesh->setMode(Mesh::MODE_TRIANGLES);

    const ofbx::Geometry *geom = m->getGeometry();

    Mesh::Lod l;
    l.material = Engine::loadResource<Material>(".embedded/DefaultMesh.mtl");
    // Export
    uint32_t vertexCount = static_cast<uint32_t>(geom->getVertexCount());
    l.vertices.resize(uint32_t(vertexCount));
    vector<uint32_t> hashes(vertexCount);

    const ofbx::Skin *skin = geom->getSkin();
    if(skin) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_ANIMATED);
        l.bones.resize(vertexCount);
        l.weights.resize(vertexCount);
    }

    const ofbx::Vec4 *colors = geom->getColors();
    if(colors) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_COLOR);
        l.colors.resize(uint32_t(vertexCount));
    }

    const ofbx::Vec2 *uvs = geom->getUVs();
    if(uvs) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_UV0);
        l.uv0.resize(uint32_t(vertexCount));
    } else {
        Log(Log::WRN) << "No uv exist";
    }

    const ofbx::Vec3 *normals = geom->getNormals();
    if(normals) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_NORMALS);
        l.normals.resize(uint32_t(vertexCount));
    } else {
        Log(Log::WRN) << "No normals exist";
    }

    const ofbx::Vec3 *tangents = geom->getTangents();
    if(tangents) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_TANGENTS);
        l.tangents.resize(uint32_t(vertexCount));
    } else {
        Log(Log::WRN) << "No tangents exist";
    }

    const ofbx::Vec3 *vertices = geom->getVertices();

    uint32_t indexCount = static_cast<uint32_t>(geom->getIndexCount());
    l.indices.resize(indexCount);

    uint32_t count = 0;

    const int *faceIndices = geom->getFaceIndices();
    for(uint32_t i = 0; i < indexCount; ++i) {
        int index = (i % 3 == 2) ? (-faceIndices[i] - 1) : faceIndices[i];

        ofbx::Vec3 v = vertices[index];
        Vector3 vertex = gInvert * (Vector3(static_cast<areal>(v.x),
                                            static_cast<areal>(v.y),
                                            static_cast<areal>(v.z)) * scale);

        uint32_t hash = 0;

        Vector2 uv;
        if(uvs) {
            ofbx::Vec2 u = uvs[index];
            uv = Vector2(static_cast<areal>(u.x),
                         static_cast<areal>(u.y));

            hash ^= (qHash(u.x) >> 9) ^ qHash(u.y);
        }

        Vector3 normal;
        if(normals) {
            ofbx::Vec3 n = normals[index];
            normal = gInvert * (Vector3(static_cast<areal>(n.x),
                                        static_cast<areal>(n.y),
                                        static_cast<areal>(n.z)));

            hash ^= (qHash(n.x) >> 13) ^ qHash(n.y) ^ (qHash(n.z) >> 7);
        }

        bool create = true;
        for(uint32_t x = 0; x < indexCount; ++x) {
            if(hashes[x] == hash && l.vertices[x] == vertex) {
                l.indices[i] = x;
                create = false;
                break;
            }
        }

        if(create) {
            l.indices[i] = count;
            l.vertices[count] = vertex;

            if(uvs) {
                l.uv0[count] = uv;
            }
            if(normals) {
                l.normals[count] = normal;
            }
            if(tangents) {
                ofbx::Vec3 t = tangents[index];
                l.tangents[count] = gInvert * (Vector3(static_cast<areal>(t.x),
                                                       static_cast<areal>(t.y),
                                                       static_cast<areal>(t.z)));
            }
            if(colors) {
                ofbx::Vec4 c = colors[index];
                l.colors[count] = Vector4(static_cast<areal>(c.x),
                                          static_cast<areal>(c.y),
                                          static_cast<areal>(c.z),
                                          static_cast<areal>(c.w));
            }

            if(mesh->flags() & Mesh::ATTRIBUTE_ANIMATED) {
                int32_t it = 0;
                Vector4 weights, bones;
                for(int32_t b = 0; b < skin->getClusterCount(); b++) {
                    const ofbx::Cluster *cluster = skin->getCluster(b);

                    const int32_t *controllIndices = cluster->getIndices();
                    const double *w = cluster->getWeights();

                    float weight = 0.0f;
                    for(int index = 0; index < cluster->getIndicesCount(); index++) {
                        if(controllIndices[index] == i) {
                            weight = static_cast<areal>(w[index]);
                            break;
                        }
                    }

                    if(weight > 0.0f && i < 4) {
                        bones[it] = b;
                        weights[it] = weight;
                        it++;
                    }
                }

                l.bones[count] = bones;
                l.weights[count] = weights;
            }

            hashes[count] = hash;
            ++count;
        }
    }

    l.vertices.resize(count);

    if(uvs) {
        l.uv0.resize(count);
    }
    if(normals) {
        l.normals.resize(count);
    }
    if(tangents) {
        l.tangents.resize(count);
    }
    if(colors) {
        l.colors.resize(count);
    }

    if(mesh->flags() & Mesh::ATTRIBUTE_ANIMATED) {
        l.bones.resize(count);
        l.weights.resize(count);
    }
    mesh->addLod(l);

    return mesh;
}
/*
void FBXConverter::importAnimation(FbxNode *node, FbxScene *scene) {
    FbxAnimStack *stack = scene->GetCurrentAnimationStack();
    for(int i = 0; i < stack->GetMemberCount(); i++) {
        //FbxAnimLayer *layer = stack->GetMember<FbxAnimLayer>(i);
        //FbxAnimCurve *xPos = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        //FbxAnimCurve *xRot = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        //FbxAnimCurve *xScl = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
    }
}

if(pMesh->type == MESH_ANIMATED) {
    pMesh->jCount = bones.size();
    pMesh->aAnim->priority = new char[pMesh->jCount];

    pMesh->jArray = new joint_data[pMesh->jCount];
    for(int s = 0; s < pMesh->jCount; s++) {
        KFbxNode *bone = bones[s]->GetLink();
        KFbxNode *parent = bone->GetParent();
        int j;
        for(j = 0; j < pMesh->jCount; j++)
            if(bones[j]->GetLink() == parent) break;

        // Getting Joint priority (Using with animation blending)
        int prop = 100;
        pMesh->aAnim->priority[s] = prop;

        if(j == bones.size()) {
            pMesh->jArray[s].parent = 0; // Root Joint
            pMesh->jArray[s].iparent = ROOT;
        } else {
            pMesh->jArray[s].parent = &pMesh->jArray[j];
            pMesh->jArray[s].iparent = j;
        }

        memset(pMesh->jArray[s].name, 0, 32);
        strncpy(pMesh->jArray[s].name, bone->GetName(), 31);

        int proxy = 0;
        pMesh->jArray[s].proxy = proxy;

        int emitter	= 0;
        pMesh->jArray[s].emitter = emitter;
    }
}
*/
