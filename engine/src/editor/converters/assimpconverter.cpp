#include "converters/assimpconverter.h"

#include <float.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <bson.h>
#include <log.h>
#include <file.h>

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

#include "converters/animconverter.h"

#define HEADER  "Header"
#define DATA    "Data"

#define FORMAT_VERSION 8

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

TString pathTo(Object *root, Object *dst) {
    TString result;
    if(root != dst) {
        TString parent = pathTo(root, dst->parent());
        if(!parent.isEmpty()) {
            result += parent + "/";
        }
        result += dst->name();
    }
    return result;
}

void stabilizeUUID(Object *object) {
    TString path = pathTo(nullptr, object);
    Engine::replaceUUID(object, Mathf::hashString(path));

    for(auto it : object->getChildren()) {
        stabilizeUUID(it);
    }
}

AssimpImportSettings::AssimpImportSettings() :
        m_rootActor(nullptr),
        m_rootBone(nullptr),
        m_useScale(false),
        m_scale(1.0f),
        m_colors(true),
        m_normals(true),
        m_animation(true),
        m_filter(Keyframe_Reduction),
        m_positionError(0.5f),
        m_rotationError(0.5f),
        m_scaleError(0.5f) {

    setVersion(FORMAT_VERSION);
}

StringList AssimpImportSettings::typeNames() const {
    return { "Prefab", "Mesh", "Pose", "AnimationClip" };
}

TString AssimpImportSettings::defaultIconPath(const TString &type) const {
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
    return m_colors;
}
void AssimpImportSettings::setColors(bool value) {
    if(m_colors != value) {
        m_colors = value;
        setModified();
    }
}

bool AssimpImportSettings::normals() const {
    return m_normals;
}
void AssimpImportSettings::setNormals(bool value) {
    if(m_normals != value) {
        m_normals = value;
        setModified();
    }
}

bool AssimpImportSettings::animation() const {
    return m_animation;
}
void AssimpImportSettings::setAnimation(bool value) {
    if(m_animation != value) {
        m_animation = value;
        setModified();
    }
}

int AssimpImportSettings::filter() const {
    return m_filter;
}
void AssimpImportSettings::setFilter(int value) {
    if(m_filter != value) {
        m_filter = value;
        setModified();
    }
}

bool AssimpImportSettings::useScale() const {
    return m_useScale;
}
void AssimpImportSettings::setUseScale(bool value) {
    if(m_useScale != value) {
        m_useScale = value;
        setModified();
    }
}

float AssimpImportSettings::customScale() const {
    return m_scale;
}
void AssimpImportSettings::setCustomScale(float value) {
    if(m_scale != value) {
        m_scale = value;
        setModified();
    }
}

float AssimpImportSettings::positionError() const {
    return m_positionError;
}
void AssimpImportSettings::setPositionError(float value) {
    if(m_positionError != value) {
        m_positionError = value;
        setModified();
    }
}

float AssimpImportSettings::rotationError() const {
    return m_rotationError;
}
void AssimpImportSettings::setRotationError(float value) {
    if(m_rotationError != value) {
        m_rotationError = value;
        setModified();
    }
}

float AssimpImportSettings::scaleError() const {
    return m_scaleError;
}
void AssimpImportSettings::setScaleError(float value) {
    if(m_scaleError != value) {
        m_scaleError = value;
        setModified();
    }
}

AssimpConverter::AssimpConverter() {

}

AssetConverterSettings *AssimpConverter::createSettings() {
    return new AssimpImportSettings();
}

Actor *AssimpConverter::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    Resource *resource = Engine::loadResource<Resource>(guid);
    if(dynamic_cast<Prefab *>(resource) != nullptr) {
        return static_cast<Actor *>(static_cast<Prefab *>(resource)->actor()->clone());
    } else if(dynamic_cast<Mesh *>(resource) != nullptr) {
        Actor *object = Engine::composeActor<MeshRender>("");
        MeshRender *render = object->getComponent<MeshRender>();
        if(render) {
            render->setMesh(static_cast<Mesh *>(resource));
        }
        return object;
    }
    return AssetConverter::createActor(settings, guid);
}

