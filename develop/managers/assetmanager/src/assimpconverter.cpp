#include "assimpconverter.h"

#include <QFileInfo>
#include <QTime>
#include <QVariantMap>
#include <QUuid>
#include <QDebug>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "bson.h"
#include "log.h"

#include "components/actor.h"
#include "components/armature.h"
#include "components/transform.h"
#include "components/skinnedmeshrender.h"
#include "components/meshrender.h"

#include "resources/mesh.h"
#include "resources/material.h"

#include "systems/resourcesystem.h"

#include "animconverter.h"

#define HEADER  "Header"
#define DATA    "Data"

#define FORMAT_VERSION 1

int32_t indexOf(const aiBone *item, const BonesList &list) {
    int i = 0;
    for(auto it : list) {
        if(it->mName == item->mName) {
            return i;
        }
        i++;
    }
    return -1;
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

AssimpImportSettings::AssimpImportSettings() :
        m_pRootActor(nullptr),
        m_pRootBone(nullptr),
        m_UseScale(false),
        m_Scale(1.0f),
        m_Colors(true),
        m_Normals(true),
        m_Animation(false),
        m_Filter(Keyframe_Reduction),
        m_PositionError(0.5f),
        m_RotationError(0.5f),
        m_ScaleError(0.5f) {

    setVersion(FORMAT_VERSION);
}

bool AssimpImportSettings::colors() const {
    return m_Colors;
}
void AssimpImportSettings::setColors(bool value) {
    if(m_Colors != value) {
        m_Colors = value;
        emit updated();
    }
}

bool AssimpImportSettings::normals() const {
    return m_Normals;
}
void AssimpImportSettings::setNormals(bool value) {
    if(m_Normals != value) {
        m_Normals = value;
        emit updated();
    }
}

bool AssimpImportSettings::animation() const {
    return m_Animation;
}
void AssimpImportSettings::setAnimation(bool value) {
    if(m_Animation != value) {
        m_Animation = value;
        emit updated();
    }
}

AssimpImportSettings::Compression AssimpImportSettings::filter() const {
    return m_Filter;
}
void AssimpImportSettings::setFilter(Compression value) {
    if(m_Filter != value) {
        m_Filter = value;
        emit updated();
    }
}

bool AssimpImportSettings::useScale() const {
    return m_UseScale;
}
void AssimpImportSettings::setUseScale(bool value) {
    if(m_UseScale != value) {
        m_UseScale = value;
        emit updated();
    }
}

float AssimpImportSettings::customScale() const {
    return m_Scale;
}
void AssimpImportSettings::setCustomScale(float value) {
    if(m_Scale != value) {
        m_Scale = value;
        emit updated();
    }
}

float AssimpImportSettings::positionError() const {
    return m_PositionError;
}
void AssimpImportSettings::setPositionError(float value) {
    if(m_PositionError != value) {
        m_PositionError = value;
        emit updated();
    }
}

float AssimpImportSettings::rotationError() const {
    return m_RotationError;
}
void AssimpImportSettings::setRotationError(float value) {
    if(m_RotationError != value) {
        m_RotationError = value;
        emit updated();
    }
}

float AssimpImportSettings::scaleError() const {
    return m_ScaleError;
}
void AssimpImportSettings::setScaleError(float value) {
    if(m_ScaleError != value) {
        m_ScaleError = value;
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

};

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
            attribs.push_back(int(b->index));

            data.push_back(attribs);
        }
        result[DATA] = data;

        return result;
    }
};

AssimpConverter::AssimpConverter() {

}

IConverterSettings *AssimpConverter::createSettings() const {
    return new AssimpImportSettings();
}

