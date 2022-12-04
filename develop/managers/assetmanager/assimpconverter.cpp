#include "assimpconverter.h"

#include <QFileInfo>
#include <QTime>
#include <QUuid>
#include <QDebug>

#include <float.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <bson.h>
#include <log.h>

#include "components/actor.h"
#include "components/armature.h"
#include "components/transform.h"
#include "components/skinnedmeshrender.h"
#include "components/meshrender.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/prefab.h"
#include "resources/pose.h"

#include "systems/resourcesystem.h"

#include "animconverter.h"

#define HEADER  "Header"
#define DATA    "Data"

#define FORMAT_VERSION 6

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
        m_Animation(true),
        m_Filter(Keyframe_Reduction),
        m_PositionError(0.5f),
        m_RotationError(0.5f),
        m_ScaleError(0.5f) {

    setType(MetaType::type<Prefab *>());
    setVersion(FORMAT_VERSION);
}

QStringList AssimpImportSettings::typeNames() const {
    return { "Prefab", "Mesh", "Pose", "AnimationClip" };
}

QString AssimpImportSettings::defaultIcon(QString type) const {
    if(type == "Mesh") {
        return ":/Style/styles/dark/images/mesh.svg";
    } else if(type == "Pose") {
        return ":/Style/styles/dark/images/pose.svg";
    } else if(type == "AnimationClip") {
        return ":/Style/styles/dark/images/anim.svg";
    }
    return ":/Style/styles/dark/images/prefab.svg";
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

AssimpConverter::AssimpConverter() {

}

AssetConverterSettings *AssimpConverter::createSettings() const {
    return new AssimpImportSettings();
}

Actor *AssimpConverter::createActor(const QString &guid) const {
    Prefab *prefab = Engine::loadResource<Prefab>(guid.toStdString());
    if(prefab) {
        return static_cast<Actor *>(prefab->actor()->clone());
    }
    return AssetConverter::createActor(guid);
}

AssetConverter::ReturnCode AssimpConverter::convertFile(AssetConverterSettings *settings) {
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

    const aiScene *scene = aiImportFile(qPrintable(fbxSettings->source()), aiProcessPreset_TargetRealtime_MaxQuality);
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
                root->transform()->setRotation(Vector3());
                root->transform()->setScale(Vector3(1.0f));
            }
        }

        if(scene->HasAnimations() && fbxSettings->animation()) {
            importAnimation(scene, fbxSettings);
        }

        aiReleaseImport(scene);

        Prefab *prefab = Engine::objectCreate<Prefab>("");
        prefab->setActor(root);

        QFile file(settings->absoluteDestination());
        if(file.open(QIODevice::WriteOnly)) {
            ByteArray data = Bson::save(Engine::toVariant(prefab));
            file.write(reinterpret_cast<const char *>(&data[0]), data.size());
            file.close();
        }

        for(auto &it : fbxSettings->m_Resources) {
            Engine::unloadResource(it.toStdString());
        }
        Log(Log::INF) << "Mesh imported in:" << time.elapsed() << "msec";

        settings->setCurrentVersion(settings->version());

        return Success;
    }
    return InternalError;
}