AssetConverter::ReturnCode AssimpConverter::convertFile(AssetConverterSettings *settings) {
    AssimpImportSettings *fbxSettings = static_cast<AssimpImportSettings *>(settings);

    fbxSettings->m_renders.clear();
    fbxSettings->m_bones.clear();
    fbxSettings->m_actors.clear();
    fbxSettings->m_meshes.clear();
    fbxSettings->m_rootActor = nullptr;
    fbxSettings->m_rootBone = nullptr;
    fbxSettings->m_flip = false;

    const aiScene *scene = aiImportFile(fbxSettings->source().data(), aiProcessPreset_TargetRealtime_MaxQuality);
    if(scene) {
        aiMetadata *meta = scene->mMetaData;
        for(uint32_t m = 0; m < meta->mNumProperties; m++) {
            aiMetadataEntry *entry = &meta->mValues[m];
            if(meta->mKeys[m] == aiString("UnitScaleFactor")) {
                if(!fbxSettings->useScale()) {
                    float value = *(reinterpret_cast<float *>(entry->mData)) * 0.01f;
                    fbxSettings->setCustomScale(value);
                }
            } else if(meta->mKeys[m] == aiString("UpAxis")) {
                int32_t value = *(reinterpret_cast<int32_t *>(entry->mData));
                if(value == 2) { // The UpAxis is Z need to switch to Y
                    fbxSettings->m_flip = true;
                }
            }
        }

        // Bone structure
        for(uint32_t m = 0; m < scene->mNumMeshes; m++) {
            aiMesh *mesh = scene->mMeshes[m];
            for(uint32_t b = 0; b < mesh->mNumBones; b++) {
                aiBone *bone = mesh->mBones[b];
                if(indexOf(bone, fbxSettings->m_bones) == -1) {
                    fbxSettings->m_bones.push_back(bone);
                }
            }
        }

        Prefab *prefab = Engine::loadResource<Prefab>(settings->destination());
        if(prefab == nullptr) {
            prefab = Engine::objectCreate<Prefab>(settings->destination());
        }

        /// \todo We need to reuse actors if possible
        Actor *root = importObject(scene, scene->mRootNode, nullptr, fbxSettings);

        prefab->setActor(root);

        if(!fbxSettings->m_bones.empty()) {
            importPose(fbxSettings);
        } else {
            if(fbxSettings->m_renders.size() == 1) {
                root = static_cast<Component *>(fbxSettings->m_renders.front())->actor();
                root->transform()->setPosition(Vector3());
                root->transform()->setRotation(Vector3());
                root->transform()->setScale(Vector3(1.0f));
            }
        }

        if(scene->HasAnimations() && fbxSettings->animation()) {
            importAnimation(scene, fbxSettings);
        }

        aiReleaseImport(scene);

        TString name(root->name());
        root->setName(settings->destination()); // fixes the same name of root in the different files issue
        stabilizeUUID(root);
        root->setName(name);

        return settings->saveBinary(Engine::toVariant(prefab), settings->absoluteDestination());
    }
    return InternalError;
}