uint8_t AssimpConverter::convertFile(IConverterSettings *settings) {
    QTime time;
    time.start();

    AssimpImportSettings *fbxSettings = static_cast<AssimpImportSettings *>(settings);

    fbxSettings->m_Renders.clear();
    fbxSettings->m_Resources.clear();
    fbxSettings->m_Bones.clear();
    fbxSettings->m_Actors.clear();
    fbxSettings->m_pRootActor = nullptr;
    fbxSettings->m_pRootBone = nullptr;
    fbxSettings->m_Flip = false;

    const aiScene *scene = aiImportFile(fbxSettings->source(), aiProcessPreset_TargetRealtime_MaxQuality);
    if(scene) {
        aiMetadata *meta = scene->mMetaData;
        for(uint32_t m = 0; m < meta->mNumProperties; m++) {
            aiMetadataEntry *entry = &meta->mValues[m];
            if(meta->mKeys[m] == aiString("UnitScaleFactor")) {
                if(!fbxSettings->useScale()) {
                    float value = *(reinterpret_cast<double *>(entry->mData)) * 0.01f;
                    fbxSettings->setCustomScale(value);
                }
            } else if(meta->mKeys[m] == aiString("UpAxis")) {
                int32_t value = *(reinterpret_cast<int32_t *>(entry->mData));
                if(value == 2) { // The UpAxis is Z need to switch to Y
                    fbxSettings->m_Flip = true;
                }
            }
        }

        for(uint32_t m = 0; m < scene->mNumMeshes; m++) {
            aiMesh *mesh = scene->mMeshes[m];
            for(uint32_t b = 0; b < mesh->mNumBones; b++) {
                aiBone *bone = mesh->mBones[b];
                if(indexOf(bone, fbxSettings->m_Bones) == -1) {
                    fbxSettings->m_Bones.push_back(bone);
                }
            }
        }

        Actor *root = importObject(scene, scene->mRootNode, nullptr, fbxSettings);

        if(!fbxSettings->m_Bones.empty()) {
            importPose(fbxSettings);
        } else {
            if(fbxSettings->m_Renders.size() == 1) {
                root = static_cast<Component *>(fbxSettings->m_Renders.front())->actor();
                root->transform()->setPosition(Vector3());
                root->transform()->setEuler(Vector3());
                root->transform()->setScale(Vector3(1.0f));
            }
        }

        if(scene->HasAnimations() && fbxSettings->animation()) {
            importAnimation(scene, fbxSettings);
        }

        aiReleaseImport(scene);

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(root));
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
        }

        for(auto it : fbxSettings->m_Resources) {
            Engine::unloadResource(it.toStdString());
        }
        Engine::resourceSystem()->update(nullptr);
        Log(Log::INF) << "Mesh imported in:" << time.elapsed() << "msec";

        settings->setCurrentVersion(settings->version());

        return 0;
    }
    return 1;
}

Actor *importObjectHelper(const aiScene *scene, const aiNode *element, const aiMatrix4x4 &p, Actor *parent, AssimpImportSettings *fbxSettings) {
    string name = element->mName.C_Str();

    if(name.find("$") != string::npos) {
        for(uint32_t c = 0; c < element->mNumChildren; c++) {
            importObjectHelper(scene, element->mChildren[c], p * element->mTransformation, parent, fbxSettings);
        }
    } else {
        Actor *actor = Engine::objectCreate<Actor>(name, parent);

        if(fbxSettings->m_pRootActor == nullptr) {
            fbxSettings->m_pRootActor = actor;
        }

        if(fbxSettings->m_pRootBone == nullptr) {
            for(auto it : fbxSettings->m_Bones) {
                if(it->mName == element->mName) {
                    fbxSettings->m_pRootBone = parent;
                    break;
                }
            }
        }

        fbxSettings->m_Actors[actor->name()] = actor;

        aiMatrix4x4 t = p * element->mTransformation;

        aiVector3D scale, euler, position;
        t.Decompose(scale, euler, position);

        Vector3 pos = Vector3(position.x, position.y, position.z) * fbxSettings->customScale();
        if(fbxSettings->m_Flip) {
            pos = Vector3(-pos.x, pos.z, pos.y);
        }

        actor->transform()->setPosition(pos);
        actor->transform()->setEuler(Vector3(euler.x, euler.y, euler.z) * RAD2DEG);
        actor->transform()->setScale(Vector3(scale.x, scale.y, scale.z));

        QString uuid = actor->name().c_str();
        if(element->mNumMeshes) {
            uint32_t index = element->mMeshes[element->mNumMeshes - 1];
            const aiMesh *mesh = scene->mMeshes[index];

            MeshSerial *result = AssimpConverter::importMesh(mesh, actor, fbxSettings);
            if(result) {
                uuid = AssimpConverter::saveData(Bson::save(Engine::toVariant(result)), actor->name().c_str(), IConverter::ContentMesh, fbxSettings);
                Engine::setResource(result, uuid.toStdString());

                if(mesh->HasBones()) {
                    SkinnedMeshRender *render = static_cast<SkinnedMeshRender *>(actor->addComponent("SkinnedMeshRender"));
                    Engine::replaceUUID(render, qHash(uuid + ".SkinnedMeshRender"));

                    render->setMesh(result);
                    fbxSettings->m_Renders.push_back(render);
                } else {
                    MeshRender *render = static_cast<MeshRender *>(actor->addComponent("MeshRender"));
                    Engine::replaceUUID(render, qHash(uuid + ".MeshRender"));

                    render->setMesh(result);
                    fbxSettings->m_Renders.push_back(render);
                }

                fbxSettings->m_Resources.push_back(uuid);
            }
        }
        Engine::replaceUUID(actor, qHash(uuid));
        Engine::replaceUUID(actor->transform(), qHash(uuid + ".Transform"));

        for(uint32_t c = 0; c < element->mNumChildren; c++) {
            aiMatrix4x4 m;
            importObjectHelper(scene, element->mChildren[c], m, actor, fbxSettings);
        }

        return actor;
    }
    return nullptr;
}