Actor *importObjectHelper(const aiScene *scene, const aiNode *element, const aiMatrix4x4 &p, Actor *parent, AssimpImportSettings *fbxSettings) {
    string name = element->mName.C_Str();

    if(name.find("$") != string::npos) {
        for(uint32_t c = 0; c < element->mNumChildren; c++) {
            importObjectHelper(scene, element->mChildren[c], p * element->mTransformation, parent, fbxSettings);
        }
    } else {
        Actor *actor = Engine::objectCreate<Actor>(name, parent);
        actor->addComponent("Transform");

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
        actor->transform()->setRotation(Vector3(euler.x, euler.y, euler.z) * RAD2DEG);
        actor->transform()->setScale(Vector3(scale.x, scale.y, scale.z));

        QString uuid = actor->name().c_str();

        Mesh *result = AssimpConverter::importMesh(scene, element, actor, fbxSettings);
        if(result) {
            uuid = AssimpConverter::saveData(Bson::save(Engine::toVariant(result)), actor->name().c_str(), MetaType::type<Mesh *>(), fbxSettings);

            Mesh *resource = Engine::loadResource<Mesh>(qPrintable(uuid));
            if(resource == nullptr) {
                Engine::setResource(result, uuid.toStdString());
                fbxSettings->m_Resources.push_back(uuid);
                resource = result;
            }

            if(!result->weights().empty()) {
                SkinnedMeshRender *render = static_cast<SkinnedMeshRender *>(actor->addComponent("SkinnedMeshRender"));
                Engine::replaceUUID(render, qHash(uuid + ".SkinnedMeshRender"));

                render->setMesh(resource);
                fbxSettings->m_Renders.push_back(render);
            } else {
                MeshRender *render = static_cast<MeshRender *>(actor->addComponent("MeshRender"));
                Engine::replaceUUID(render, qHash(uuid + ".MeshRender"));

                render->setMesh(resource);
                fbxSettings->m_Renders.push_back(render);
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

Mesh *AssimpConverter::importMesh(const aiScene *scene, const aiNode *element, Actor *parent, AssimpImportSettings *fbxSettings) {
    if(element->mNumMeshes) {
        Mesh *mesh = new Mesh;

        mesh->setTopology(Mesh::Triangles);
        mesh->setMaterial(Engine::loadResource<Material>(".embedded/DefaultMesh.mtl"));

        size_t total_v = 0;
        size_t total_i = 0;

        size_t count_v = 0;
        size_t count_i = 0;

        for(uint32_t index = 0; index < element->mNumMeshes; index++) {
            const aiMesh *item = scene->mMeshes[element->mMeshes[index]];
            count_v += item->mNumVertices;
            count_i += static_cast<uint32_t>(item->mNumFaces * 3);
        }

        IndexVector &indices = mesh->indices();
        indices.resize(count_i);

        Vector3Vector &vertices = mesh->vertices();
        vertices.resize(count_v);

        Vector4Vector &colors = mesh->colors();
        colors.resize(count_v);

        Vector2Vector &uv0 = mesh->uv0();
        uv0.resize(count_v);

        Vector3Vector &normals = mesh->normals();
        normals.resize(count_v);

        Vector3Vector &tangents = mesh->tangents();
        tangents.resize(count_v);

        for(uint32_t index = 0; index < element->mNumMeshes; index++) {
            uint32_t it = element->mMeshes[index];
            const aiMesh *item = scene->mMeshes[it];

            // Export
            Vector3 delta;
            if(item->HasBones()) {
                Transform *t = parent->transform();
                Matrix4 rot;
                rot.rotate(t->worldRotation());

                Vector3 s = t->worldScale();
                Vector3 p = t->worldPosition();

                delta = rot.inverse() * Vector3(p.x / s.x, p.y / s.y, p.z / s.z);
            }

            uint32_t vertexCount = item->mNumVertices;

            float scl = fbxSettings->customScale();
            for(uint32_t v = 0; v < vertexCount; v++) {
                Vector3 pos(item->mVertices[v].x * scl, item->mVertices[v].y * scl, item->mVertices[v].z * scl);
                if(fbxSettings->m_Flip) {
                    pos = Vector3(-pos.x, pos.z, pos.y);
                }
                vertices[total_v + v] = delta + pos;
            }

            if(item->HasVertexColors(0)) {
                memcpy(&colors[total_v], item->mColors[0], sizeof(Vector4) * vertexCount);
            }

            if(item->HasTextureCoords(0)) {
                aiVector3D *uv = item->mTextureCoords[0];
                for(uint32_t u = 0; u < vertexCount; u++) {
                    uv0[total_v + u] = Vector2(uv[u].x, uv[u].y);
                }
            } else {
                Log(Log::WRN) << "No uv exist";
            }

            if(item->HasNormals()) {
                for(uint32_t n = 0; n < vertexCount; n++) {
                    normals[total_v + n] = fbxSettings->m_Flip ? Vector3(-(item->mNormals[n].x), item->mNormals[n].z, item->mNormals[n].y) :
                                                                 Vector3(  item->mNormals[n].x,  item->mNormals[n].y, item->mNormals[n].z);
                }
            } else {
                Log(Log::WRN) << "No normals exist";
            }

            if(item->HasTangentsAndBitangents()) {
                for(uint32_t t = 0; t < vertexCount; t++) {
                    tangents[total_v + t] = fbxSettings->m_Flip ? Vector3(-(item->mTangents[t].x), item->mTangents[t].z, item->mTangents[t].y) :
                                                                  Vector3(  item->mTangents[t].x,  item->mTangents[t].y, item->mTangents[t].z);
                }
            } else {
                Log(Log::WRN) << "No tangents exist";
            }

            uint32_t indexCount = static_cast<uint32_t>(item->mNumFaces * 3);
            for(uint32_t i = 0; i < item->mNumFaces; i++) {
                aiFace *face = &item->mFaces[i];

                uint32_t index = total_i + i * 3;

                indices[index+0] = total_v + face->mIndices[0];
                indices[index+1] = total_v + face->mIndices[1];
                indices[index+2] = total_v + face->mIndices[2];
            }

            if(item->HasBones()) {
                Vector4Vector &weights = mesh->weights();
                Vector4Vector &bones = mesh->bones();

                weights.resize(total_v + vertexCount);
                bones.resize(total_v + vertexCount);

                memset(&weights[total_v], 0, sizeof(Vector4) * vertexCount);
                memset(&bones[total_v], 0, sizeof(Vector4) * vertexCount);

                for(uint32_t b = 0; b < item->mNumBones; b++) {
                    aiBone *bone = item->mBones[b];
                    int32_t index = indexOf(bone, fbxSettings->m_Bones);
                    for(uint32_t w = 0; w < bone->mNumWeights; w++) {
                        aiVertexWeight *weight = &bone->mWeights[w];

                        uint8_t a;
                        for(a = 0; a < 4; a++) {
                            if(weights[total_v + weight->mVertexId].v[a] <= 0.0f) {
                                break;
                            }
                        }
                        if(a < 4) {
                            weights[total_v + weight->mVertexId].v[a] = weight->mWeight;
                            bones[total_v + weight->mVertexId].v[a] = index;
                        }
                    }
                }
            }

            total_v += vertexCount;
            total_i += indexCount;
        }

        return mesh;
    }
    return nullptr;
}

static bool compare(const AnimationTrack &left, const AnimationTrack &right) {
    return left.path() > right.path();
}

void optimizeVectorTrack(AnimationTrack &track, float threshold) {
    auto &curves = track.curves();
    for(uint32_t i = 1; i < curves[0].m_Keys.size() - 1; i++) {
        Vector3 k0( curves[0].m_Keys[i - 1].m_Value,
                    curves[1].m_Keys[i - 1].m_Value,
                    curves[2].m_Keys[i - 1].m_Value);

        Vector3 k1( curves[0].m_Keys[i].m_Value,
                    curves[1].m_Keys[i].m_Value,
                    curves[2].m_Keys[i].m_Value);

        Vector3 k2( curves[0].m_Keys[i + 1].m_Value,
                    curves[1].m_Keys[i + 1].m_Value,
                    curves[2].m_Keys[i + 1].m_Value);

        Vector3 pd = (k2 - k0);
        float d0 = pd.dot(k0);
        float d1 = pd.dot(k1);
        float d2 = pd.dot(k2);
        if (d1 < d0 || d1 > d2) {
            continue;
        }

        float d = Mathf::distanceToSegment<Vector3>(k0, k2, k1);
        if(d > pd.length() * threshold) {
            continue;
        }

        AnimationCurve::Keys &curve0 = curves[0].m_Keys;
        curve0.erase(curve0.begin() + i);
        AnimationCurve::Keys &curve1 = curves[1].m_Keys;
        curve1.erase(curve1.begin() + i);
        AnimationCurve::Keys &curve2 = curves[2].m_Keys;
        curve2.erase(curve2.begin() + i);

        i--;
    }
}

void optimizeQuaternionTrack(AnimationTrack &track, float threshold) {
    auto &curves = track.curves();
    for(uint32_t i = 1; i < curves[0].m_Keys.size() - 1; i++) {
        Quaternion k0( curves[0].m_Keys[i - 1].m_Value,
                       curves[1].m_Keys[i - 1].m_Value,
                       curves[2].m_Keys[i - 1].m_Value,
                       curves[3].m_Keys[i - 1].m_Value);

        Quaternion k1( curves[0].m_Keys[i].m_Value,
                       curves[1].m_Keys[i].m_Value,
                       curves[2].m_Keys[i].m_Value,
                       curves[3].m_Keys[i].m_Value);

        Quaternion k2( curves[0].m_Keys[i + 1].m_Value,
                       curves[1].m_Keys[i + 1].m_Value,
                       curves[2].m_Keys[i + 1].m_Value,
                       curves[3].m_Keys[i + 1].m_Value);

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

        AnimationCurve::Keys &curve0 = curves[0].m_Keys;
        curve0.erase(curve0.begin() + i);
        AnimationCurve::Keys &curve1 = curves[1].m_Keys;
        curve1.erase(curve1.begin() + i);
        AnimationCurve::Keys &curve2 = curves[2].m_Keys;
        curve2.erase(curve2.begin() + i);
        AnimationCurve::Keys &curve3 = curves[3].m_Keys;
        curve3.erase(curve3.begin() + i);
        i--;
    }
}

void AssimpConverter::importAnimation(const aiScene *scene, AssimpImportSettings *fbxSettings) {
    for(uint32_t a = 0; a < scene->mNumAnimations; a++) {
        aiAnimation *animation = scene->mAnimations[a];

        AnimationClip clip;
        clip.setName(animation->mName.C_Str());

        double animRate = (animation->mTicksPerSecond > 0) ? animation->mTicksPerSecond : 1;

        for(uint32_t c = 0; c < animation->mNumChannels; c++) {
            aiNodeAnim *channel = animation->mChannels[c];
            auto it = fbxSettings->m_Actors.find(channel->mNodeName.C_Str());

            if(it != fbxSettings->m_Actors.end()) {
                Actor *actor = it->second;
                string path = pathTo(fbxSettings->m_pRootActor, actor->transform());

                if(channel->mNumPositionKeys > 1) {
                    AnimationTrack track;

                    track.setPath(path);
                    track.setProperty("position");

                    uint32_t duration = uint32_t((channel->mPositionKeys[channel->mNumPositionKeys - 1].mTime / animRate) * 1000);
                    track.setDuration(duration);

                    AnimationCurve x, y, z;
                    for(uint32_t k = 0; k < channel->mNumPositionKeys; k++) {
                        aiVectorKey *key = &channel->mPositionKeys[k];

                        AnimationCurve::KeyFrame frameX, frameY, frameZ;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frameX.m_Position = frameY.m_Position = frameZ.m_Position = (float)time / (float)duration; // Normalized time
                        frameX.m_Type = frameY.m_Type = frameZ.m_Type = AnimationCurve::KeyFrame::Linear;

                        Vector3 pos = Vector3(key->mValue.x, key->mValue.y, key->mValue.z) * fbxSettings->customScale();
                        if(fbxSettings->m_Flip) {
                            pos = Vector3(-pos.x, pos.z, pos.y);
                        }

                        frameX.m_Value = pos.x;
                        frameY.m_Value = pos.y;
                        frameZ.m_Value = pos.z;

                        x.m_Keys.push_back(frameX);
                        y.m_Keys.push_back(frameY);
                        z.m_Keys.push_back(frameZ);
                    }
                    auto &curves = track.curves();

                    curves[0] = x;
                    curves[1] = y;
                    curves[2] = z;

                    if(fbxSettings->filter()) {
                        optimizeVectorTrack(track, fbxSettings->positionError());
                    }

                    clip.m_Tracks.push_back(track);
                }

                if(channel->mNumRotationKeys > 1) {
                    AnimationTrack track;

                    track.setPath(path);
                    track.setProperty("quaternion");

                    uint32_t duration = uint32_t((channel->mRotationKeys[channel->mNumRotationKeys - 1].mTime / animRate) * 1000);
                    track.setDuration(duration);

                    AnimationCurve x, y, z, w;
                    for(uint32_t k = 0; k < channel->mNumRotationKeys; k++) {
                        aiQuatKey *key = &channel->mRotationKeys[k];

                        AnimationCurve::KeyFrame frameX, frameY, frameZ, frameW;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frameX.m_Position = frameY.m_Position = frameZ.m_Position = frameW.m_Position = (float)time / (float)duration; // Normalized time
                        duration = MAX(duration, frameX.m_Position);
                        frameX.m_Type = frameY.m_Type = frameZ.m_Type = frameW.m_Type = AnimationCurve::KeyFrame::Linear;

                        frameX.m_Value = key->mValue.x;
                        frameY.m_Value = key->mValue.y;
                        frameZ.m_Value = key->mValue.z;
                        frameW.m_Value = key->mValue.w;

                        x.m_Keys.push_back(frameX);
                        y.m_Keys.push_back(frameY);
                        z.m_Keys.push_back(frameZ);
                        w.m_Keys.push_back(frameW);
                    }
                    auto &curves = track.curves();

                    curves[0] = x;
                    curves[1] = y;
                    curves[2] = z;
                    curves[3] = w;

                    if(fbxSettings->filter()) {
                        optimizeQuaternionTrack(track, fbxSettings->rotationError());
                    }

                    clip.m_Tracks.push_back(track);
                }

                if(channel->mNumScalingKeys > 1) {
                    AnimationTrack track;

                    track.setPath(path);
                    track.setProperty("scale");

                    uint32_t duration = uint32_t((channel->mScalingKeys[channel->mNumScalingKeys - 1].mTime / animRate) * 1000);
                    track.setDuration(duration);

                    AnimationCurve x, y, z;
                    for(uint32_t k = 0; k < channel->mNumScalingKeys; k++) {
                        aiVectorKey *key = &channel->mScalingKeys[k];

                        AnimationCurve::KeyFrame frameX, frameY, frameZ;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frameX.m_Position = frameY.m_Position = frameZ.m_Position = (float)time / (float)duration; // Normalized time
                        duration = MAX(duration, frameX.m_Position);
                        frameX.m_Type = frameY.m_Type = frameZ.m_Type = AnimationCurve::KeyFrame::Linear;

                        frameX.m_Value = key->mValue.x;
                        frameY.m_Value = key->mValue.y;
                        frameZ.m_Value = key->mValue.z;

                        x.m_Keys.push_back(frameX);
                        y.m_Keys.push_back(frameY);
                        z.m_Keys.push_back(frameZ);
                    }
                    auto &curves = track.curves();

                    curves[0] = x;
                    curves[1] = y;
                    curves[2] = z;

                    if(fbxSettings->filter()) {
                        optimizeVectorTrack(track, fbxSettings->scaleError());
                    }

                    clip.m_Tracks.push_back(track);
                }
            }
        }

        clip.m_Tracks.sort(compare);

        int32_t type = MetaType::type<AnimationClip *>();
        saveData(Bson::save(Engine::toVariant(&clip)), clip.name().c_str(), type, fbxSettings);
    }
}

void AssimpConverter::importPose(AssimpImportSettings *fbxSettings) {
    Pose *pose = new Pose;
    pose->setName("Pose");

    for(auto it : fbxSettings->m_Bones) {
        aiVector3D scl, rot, pos;
        it->mOffsetMatrix.Decompose(scl, rot, pos);

        Bone b;
        b.setPosition(Vector3(pos.x, pos.y, pos.z) * fbxSettings->customScale());
        b.setRotation(Vector3(rot.x, rot.y, rot.z) * RAD2DEG);
        b.setScale(Vector3(scl.x, scl.y, scl.z));

        auto result = fbxSettings->m_Actors.find(it->mName.C_Str());
        if(result != fbxSettings->m_Actors.end()) {
            b.setIndex(result->second->transform()->uuid());
        }

        pose->addBone(&b);
    }

    QString uuid = saveData(Bson::save(Engine::toVariant(pose)), pose->name().c_str(), MetaType::type<Pose *>(), fbxSettings);

    Pose *resource = Engine::loadResource<Pose>(qPrintable(uuid));
    if(resource == nullptr) {
        Engine::setResource(pose, uuid.toStdString());
        fbxSettings->m_Resources.push_back(uuid);
        resource = pose;
    }

    fbxSettings->m_Resources.push_back(uuid);

    if(fbxSettings->m_pRootBone) {
        Armature *armature = dynamic_cast<Armature *>(fbxSettings->m_pRootBone->addComponent("Armature"));
        armature->setBindPose(resource);
        Engine::replaceUUID(armature, qHash(uuid + ".Armature"));

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
