#include "fbxconverter.h"

#include "resources/mesh.h"
#include "file.h"
#include "log.h"

#include <bson.h>
#include <json.h>
#include <variantanimation.h>

#include <cstring>

#include <QFileInfo>
#include <QTime>
#include <QVariantMap>
#include <QUuid>
#include <QDebug>

#include <ofbx.h>

#include "converters/converter.h"
#include "animconverter.h"
#include "projectmanager.h"

#include "resources/material.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/armature.h"
#include "components/meshrender.h"
#include "components/skinnedmeshrender.h"

#define HEADER  "Header"
#define DATA    "Data"

#define FORMAT_VERSION 1

static Matrix3 gInvert;

FbxImportSettings::FbxImportSettings() :
        m_UseScale(false),
        m_Scale(1.0f),
        m_Colors(true),
        m_Normals(true),
        m_Animation(false) {

    setVersion(FORMAT_VERSION);
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

class PoseSerial : public Pose {
    VariantMap saveUserData() const {
        VariantMap result;

        VariantList data;
        for(uint32_t i = 0; i < size(); i++) {
            VariantList attribs;

            const Bone *b = bone(i);

            attribs.push_back(b->position);
            attribs.push_back(b->rotation);
            attribs.push_back(b->scale);

            data.push_back(attribs);
        }
        result[DATA] = data;

        return result;
    }
};

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
            if(flag & ATTRIBUTE_SKINNED) { // Optional field
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
    QTime time;
    time.start();

    FbxImportSettings *fbxSettings = static_cast<FbxImportSettings *>(settings);

    QFile fp(fbxSettings->source());
    if(fp.open(QIODevice::ReadOnly)) {
        QByteArray data = fp.readAll();
        fp.close();

        ofbx::IScene *scene = ofbx::load(reinterpret_cast<const uint8_t *>(data.constData()),
                                         data.size(),
                                         uint64_t(ofbx::LoadFlags::TRIANGULATE));

        if(!fbxSettings->useScale()) {
            const ofbx::GlobalSettings *global = scene->getGlobalSettings();
            fbxSettings->setCustomScale(static_cast<float>(global->UnitScaleFactor) * 0.01f);
        }
        QStringList resources;

        list<Actor *> actors;
        std::list<FBXObjectsList> skins;
        int meshCount = scene->getMeshCount();
        for(int i = 0; i < meshCount; ++i) {
            const ofbx::Mesh *m = scene->getMesh(i);

            Actor *actor = importObject(m, nullptr, fbxSettings);
            if(actor) {
                actors.push_back(actor);
            }

            MeshSerial *mesh = importMesh(m, fbxSettings->customScale());
            if(mesh) {
                if(mesh->flags() & Mesh::ATTRIBUTE_SKINNED) {
                    Vector3Vector vertices = mesh->vertices(0);
                    Matrix4 t;
                    t.translate(actor->transform()->worldPosition() * 0.1f);
                    for(uint32_t v = 0; v < vertices.size(); v++) {
                        //vertices[v] = gInvert * (actor->transform()->worldTransform() * (vertices[v] * fbxSettings->customScale()));

                        vertices[v] = (gInvert * (vertices[v] * fbxSettings->customScale()));
                    }
                    mesh->setVertices(0, vertices);
                }

                QString uuid = saveData(Bson::save(Engine::toVariant(mesh)), actor->name().c_str(), IConverter::ContentMesh, fbxSettings);
                Engine::replaceUUID(actor, qHash(uuid));
                Engine::replaceUUID(actor->transform(), qHash(uuid + ".Transform"));
                Engine::setResource(mesh, uuid.toStdString());

                if(mesh->flags() & Mesh::ATTRIBUTE_SKINNED) {
                    SkinnedMeshRender *render = static_cast<SkinnedMeshRender *>(actor->addComponent("SkinnedMeshRender"));
                    render->setMesh(mesh);
                    Engine::replaceUUID(render, qHash(uuid + ".SkinnedMeshRender"));

                    FBXObjectsList bones;

                    const ofbx::Skin *skin = m->getGeometry()->getSkin();
                    int clusterCount = skin->getClusterCount();
                    for(int i = 0; i < clusterCount; i++) {
                        const ofbx::Cluster *cluster = skin->getCluster(i);
                        if(cluster) {
                            bones.push_back(cluster->getLink());
                        }
                    }
                    skins.push_back(bones);
                } else {
                    MeshRender *render = static_cast<MeshRender *>(actor->addComponent("MeshRender"));
                    render->setMesh(mesh);
                    Engine::replaceUUID(render, qHash(uuid + ".MeshRender"));
                }
                resources.push_back(uuid);
            }
        }
        skins.unique();

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

            if(!skins.empty()) {
                PoseSerial *pose = new PoseSerial;
                pose->setName("Pose");

                QMap<const ofbx::Object *, Actor *> boneMap;
                Armature *armature = nullptr;
                for(auto it : skins.front()) {
                    Actor *parent = root;
                    bool isRootBone = false;
                    auto bone = boneMap.find(it->getParent());
                    if(bone != boneMap.end()) {
                        parent = bone.value();
                    } else {
                        const ofbx::Object *obj = it->getParent();
                        if(obj != scene->getRoot()) {
                            parent = importObject(obj, root, fbxSettings);
                        }
                        isRootBone = true;
                    }

                    Actor *item = importObject(it, parent, fbxSettings);
                    if(isRootBone) {
                        armature = dynamic_cast<Armature *>(item->addComponent("Armature"));
                        armature->setBindPose(pose);
                    }
                    boneMap[it] = item;

                    Transform *t = item->transform();
                    Pose::Bone b;
                    b.position = t->position();
                    b.rotation = t->euler();
                    b.scale = t->scale();

                    pose->addBone(b);
                }

                QString uuid = saveData(Bson::save(Engine::toVariant(pose)), pose->name().c_str(), IConverter::ContentPose, fbxSettings);

                Engine::setResource(pose, uuid.toStdString());
                resources.push_back(uuid);

                for(auto it : actors) {
                    SkinnedMeshRender *skinned = dynamic_cast<SkinnedMeshRender *>(it->component("SkinnedMeshRender"));
                    if(skinned) {
                        skinned->setArmature(armature);
                    }
                }
                importAnimation(boneMap, scene, fbxSettings);
            }
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
        Log(Log::INF) << "Mesh imported in:" << time.elapsed() << "msec";

        settings->setCurrentVersion(settings->version());

        return 0;
    }
    return 1;
}

Actor *FBXConverter::importObject(const ofbx::Object *object, Actor *parent, FbxImportSettings *settings) {
    Actor *actor = Engine::objectCreate<Actor>();

    actor->setParent(parent);
    actor->setName(object->name);

    ofbx::Vec3 p = object->getLocalTranslation();
    Vector3 pos = (Vector3(static_cast<areal>(p.x),
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

    return actor;
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
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_SKINNED);
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

        Vector3 vertex;
        if(mesh->flags() & Mesh::ATTRIBUTE_SKINNED) {
            vertex = Vector3(static_cast<areal>(v.x),
                             static_cast<areal>(v.y),
                             static_cast<areal>(v.z));
        } else {
            vertex = gInvert * (Vector3(static_cast<areal>(v.x),
                                        static_cast<areal>(v.y),
                                        static_cast<areal>(v.z)) * scale);
        }

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

            if(mesh->flags() & Mesh::ATTRIBUTE_SKINNED) {
                int32_t it = 0;
                Vector4 weights(0.0f), bones(0.0f);
                for(int32_t b = 0; b < skin->getClusterCount(); b++) {
                    const ofbx::Cluster *cluster = skin->getCluster(b);
                    if(cluster->getIndicesCount() > 0) {
                        const int32_t *controllIndices = cluster->getIndices();
                        const double *w = cluster->getWeights();

                        float weight = 0.0f;
                        int indicesCount = cluster->getIndicesCount();
                        for(int boneIndex = 0; boneIndex < indicesCount; boneIndex++) {
                            int controll = controllIndices[boneIndex];
                            if(controll == i) {
                                weight = static_cast<areal>(w[boneIndex]);
                                break;
                            }
                        }

                        if(weight > 0.0f && it < 4) {
                            bones[it] = b;
                            weights[it] = weight;
                            it++;
                        }
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

    if(mesh->flags() & Mesh::ATTRIBUTE_SKINNED) {
        l.bones.resize(count);
        l.weights.resize(count);
    }
    mesh->addLod(l);

    return mesh;
}

static AnimationCurve importCurve(const ofbx::AnimationCurve *curve, float scale) {
    uint32_t count = uint32_t(curve->getKeyCount());
    const ofbx::i64 *keys = curve->getKeyTime();
    const float *values = curve->getKeyValue();

    AnimationCurve result;
    result.m_Keys.resize(count);
    for(uint32_t i = 0; i < count; i++) {
        AnimationCurve::KeyFrame key;
        key.m_Type = AnimationCurve::KeyFrame::Linear;
        key.m_Value = values[i] * scale;
        double position = ofbx::fbxTimeToSeconds(keys[i]);
        key.m_Position = uint32_t(position * 1000.0);

        result.m_Keys[i] = key;
    }
    return result;
}

static AnimationClip::Track importTrack(const ofbx::AnimationCurveNode *node, const string &path, const char *property, Vector3 factor) {
    AnimationClip::Track track;

    track.path = path;
    track.property = property;

    const ofbx::AnimationCurve *x = node->getCurve(0);
    const ofbx::AnimationCurve *y = node->getCurve(1);
    const ofbx::AnimationCurve *z = node->getCurve(2);

    if(x) track.curves[0] = importCurve(x, factor.x);
    if(y) track.curves[1] = importCurve(y, factor.y);
    if(z) track.curves[2] = importCurve(z, factor.z);

    return track;
}

static string pathTo(Object *src, Object *dst) {
    string result;
    if(src != dst) {
        string parent = pathTo(src, dst->parent());
        if(!parent.empty()) {
            result += parent + "/";
        }
        result += dst->name();
    }
    return result;
}

static bool compare(const AnimationClip::Track &left, const AnimationClip::Track &right) {
    return left.path > right.path;
}

void FBXConverter::importAnimation(const QMap<const ofbx::Object *, Actor *> &bones, ofbx::IScene *scene, FbxImportSettings *settings) {
    const ofbx::AnimationStack *stack = scene->getAnimationStack(0);
    const ofbx::AnimationLayer *layer = stack->getLayer(0);

    if(layer) {
        AnimationClipSerial clip;
        clip.setName("AnimationClip");

        for(auto it : bones.keys()) {
            const ofbx::AnimationCurveNode *t = layer->getCurveNode(*it, "Lcl Translation");
            const ofbx::AnimationCurveNode *r = layer->getCurveNode(*it, "Lcl Rotation");
            const ofbx::AnimationCurveNode *s = layer->getCurveNode(*it, "Lcl Scaling");

            string path = pathTo(nullptr, bones.value(it)->transform());
            if(t) {
                const float scale = (settings) ? settings->customScale() : 1.0f;
                clip.m_Tracks.push_back(importTrack(t, path, "Position", Vector3(scale)));
            }

            if(r) {
                clip.m_Tracks.push_back(importTrack(r, path, "Rotation", gInvert * Vector3(1.0f)));
            }

            if(s) {
                clip.m_Tracks.push_back(importTrack(s, path, "Scale", Vector3(1.0f)));
            }
        }
        clip.m_Tracks.sort(compare);

        saveData(Bson::save(Engine::toVariant(&clip)), clip.name().c_str(), IConverter::ContentAnimation, settings);
    }
}

QString FBXConverter::saveData(const ByteArray &data, const QString &path, int32_t type, FbxImportSettings *settings) {
    QString uuid = settings->subItem(path);
    if(uuid.isEmpty()) {
        uuid = QUuid::createUuid().toString();
    }
    settings->setSubItem(path, uuid, type);
    QFileInfo dst(settings->absoluteDestination());

    QFile file(dst.absolutePath() + "/" + uuid);
    if(file.open(QIODevice::WriteOnly)) {
        file.write(reinterpret_cast<const char *>(&data[0]), data.size());
        file.close();
    }
    return uuid;
}