Actor *AssimpConverter::importObject(const aiScene *scene, const aiNode *element, Actor *parent, AssimpImportSettings *fbxSettings) {
    aiMatrix4x4 m;
    return importObjectHelper(scene, element, m, parent, fbxSettings);
}

MeshSerial *AssimpConverter::importMesh(const aiMesh *item, Actor *parent, AssimpImportSettings *fbxSettings) {
    MeshSerial *mesh = new MeshSerial;
    mesh->setMode(Mesh::MODE_TRIANGLES);

    Mesh::Lod l;
    l.material = Engine::loadResource<Material>(".embedded/DefaultMesh.mtl");
    // Export
    uint32_t vertexCount = item->mNumVertices;
    l.vertices.resize(vertexCount);

    Vector3 m;
    if(item->HasBones()) {
        Transform *t = parent->transform();
        Matrix4 rot;
        rot.rotate(t->worldEuler());

        Vector3 s = t->worldScale();
        Vector3 p = t->worldPosition();

        m = rot.inverse() * Vector3(p.x / s.x, p.y / s.y, p.z / s.z);
    }

    for(uint32_t v = 0; v < vertexCount; v++) {
        Vector3 pos = Vector3(item->mVertices[v].x, item->mVertices[v].y, item->mVertices[v].z) * fbxSettings->customScale();
        if(fbxSettings->m_Flip) {
            pos = Vector3(-pos.x, pos.z, pos.y);
        }
        l.vertices[v] = m + pos;
    }

    if(item->HasVertexColors(0)) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_COLOR);
        l.colors.resize(vertexCount);
        memcpy(&l.colors[0], item->mColors[0], sizeof(Vector4) * vertexCount);
    }

    if(item->HasTextureCoords(0)) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_UV0);
        l.uv0.resize(vertexCount);
        aiVector3D *uv = item->mTextureCoords[0];
        for(uint32_t u = 0; u < vertexCount; u++) {
            l.uv0[u] = Vector2(uv[u].x, uv[u].y);
        }
    } else {
        Log(Log::WRN) << "No uv exist";
    }

    if(item->HasNormals()) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_NORMALS);
        l.normals.resize(vertexCount);
        for(uint32_t n = 0; n < vertexCount; n++) {
            l.normals[n] = fbxSettings->m_Flip ? Vector3(-(item->mNormals[n].x), item->mNormals[n].z, item->mNormals[n].y) :
                                                 Vector3(  item->mNormals[n].x,  item->mNormals[n].y, item->mNormals[n].z);
        }
    } else {
        Log(Log::WRN) << "No normals exist";
    }

    if(item->HasTangentsAndBitangents()) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_TANGENTS);
        l.tangents.resize(vertexCount);
        for(uint32_t t = 0; t < vertexCount; t++) {
            l.tangents[t] = fbxSettings->m_Flip ? Vector3(-(item->mTangents[t].x), item->mTangents[t].z, item->mTangents[t].y) :
                                                  Vector3(  item->mTangents[t].x,  item->mTangents[t].y, item->mTangents[t].z);
        }
    } else {
        Log(Log::WRN) << "No tangents exist";
    }

    uint32_t indexCount = static_cast<uint32_t>(item->mNumFaces * 3);
    l.indices.resize(indexCount);

    for(uint32_t i = 0; i < item->mNumFaces; i++) {
        aiFace *face = &item->mFaces[i];

        uint32_t index = i * 3;

        l.indices[index+0] = face->mIndices[0];
        l.indices[index+1] = face->mIndices[1];
        l.indices[index+2] = face->mIndices[2];
    }

    if(item->HasBones()) {
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_SKINNED);

        l.weights.resize(vertexCount);
        l.bones.resize(vertexCount);

        memset(&l.weights[0], 0, sizeof(Vector4) * vertexCount);
        memset(&l.bones[0], 0, sizeof(Vector4) * vertexCount);

        for(uint32_t b = 0; b < item->mNumBones; b++) {
            aiBone *bone = item->mBones[b];
            int32_t index = indexOf(bone, fbxSettings->m_Bones);
            for(uint32_t w = 0; w < bone->mNumWeights; w++) {
                aiVertexWeight *weight = &bone->mWeights[w];

                uint8_t a;
                for(a = 0; a < 4; a++) {
                    if(l.weights[weight->mVertexId].v[a] <= 0.0f) {
                        break;
                    }
                }
                if(a < 4) {
                    l.weights[weight->mVertexId].v[a] = weight->mWeight;
                    l.bones[weight->mVertexId].v[a] = index;
                }
            }
        }
    }

    mesh->addLod(l);

    return mesh;
}

