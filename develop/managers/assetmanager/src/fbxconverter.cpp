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

#include <fbxsdk.h>

#include "converters/converter.h"
#include "projectmanager.h"

#include "resources/material.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/meshrender.h"

#define HEADER  "Header"
#define DATA    "Data"

static Matrix3 gInvert;

struct index_data {
    int32_t vIndex;
    int32_t uIndex;
    int32_t xIndex;

    bool operator== (const index_data &right) {
        return (vIndex == right.vIndex) && (uIndex == right.uIndex);
    }
};
typedef vector<index_data>  indexVector;

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

    FbxManager *lSdkManager = FbxManager::Create();
    // Create an IOSettings object.
    FbxIOSettings *ios = FbxIOSettings::Create( lSdkManager, IOSROOT );
    lSdkManager->SetIOSettings(ios);

    FbxImporter *lImporter = FbxImporter::Create( lSdkManager, "" );
    // Initialize the importer.
    if(lImporter->Initialize(settings->source(), -1, lSdkManager->GetIOSettings()) == false) {
        Log(Log::ERR) << "Call to KFbxImporter::Initialize() failed." ;
        return 1;
    }
    // Create a new scene so it can be populated by the imported file.
    FbxScene *lScene = FbxScene::Create(lSdkManager, "Scene");
    // Import the contents of the file into the scene.
    lImporter->Import(lScene);

    // File format version numbers to be populated.
    int lFileMajor, lFileMinor, lFileRevision;
    // Populate the FBX file format version numbers with the import file.
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    FbxNode *lRootNode = lScene->GetRootNode();

    if(!s->useScale()) {
        FbxGlobalSettings &global = lScene->GetGlobalSettings();
        s->setCustomScale(static_cast<float>(global.GetSystemUnit().GetScaleFactor()) * 0.01f);
    }
    QStringList resources;

    list<Actor *> actors;
    for(int n = 0; n < lRootNode->GetChildCount(); n++) {
        Actor *a = importNode(lRootNode->GetChild(n), lScene, s, resources);
        if(a) {
            actors.push_back(a);
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

    lScene->Destroy(true);
    lImporter->Destroy();
    ios->Destroy();
    lSdkManager->Destroy();

    for(auto it : resources) {
        Engine::unloadResource(it.toStdString(), true);
    }

    Log(Log::INF) << "Mesh imported in:" << t.elapsed() << "msec";

    return 0;
}

Actor *FBXConverter::importNode(FbxNode *node, FbxScene *scene, FbxImportSettings *settings, QStringList &list) {
    FbxNodeAttribute *attrib = node->GetNodeAttribute();

    Actor *actor = Engine::objectCreate<Actor>();

    FbxDouble3 p = node->LclTranslation.Get();
    actor->transform()->setPosition(gInvert * (Vector3(static_cast<areal>(p[0]),
                                                       static_cast<areal>(p[1]),
                                                       static_cast<areal>(p[2])) * settings->customScale()));

    FbxDouble3 r = node->LclRotation.Get();
    actor->transform()->setEuler(gInvert * Vector3(static_cast<areal>(r[0]),
                                                   static_cast<areal>(r[1]),
                                                   static_cast<areal>(r[2])));

    FbxDouble3 s = node->LclScaling.Get();
    actor->transform()->setScale(Vector3(static_cast<areal>(s[0]),
                                         static_cast<areal>(s[1]),
                                         static_cast<areal>(s[2])));

    QString path(node->GetNameOnly());

    actor->setName(qPrintable(path));

    if(attrib && attrib->GetAttributeType() == FbxNodeAttribute::eMesh) {
        MeshSerial *mesh = importMesh(static_cast<FbxMesh *>(attrib), settings->customScale());

        if(mesh->flags() & Mesh::ATTRIBUTE_ANIMATED) {
            importAnimation(node, scene);
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
/*
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
}

MeshSerial *FBXConverter::importMesh(FbxMesh *m, float scale) {
    MeshSerial *mesh = new MeshSerial;
    mesh->setMode(Mesh::MODE_TRIANGLES);

    vector<FbxCluster *> boneCluster;

    for(int d = 0; d < m->GetDeformerCount(); d++) {
        FbxDeformer *deformer = m->GetDeformer(d);
        if(deformer->GetDeformerType() == FbxDeformer::eSkin) {
            mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_ANIMATED);

            FbxSkin *skin = static_cast<FbxSkin *>(deformer);
            for(int s = 0; s < skin->GetClusterCount(); s++) {
                FbxCluster *bone = skin->GetCluster(s);
                uint32_t j;
                for(j = 0; j < boneCluster.size(); j++) {
                    if(boneCluster[j] == bone) break;
                }
                if(j != boneCluster.size()) {
                    continue;
                }
                boneCluster.push_back(bone);
            }
            break;
        }
    }

    Mesh::Lod l;
    l.material = Engine::loadResource<Material>("{00000000-0402-0000-0000-000000000000}");

    // Export
    FbxVector4 *verts = m->GetControlPoints();

    FbxGeometryElementVertexColor *colors = m->GetElementVertexColor();
    if(colors) {
        colors->IncRefCount();
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_COLOR);
    }

    FbxGeometryElementUV *uv = m->GetElementUV();
    if(uv) {
        uv->IncRefCount();
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_UV0);
    } else {
        Log(Log::WRN) << "No uv exist";
    }

    FbxGeometryElementNormal *normals = m->GetElementNormal();
    if(normals) {
        normals->IncRefCount();
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_NORMALS);
    } else {
        Log(Log::WRN) << "No normals exist";
    }

    FbxGeometryElementTangent *tangents = m->GetElementTangent();
    if(tangents) {
        tangents->IncRefCount();
        mesh->setFlags(mesh->flags() | Mesh::ATTRIBUTE_TANGENTS);
    } else {
        Log(Log::WRN) << "No tangents exist";
    }

    indexVector indices;

    int tCount  = m->GetPolygonCount();
    for(int triangle = 0; triangle < tCount; triangle++) {
        int size = m->GetPolygonSize(triangle);
        for(int h = 0; h < ((size == 4) ? 2 : 1); h++) {
            for(int k = 0; k < 3; k++) {
                index_data data;
                data.vIndex = m->GetPolygonVertex(triangle, (k == 0) ? k : k + h);
                data.uIndex	= m->GetTextureUVIndex(triangle, (k == 0) ? k : k + h);
                data.xIndex = m->GetPolygonVertexIndex(triangle) + ((k == 0) ? k : k + h);

                uint32_t count = indices.size();

                bool create	= true;
                for(uint32_t i = 0; i < count; i++) {
                    if(indices[i] == data) {
                        // Vertex already exist. Just add it into indices list
                        l.indices.push_back(i);
                        create	= false;
                        break;
                    }
                }

                // Add vertex to arrays
                if(create) {
                    indices.push_back(data);

                    FbxVector4 v = verts[data.vIndex];
                    l.vertices.push_back(gInvert * (Vector3(static_cast<areal>(v[0]),
                                                            static_cast<areal>(v[1]),
                                                            static_cast<areal>(v[2])) * scale));
                    l.indices.push_back(count);

                    if(colors) {
                        FbxColor c = colors->GetDirectArray().GetAt(data.vIndex);
                        l.colors.push_back(Vector4(static_cast<areal>(c.mRed),
                                                   static_cast<areal>(c.mGreen),
                                                   static_cast<areal>(c.mBlue),
                                                   static_cast<areal>(c.mAlpha)));
                    }

                    if(normals) {
                        switch (normals->GetReferenceMode()) {
                            case FbxLayerElement::eDirect: {
                                v = normals->GetDirectArray().GetAt(data.xIndex);
                            } break;
                            case FbxLayerElement::eIndexToDirect: {
                                int index = normals->GetIndexArray().GetAt(data.xIndex);
                                v = normals->GetDirectArray().GetAt(index);
                            } break;
                            default: {
                                Log(Log::ERR) << "Invalid normals reference mode";
                            } break;
                        }
                        l.normals.push_back(gInvert * Vector3(static_cast<areal>(v[0]),
                                                              static_cast<areal>(v[1]),
                                                              static_cast<areal>(v[2])));
                    }

                    if(tangents) {
                        switch (tangents->GetReferenceMode()) {
                            case FbxLayerElement::eDirect: {
                                v  = tangents->GetDirectArray().GetAt(data.xIndex);
                            } break;
                            case FbxLayerElement::eIndexToDirect: {
                                int index = tangents->GetIndexArray().GetAt(data.xIndex);
                                v  = tangents->GetDirectArray().GetAt(index);
                            } break;
                            default: {
                                Log(Log::ERR) << "Invalid tangents reference mode";
                            } break;
                        }
                        l.tangents.push_back(gInvert * Vector3(static_cast<areal>(v[0]),
                                                               static_cast<areal>(v[1]),
                                                               static_cast<areal>(v[2])));
                    }

                    if(uv) {
                        v = uv->GetDirectArray().GetAt(data.uIndex);
                        l.uv0.push_back(Vector2(static_cast<areal>(v[0]),
                                                static_cast<areal>(v[1])));
                    }

                    if(mesh->flags() & Mesh::ATTRIBUTE_ANIMATED) {
                        int32_t i = 0;
                        Vector4 weights, bones;
                        for(uint32_t b = 0; b < boneCluster.size(); b++) {
                            int *controllIndices = boneCluster[b]->GetControlPointIndices();
                            double *w = boneCluster[b]->GetControlPointWeights();

                            float weight = 0.0f;
                            for(int index = 0; index < boneCluster[b]->GetControlPointIndicesCount(); index++) {
                                if(controllIndices[index] == data.vIndex) {
                                    weight = static_cast<areal>(w[index]);
                                    break;
                                }
                            }

                            if(weight > 0.0f && i < 4) {
                                bones[i] = b;
                                weights[i] = weight;
                                i++;
                            }
                        }

                        l.bones.push_back(bones);
                        l.weights.push_back(weights);
                    }
                }
            }
        }
    }
    mesh->addLod(l);

    return mesh;
}

void FBXConverter::importAnimation(FbxNode *node, FbxScene *scene) {
    FbxAnimStack *stack = scene->GetCurrentAnimationStack();
    for(int i = 0; i < stack->GetMemberCount(); i++) {
        //FbxAnimLayer *layer = stack->GetMember<FbxAnimLayer>(i);
        //FbxAnimCurve *xPos = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        //FbxAnimCurve *xRot = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
        //FbxAnimCurve *xScl = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
    }
}