Actor *importObjectHelper(const aiScene *scene, const aiNode *element, const aiMatrix4x4 &p, Actor *parent, AssimpImportSettings *fbxSettings) {
    std::string name = element->mName.C_Str();

    if(name.find("$") != std::string::npos) {
        for(uint32_t c = 0; c < element->mNumChildren; c++) {
            importObjectHelper(scene, element->mChildren[c], p * element->mTransformation, parent, fbxSettings);
        }
    } else {
        Actor *actor = Engine::objectCreate<Actor>(name, parent);
        Transform *transform = static_cast<Transform *>(actor->addComponent("Transform"));

        if(fbxSettings->m_rootActor == nullptr) {
            fbxSettings->m_rootActor = actor;
        }

        if(fbxSettings->m_rootBone == nullptr) {
            for(auto it : fbxSettings->m_bones) {
                if(it->mName == element->mName) {
                    fbxSettings->m_rootBone = parent;
                    break;
                }
            }
        }

        fbxSettings->m_actors[actor->name()] = actor;

        aiMatrix4x4 t = p * element->mTransformation;

        aiVector3D scale, euler, position;
        t.Decompose(scale, euler, position);

        Vector3 pos = Vector3(position.x, position.y, position.z) * fbxSettings->customScale();
        if(fbxSettings->m_flip) {
            pos = Vector3(-pos.x, pos.z, pos.y);
        }

        transform->setPosition(pos);
        transform->setRotation(Vector3(euler.x, euler.y, euler.z) * RAD2DEG);
        transform->setScale(Vector3(scale.x, scale.y, scale.z));

        Mesh *result = AssimpConverter::importMesh(scene, element, actor, fbxSettings);
        if(result) {
            if(!result->weights().empty()) {
                SkinnedMeshRender *render = static_cast<SkinnedMeshRender *>(actor->addComponent("SkinnedMeshRender"));

                render->setMesh(result);
                fbxSettings->m_renders.push_back(render);
            } else {
                MeshRender *render = static_cast<MeshRender *>(actor->addComponent("MeshRender"));

                render->setMesh(result);
                fbxSettings->m_renders.push_back(render);
            }
        }

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

Mesh *AssimpConverter::importMesh(const aiScene *scene, const aiNode *element, Actor *actor, AssimpImportSettings *fbxSettings) {
    if(element->mNumMeshes) {
        uint32_t hash = 16;
        for(uint32_t index = 0; index < element->mNumMeshes; index++) {
            Mathf::hashCombine(hash, element->mMeshes[index]);
        }

        Mesh *mesh = nullptr;

        auto it = fbxSettings->m_meshes.find(hash);
        if(it != fbxSettings->m_meshes.end()) {
            mesh = it->second;
        }

        if(mesh) {
            return mesh;
        }

        size_t total_v = 0;
        size_t total_i = 0;

        size_t count_v = 0;
        size_t count_i = 0;

        for(uint32_t index = 0; index < element->mNumMeshes; index++) {
            const aiMesh *item = scene->mMeshes[element->mMeshes[index]];

            if(mesh == nullptr) {
                TString uuid = fbxSettings->subItem(actor->name(), true);
                mesh = Engine::loadResource<Mesh>(uuid);
                if(mesh == nullptr) {
                    mesh = Engine::objectCreate<Mesh>(uuid);
                }
            }

            count_v += item->mNumVertices;
            count_i += static_cast<uint32_t>(item->mNumFaces * 3);
        }

        IndexVector &indices = mesh->indices();
        indices.resize(count_i);

        Vector3Vector &vertices = mesh->vertices();
        vertices.resize(count_v);

        Vector4Vector &colors = mesh->colors();
        colors.resize(count_v, Vector4(1.0f));

        Vector2Vector &uv0 = mesh->uv0();
        uv0.resize(count_v);

        Vector3Vector &normals = mesh->normals();
        normals.resize(count_v);

        Vector3Vector &tangents = mesh->tangents();
        tangents.resize(count_v);

        for(uint32_t index = 0; index < element->mNumMeshes; index++) {
            uint32_t it = element->mMeshes[index];
            const aiMesh *item = scene->mMeshes[it];
            if(item->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
                continue;
            }

            // Export
            Matrix4 mov;
            Matrix3 rot;
            if(item->HasBones()) {
                Transform *t = actor->transform();
                rot = t->worldQuaternion().toMatrix();
                mov = t->worldTransform();
            }

            uint32_t vertexCount = item->mNumVertices;

            float scl = fbxSettings->customScale();
            for(uint32_t v = 0; v < vertexCount; v++) {
                Vector3 pos(item->mVertices[v].x * scl, item->mVertices[v].y * scl, item->mVertices[v].z * scl);
                if(fbxSettings->m_flip) {
                    pos = Vector3(-pos.x, pos.z, pos.y);
                }
                vertices[total_v + v] = mov * pos;
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
                aWarning() << "No uv exist";
            }

            if(item->HasNormals()) {
                for(uint32_t n = 0; n < vertexCount; n++) {
                    Vector3 norm(fbxSettings->m_flip ? Vector3(-(item->mNormals[n].x), item->mNormals[n].z, item->mNormals[n].y) :
                                                       Vector3(  item->mNormals[n].x,  item->mNormals[n].y, item->mNormals[n].z));

                    normals[total_v + n] = rot * norm;
                }
            } else {
                aWarning() << "No normals exist";
            }

            if(item->HasTangentsAndBitangents()) {
                for(uint32_t t = 0; t < vertexCount; t++) {
                    Vector3 tan(fbxSettings->m_flip ? Vector3(-(item->mTangents[t].x), item->mTangents[t].z, item->mTangents[t].y) :
                                                      Vector3(  item->mTangents[t].x,  item->mTangents[t].y, item->mTangents[t].z));

                    tangents[total_v + t] = rot * tan;
                }
            } else {
                aWarning() << "No tangents exist";
            }

            uint32_t indexCount = static_cast<uint32_t>(item->mNumFaces * 3);
            for(uint32_t i = 0; i < item->mNumFaces; i++) {
                aiFace *face = &item->mFaces[i];

                size_t v_index = total_i + i * 3;

                indices[v_index+0] = total_v + face->mIndices[0];
                indices[v_index+1] = total_v + face->mIndices[1];
                indices[v_index+2] = total_v + face->mIndices[2];
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
                    int32_t index = indexOf(bone, fbxSettings->m_bones);
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

            mesh->setSubMesh(total_i, index);
            mesh->setDefaultMaterial(Engine::loadResource<Material>(".embedded/DefaultMesh.shader"), index);

            total_v += vertexCount;
            total_i += indexCount;
        }

        fbxSettings->saveSubData(mesh, actor->name(), MetaType::name<Mesh>());
        fbxSettings->m_meshes[hash] = mesh;

        return mesh;
    }
    return nullptr;
}

static bool compare(const AnimationTrack &left, const AnimationTrack &right) {
    return right.path() < left.path();
}

void optimizeVectorTrack(AnimationTrack &track, float threshold) {
    AnimationCurve &curve = track.curve();
    for(uint32_t i = 1; i < curve.m_keys.size() - 1; i++) {
        Vector3 k0(curve.m_keys[i - 1].m_value.toVector3());
        Vector3 k1(curve.m_keys[i].m_value.toVector3());
        Vector3 k2(curve.m_keys[i + 1].m_value.toVector3());

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

        curve.m_keys.erase(curve.m_keys.begin() + i);

        i--;
    }
}

void optimizeQuaternionTrack(AnimationTrack &track, float threshold) {
    AnimationCurve &curve = track.curve();
    for(uint32_t i = 1; i < curve.m_keys.size() - 1; i++) {
        Quaternion k0(curve.m_keys[i - 1].m_value.toQuaternion());
        Quaternion k1(curve.m_keys[i].m_value.toQuaternion());
        Quaternion k2(curve.m_keys[i + 1].m_value.toQuaternion());

        if(k0.equal(k2) && !k0.equal(k1)) {
            continue;
        }

        Quaternion r1(k0.inverse() * k1);
        Quaternion r0(k0.inverse() * k2);

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

        curve.m_keys.erase(curve.m_keys.begin() + i);

        i--;
    }
}

void AssimpConverter::importAnimation(const aiScene *scene, AssimpImportSettings *fbxSettings) {
    for(uint32_t a = 0; a < scene->mNumAnimations; a++) {
        aiAnimation *animation = scene->mAnimations[a];

        TString uuid = fbxSettings->subItem(animation->mName.C_Str(), true);
        AnimationClip *clip = Engine::loadResource<AnimationClip>(uuid);
        if(clip == nullptr) {
            clip = Engine::objectCreate<AnimationClip>(uuid);
        }

        double animRate = (animation->mTicksPerSecond > 0) ? animation->mTicksPerSecond : 1;

        for(uint32_t c = 0; c < animation->mNumChannels; c++) {
            aiNodeAnim *channel = animation->mChannels[c];
            auto it = fbxSettings->m_actors.find(channel->mNodeName.C_Str());

            if(it != fbxSettings->m_actors.end()) {
                Actor *actor = it->second;
                TString path = pathTo(fbxSettings->m_rootActor, actor->transform());

                if(channel->mNumPositionKeys > 1) {
                    AnimationTrack track;

                    track.setPath(path);
                    track.setProperty("position");

                    uint32_t duration = uint32_t((channel->mPositionKeys[channel->mNumPositionKeys - 1].mTime / animRate) * 1000);
                    track.setDuration(duration);

                    AnimationCurve &curve = track.curve();

                    for(uint32_t k = 0; k < channel->mNumPositionKeys; k++) {
                        aiVectorKey *key = &channel->mPositionKeys[k];

                        AnimationCurve::KeyFrame frame;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frame.m_position = (float)time / (float)duration; // Normalized time
                        frame.m_type = AnimationCurve::KeyFrame::Linear;

                        Vector3 pos = Vector3(key->mValue.x, key->mValue.y, key->mValue.z) * fbxSettings->customScale();
                        if(fbxSettings->m_flip) {
                            pos = Vector3(-pos.x, pos.z, pos.y);
                        }

                        frame.m_value = Vector3(pos.x, pos.y, pos.z);

                        curve.m_keys.push_back(frame);
                    }

                    if(fbxSettings->filter()) {
                        optimizeVectorTrack(track, fbxSettings->positionError());
                    }

                    clip->m_tracks.push_back(track);
                }

                if(channel->mNumRotationKeys > 1) {
                    AnimationTrack track;

                    track.setPath(path);
                    track.setProperty("quaternion");

                    uint32_t duration = uint32_t((channel->mRotationKeys[channel->mNumRotationKeys - 1].mTime / animRate) * 1000);
                    track.setDuration(duration);

                    AnimationCurve &curve = track.curve();

                    for(uint32_t k = 0; k < channel->mNumRotationKeys; k++) {
                        aiQuatKey *key = &channel->mRotationKeys[k];

                        AnimationCurve::KeyFrame frame;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frame.m_position = (float)time / (float)duration; // Normalized time
                        duration = MAX(duration, frame.m_position);
                        frame.m_type = AnimationCurve::KeyFrame::Linear;
                        frame.m_value = Quaternion(key->mValue.x, key->mValue.y, key->mValue.z, key->mValue.w);

                        curve.m_keys.push_back(frame);
                    }

                    if(fbxSettings->filter()) {
                        optimizeQuaternionTrack(track, fbxSettings->rotationError());
                    }

                    clip->m_tracks.push_back(track);
                }

                if(channel->mNumScalingKeys > 1) {
                    AnimationTrack track;

                    track.setPath(path);
                    track.setProperty("scale");

                    uint32_t duration = uint32_t((channel->mScalingKeys[channel->mNumScalingKeys - 1].mTime / animRate) * 1000);
                    track.setDuration(duration);

                    AnimationCurve &curve = track.curve();

                    for(uint32_t k = 0; k < channel->mNumScalingKeys; k++) {
                        aiVectorKey *key = &channel->mScalingKeys[k];

                        AnimationCurve::KeyFrame frame;

                        uint32_t time = uint32_t((key->mTime / animRate) * 1000);
                        frame.m_position = (float)time / (float)duration; // Normalized time
                        duration = MAX(duration, frame.m_position);
                        frame.m_type = AnimationCurve::KeyFrame::Linear;
                        frame.m_value = Vector3(key->mValue.x, key->mValue.y, key->mValue.z);

                        curve.m_keys.push_back(frame);
                    }

                    if(fbxSettings->filter()) {
                        optimizeVectorTrack(track, fbxSettings->scaleError());
                    }

                    clip->m_tracks.push_back(track);
                }
            }
        }

        clip->m_tracks.sort(compare);

        fbxSettings->saveSubData(clip, animation->mName.C_Str(), MetaType::name<AnimationClip>());
    }
}

void AssimpConverter::importPose(AssimpImportSettings *fbxSettings) {
    TString uuid = fbxSettings->subItem("Pose", true);
    Pose *pose = Engine::loadResource<Pose>(uuid);
    if(pose == nullptr) {
        pose = Engine::objectCreate<Pose>(uuid);
    }

    for(auto it : fbxSettings->m_bones) {
        aiVector3D scl, rot, pos;
        it->mOffsetMatrix.Decompose(scl, rot, pos);

        Bone b;
        b.setPosition(Vector3(pos.x, pos.y, pos.z) * fbxSettings->customScale());
        b.setRotation(Vector3(rot.x, rot.y, rot.z) * RAD2DEG);
        b.setScale(Vector3(scl.x, scl.y, scl.z));

        auto result = fbxSettings->m_actors.find(it->mName.C_Str());
        if(result != fbxSettings->m_actors.end()) {
            b.setIndex(Mathf::hashString(result->second->name()));
        }

        pose->addBone(&b);
    }

    fbxSettings->saveSubData(pose, "Pose", MetaType::name<Pose>());

    if(fbxSettings->m_rootBone) {
        Armature *armature = static_cast<Armature *>(fbxSettings->m_rootBone->addComponent("Armature"));
        armature->setBindPose(pose);

        for(auto r : fbxSettings->m_renders) {
            SkinnedMeshRender *render = static_cast<SkinnedMeshRender *>(r);
            render->setArmature(armature);
        }
    }
}