static bool compare(const AnimationClip::Track &left, const AnimationClip::Track &right) {
    return left.path > right.path;
}

void optimizeVectorTrack(AnimationClip::Track &track, float threshold) {
    for(uint32_t i = 1; i < track.curves[0].m_Keys.size() - 1; i++) {
        Vector3 k0( track.curves[0].m_Keys[i - 1].m_Value,
                    track.curves[1].m_Keys[i - 1].m_Value,
                    track.curves[2].m_Keys[i - 1].m_Value);

        Vector3 k1( track.curves[0].m_Keys[i].m_Value,
                    track.curves[1].m_Keys[i].m_Value,
                    track.curves[2].m_Keys[i].m_Value);

        Vector3 k2( track.curves[0].m_Keys[i + 1].m_Value,
                    track.curves[1].m_Keys[i + 1].m_Value,
                    track.curves[2].m_Keys[i + 1].m_Value);

        Vector3 pd = (k2 - k0);
        float d0 = pd.dot(k0);
        float d1 = pd.dot(k1);
        float d2 = pd.dot(k2);
        if (d1 < d0 || d1 > d2) {
            continue;
        }

        float d = distanceToSegment<Vector3>(k0, k2, k1);
        if(d > pd.length() * threshold) {
            continue;
        }

        AnimationCurve::Keys &curve0 = track.curves[0].m_Keys;
        curve0.erase(curve0.begin() + i);
        AnimationCurve::Keys &curve1 = track.curves[1].m_Keys;
        curve1.erase(curve1.begin() + i);
        AnimationCurve::Keys &curve2 = track.curves[2].m_Keys;
        curve2.erase(curve2.begin() + i);

        i--;
    }
}

void optimizeQuaternionTrack(AnimationClip::Track &track, float threshold) {
    for(uint32_t i = 1; i < track.curves[0].m_Keys.size() - 1; i++) {
        Quaternion k0( track.curves[0].m_Keys[i - 1].m_Value,
                       track.curves[1].m_Keys[i - 1].m_Value,
                       track.curves[2].m_Keys[i - 1].m_Value,
                       track.curves[3].m_Keys[i - 1].m_Value);

        Quaternion k1( track.curves[0].m_Keys[i].m_Value,
                       track.curves[1].m_Keys[i].m_Value,
                       track.curves[2].m_Keys[i].m_Value,
                       track.curves[3].m_Keys[i].m_Value);

        Quaternion k2( track.curves[0].m_Keys[i + 1].m_Value,
                       track.curves[1].m_Keys[i + 1].m_Value,
                       track.curves[2].m_Keys[i + 1].m_Value,
                       track.curves[3].m_Keys[i + 1].m_Value);

        if(k0.equal(k2) && !k0.equal(k1)) {
            continue;
        }

        Quaternion r1 = (k0.inverse() * k1);
        Quaternion r0 = (k0.inverse() * k2);

        r1.normalize();
        r0.normalize();

        Vector3 v0, v1;
        float a0, a1;

        r0.axisAngle(v0, a0);
        r1.axisAngle(v1, a1);

        if(abs(a0) > threshold) {
            continue;
        }

        if(v1.dot(v0) < 0) {
            v0 = -v0;
            a0 = -a0;
        }

        v0.normalize();
        v1.normalize();

        float err01 = acos(v1.dot(v0)) / PI;
        if (err01 > threshold) { // Not the same axis
            continue;
        }

        if(a1 * a0 < 0) { // Not the same direction
            continue;
        }

        float tr = a1 / a0;
        if (tr < 0 || tr > 1) { // Rotating too much or too less
            continue;
        }

        AnimationCurve::Keys &curve0 = track.curves[0].m_Keys;
        curve0.erase(curve0.begin() + i);
        AnimationCurve::Keys &curve1 = track.curves[1].m_Keys;
        curve1.erase(curve1.begin() + i);
        AnimationCurve::Keys &curve2 = track.curves[2].m_Keys;
        curve2.erase(curve2.begin() + i);
        AnimationCurve::Keys &curve3 = track.curves[3].m_Keys;
        curve3.erase(curve3.begin() + i);
        i--;
    }
}

void AssimpConverter::importAnimation(const aiScene *scene, AssimpImportSettings *fbxSettings) {
    AnimationClipSerial clip;
    clip.setName("AnimationClip");

    for(uint32_t a = 0; a < scene->mNumAnimations; a++) {
        aiAnimation *animation = scene->mAnimations[a];

        double animRate = (animation->mTicksPerSecond > 0) ? animation->mTicksPerSecond : 1;

        for(uint32_t c = 0; c < animation->mNumChannels; c++) {
            aiNodeAnim *channel = animation->mChannels[c];
            auto it = fbxSettings->m_Actors.find(channel->mNodeName.C_Str());

            if(it != fbxSettings->m_Actors.end()) {
                Actor *actor = it->second;
                string path = pathTo(fbxSettings->m_pRootActor, actor->transform());

                if(channel->mNumPositionKeys > 1) {
                    AnimationClip::Track track;

                    track.path = path;
                    track.property = "Position";

                    AnimationCurve x, y, z;
                    for(uint32_t k = 0; k < channel->mNumPositionKeys; k++) {
                        aiVectorKey *key = &channel->mPositionKeys[k];

                        AnimationCurve::KeyFrame frameX, frameY, frameZ;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frameX.m_Position = frameY.m_Position = frameZ.m_Position = time;
                        frameX.m_Type = frameY.m_Type = frameZ.m_Type = AnimationCurve::KeyFrame::Linear;

                        frameX.m_Value = key->mValue.x;
                        frameY.m_Value = key->mValue.y;
                        frameZ.m_Value = key->mValue.z;

                        x.m_Keys.push_back(frameX);
                        y.m_Keys.push_back(frameY);
                        z.m_Keys.push_back(frameZ);
                    }
                    track.curves[0] = x;
                    track.curves[1] = y;
                    track.curves[2] = z;

                    if(fbxSettings->filter()) {
                        optimizeVectorTrack(track, fbxSettings->positionError());
                    }

                    clip.m_Tracks.push_back(track);
                }

                if(channel->mNumRotationKeys > 1) {
                    AnimationClip::Track track;

                    track.path = path;
                    track.property = "_Rotation";

                    AnimationCurve x, y, z, w;
                    for(uint32_t k = 0; k < channel->mNumRotationKeys; k++) {
                        aiQuatKey *key = &channel->mRotationKeys[k];

                        AnimationCurve::KeyFrame frameX, frameY, frameZ, frameW;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frameX.m_Position = frameY.m_Position = frameZ.m_Position = frameW.m_Position = time;
                        frameX.m_Type = frameY.m_Type = frameZ.m_Type = AnimationCurve::KeyFrame::Linear;

                        frameX.m_Value = key->mValue.x;
                        frameY.m_Value = key->mValue.y;
                        frameZ.m_Value = key->mValue.z;
                        frameW.m_Value = key->mValue.w;

                        x.m_Keys.push_back(frameX);
                        y.m_Keys.push_back(frameY);
                        z.m_Keys.push_back(frameZ);
                        w.m_Keys.push_back(frameW);
                    }
                    track.curves[0] = x;
                    track.curves[1] = y;
                    track.curves[2] = z;
                    track.curves[3] = w;

                    if(fbxSettings->filter()) {
                        optimizeQuaternionTrack(track, fbxSettings->rotationError());
                    }

                    clip.m_Tracks.push_back(track);
                }

                if(channel->mNumScalingKeys > 1) {
                    AnimationClip::Track track;

                    track.path = path;
                    track.property = "Scale";

                    AnimationCurve x, y, z;
                    for(uint32_t k = 0; k < channel->mNumScalingKeys; k++) {
                        aiVectorKey *key = &channel->mScalingKeys[k];

                        AnimationCurve::KeyFrame frameX, frameY, frameZ;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frameX.m_Position = frameY.m_Position = frameZ.m_Position = time;
                        frameX.m_Type = frameY.m_Type = frameZ.m_Type = AnimationCurve::KeyFrame::Linear;

                        frameX.m_Value = key->mValue.x;
                        frameY.m_Value = key->mValue.y;
                        frameZ.m_Value = key->mValue.z;

                        x.m_Keys.push_back(frameX);
                        y.m_Keys.push_back(frameY);
                        z.m_Keys.push_back(frameZ);
                    }
                    track.curves[0] = x;
                    track.curves[1] = y;
                    track.curves[2] = z;

                    if(fbxSettings->filter()) {
                        optimizeVectorTrack(track, fbxSettings->scaleError());
                    }

                    clip.m_Tracks.push_back(track);
                }
            }
        }
    }

    clip.m_Tracks.sort(compare);

    saveData(Bson::save(Engine::toVariant(&clip)), clip.name().c_str(), IConverter::ContentAnimation, fbxSettings);
}

void AssimpConverter::importPose(AssimpImportSettings *fbxSettings) {
    PoseSerial *pose = new PoseSerial;
    pose->setName("Pose");

    for(auto it : fbxSettings->m_Bones) {
        aiVector3D scl, rot, pos;
        it->mOffsetMatrix.Decompose(scl, rot, pos);

        Pose::Bone b;
        b.position = Vector3(pos.x, pos.y, pos.z) * fbxSettings->customScale();
        b.rotation = Vector3(rot.x, rot.y, rot.z) * RAD2DEG;
        b.scale = Vector3(scl.x, scl.y, scl.z);
        b.index = 0;

        auto result = fbxSettings->m_Actors.find(it->mName.C_Str());
        if(result != fbxSettings->m_Actors.end()) {
            b.index = result->second->transform()->uuid();
        }

        pose->addBone(b);
    }

    QString uuid = saveData(Bson::save(Engine::toVariant(pose)), pose->name().c_str(), IConverter::ContentPose, fbxSettings);

    Engine::setResource(pose, uuid.toStdString());
    fbxSettings->m_Resources.push_back(uuid);

    if(fbxSettings->m_pRootBone) {
        Armature *armature = dynamic_cast<Armature *>(fbxSettings->m_pRootBone->addComponent("Armature"));
        armature->setBindPose(pose);

        for(auto r : fbxSettings->m_Renders) {
            SkinnedMeshRender *render = static_cast<SkinnedMeshRender *>(r);
            render->setArmature(armature);
        }
    }
}

QString AssimpConverter::saveData(const ByteArray &data, const QString &path, int32_t type, AssimpImportSettings *settings) {
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
