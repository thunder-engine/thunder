#include "fbxconverter.h"

#include "resources/mesh.h"
#include "file.h"
#include "log.h"

#include <abson.h>

#include <QFileInfo>

#include "baseconvertersettings.h"
#include "projectmanager.h"

Matrix3 gInvert;

struct index_data {
    int             vIndex;
    int             uIndex;
    int             xIndex;

    bool operator== (const index_data &right) {
        return (vIndex == right.vIndex) && (uIndex == right.uIndex) && (xIndex == right.xIndex);
    }
};
typedef vector<index_data>  indexVector;

FBXConverter::FBXConverter() {
    gInvert[0]    =-1.0;
    gInvert[1]    = 0.0;
    gInvert[2]    = 0.0;

    gInvert[3]    = 0.0;
    gInvert[4]    = 0.0;
    gInvert[5]    = 1.0;

    gInvert[6]    = 0.0;
    gInvert[7]    = 1.0;
    gInvert[8]    = 0.0;

}

FBXConverter::~FBXConverter() {

}

string FBXConverter::format() const {
    return "fbx";
}

IConverter::ContentTypes FBXConverter::type() const {
    return IConverter::ContentMesh;
}

uint8_t FBXConverter::convertFile(IConverterSettings *settings) {
    Mesh mesh;

    importFBX(settings->source(), mesh);

    //exportAnimation(dst);

    for(auto &surface : mesh.m_Surfaces) {
        for(auto &lod : surface.lods) {
            lod.material    = nullptr;
        }
    }

    QFile file(ProjectManager::instance()->importPath() + "/" + settings->destination());
    if(file.open(QIODevice::WriteOnly)) {
        AByteArray data   = ABson::save(Engine::toVariant(&mesh));
        file.write((const char *)&data[0], data.size());
        file.close();
        return 0;
    }

    return 1;
}

void FBXConverter::importFBX(const string &src, Mesh &mesh) {
    KFbxSdkManager *lSdkManager = KFbxSdkManager::Create();
    // Create an IOSettings object.
    KFbxIOSettings *ios         = KFbxIOSettings::Create( lSdkManager, IOSROOT );
    lSdkManager->SetIOSettings(ios);

    KFbxImporter *lImporter	= KFbxImporter::Create( lSdkManager, "" );
    // Initialize the importer.
    if(lImporter->Initialize(src.c_str(), -1, lSdkManager->GetIOSettings()) == false) {
        Log(Log::ERR) << "Call to KFbxImporter::Initialize() failed." << lImporter->GetLastErrorString();
        return;
    }
    // Create a new scene so it can be populated by the imported file.
    KFbxScene *lScene = KFbxScene::Create(lSdkManager, "Scene");
    // Import the contents of the file into the scene.
    lImporter->Import(lScene);

    // File format version numbers to be populated.
    int lFileMajor, lFileMinor, lFileRevision;
    // Populate the FBX file format version numbers with the import file.
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

    importDynamic(lScene->GetRootNode(), mesh);

    lScene->Destroy(true, true);
    lImporter->Destroy();
    ios->Destroy();
    lSdkManager->Destroy();
}

void FBXConverter::importDynamic(KFbxNode *lRootNode, Mesh &mesh) {
    vector<KFbxCluster *> bones;

    for(int n = 0; n < lRootNode->GetChildCount(); n++) {
        KFbxNode *node              = lRootNode->GetChild(n);
        KFbxNodeAttribute *attrib   = node->GetNodeAttribute();
        if(attrib && attrib->GetAttributeType() == KFbxNodeAttribute::eMESH) {
            KFbxMesh *m = (KFbxMesh *)attrib;
/*
            bool valid  = false;
            for(int d = 0; d < m->GetDeformerCount(); d++) {
                KFbxDeformer *deformer	= m->GetDeformer(d);
                if(deformer->GetDeformerType() == KFbxDeformer::eSKIN) {
                    mesh.type       = MESH_ANIMATED;
                    KFbxSkin *skin  = (KFbxSkin *)deformer;
                    // Export bone hierarchy
                    for(int s = 0; s < skin->GetClusterCount(); s++) {
                        KFbxCluster *bone   = skin->GetCluster(s);
                        int j;
                        for(j = 0; j < bones.size(); j++) {
                            if(bones[j] == bone) break;
                        }
                        if(j != bones.size())
                            continue;

                        bones.push_back(bone);
                    }
                    valid	= true;
                    break;
                }
            }
            if(!valid)
                continue;
*/
            Mesh::Surface s;
            Mesh::Lod l;

            s.collision	= false;
            // Transform marix
            KFbxXMatrix mTransform	= node->EvaluateGlobalTransform();
            /// \todo: Is it needed?
            Matrix3 transform;
            transform[0]    = mTransform.mData[0].mData[0];
            transform[1]    = mTransform.mData[0].mData[1];
            transform[2]    = mTransform.mData[0].mData[2];

            transform[3]    = mTransform.mData[1].mData[0];
            transform[4]    = mTransform.mData[1].mData[1];
            transform[5]    = mTransform.mData[1].mData[2];

            transform[6]    = mTransform.mData[2].mData[0];
            transform[7]    = mTransform.mData[2].mData[1];
            transform[8]    = mTransform.mData[2].mData[2];
            transform.orthonormalize();

            indexVector indices;

            // Polygons
            int tCount			= m->GetPolygonCount();
            l.indices			= Mesh::IndexVector(tCount * 3);
            uint32_t index      = 0;
            // Export
            KFbxVector4 *verts;
            KFbxGeometryElementUV *uv;
            KFbxGeometryElementNormal *normals;
            KFbxGeometryElementTangent *tangents;

            verts		= m->GetControlPoints();
            uv			= m->GetElementUV();
            normals		= m->GetElementNormal();
            tangents	= m->GetElementTangent();

            for(int triangle = 0; triangle < tCount; triangle++) {
                for(int k = 0; k < 3; k++) {

                    index_data data;
                    data.vIndex = m->GetPolygonVertex(triangle, k);
                    data.uIndex	= m->GetTextureUVIndex(triangle, k);
                    data.xIndex = m->GetPolygonVertexIndex(triangle) + k;

                    bool create	= true;
                    int count	= indices.size();

                    for(int vert = 0; vert < count; vert++) {
                        index_data *it	= &indices[vert];
                        if(*it == data) {
                            // Vertex already exist. Just add it into indices list
                            l.indices[triangle * 3 + k]	= vert;
                            create	= false;
                            break;
                        }
                    }

                    // Add vertex to array
                    if(create) {
                        Mesh::Vertex vert;
                        KFbxVector4 v;

                        v           = mTransform.MultT(verts[data.vIndex]);
                        vert.xyz    = Vector4(v.GetAt(0), v.GetAt(1), v.GetAt(2), 1.0);

                        if(normals) {
                            bool mode   = (normals->GetMappingMode() == KFbxLayerElement::eBY_CONTROL_POINT);
                            switch (normals->GetReferenceMode()) {
                                case KFbxLayerElement::eDIRECT: {
                                    v   = normals->GetDirectArray().GetAt(data.xIndex);
                                } break;
                                case KFbxLayerElement::eINDEX_TO_DIRECT: {
                                    int index   = normals->GetIndexArray().GetAt(data.xIndex);
                                    v           = normals->GetDirectArray().GetAt(index);
                                } break;
                                default: {
                                    Log(Log::ERR) << "Invalid normals reference mode";
                                } break;

                            }
                            vert.n  = Vector3(v.GetAt(0), v.GetAt(1), v.GetAt(2));
                        } else {
                            Log(Log::WRN) << "No normals exist";
                        }

                        if(tangents) {
                            switch (tangents->GetReferenceMode()) {
                                case KFbxLayerElement::eDIRECT: {
                                    v       = tangents->GetDirectArray().GetAt(data.xIndex);
                                } break;
                                case KFbxLayerElement::eINDEX_TO_DIRECT: {
                                    int index   = tangents->GetIndexArray().GetAt(data.xIndex);
                                    v           = tangents->GetDirectArray().GetAt(index);
                                } break;
                                default: {
                                    Log(Log::ERR) << "Invalid tangents reference mode";
                                } break;

                            }
                            vert.t  = Vector3(v.GetAt(0), v.GetAt(1), v.GetAt(2));
                        } else {
                            Log(Log::WRN) << "No tangents exist";
                        }

                        if(uv) {
                            v       = uv->GetDirectArray().GetAt(data.uIndex);
                            vert.uv0= Vector2(v.GetAt(0), v.GetAt(1));
                        }

                        if(!mesh.isAnimated()) {
                            vert.n  = transform * vert.n;
                            vert.t  = transform * vert.t;
                        }

                        vert.xyz    = Vector4(gInvert * Vector3(vert.xyz.v), vert.xyz.w);
                        vert.n      = gInvert * vert.n;
                        vert.t      = gInvert * vert.t;

                        l.indices[triangle * 3 + k]		= count;

                        indices.push_back(data);

                        if(mesh.isAnimated()) {
                            int w	= 0;
                            for(int b = 0; b < bones.size(); b++) {
                                int *indices	= bones[b]->GetControlPointIndices();
                                double *weights = bones[b]->GetControlPointWeights();
                                for(int i = 0; i < bones[b]->GetControlPointIndicesCount(); i++) {
                                    if(indices[i] == data.vIndex) {
                                        //KFbxNode *bone  = bones[b]->GetLink();

                                        vert.index[w]	= (float)b;
                                        vert.weight[w]	= (float)weights[i];

                                        w++;
                                    }
                                }
                            }
                            vert.xyz[3]	= (float)w;
                        }

                        l.vertices.push_back(vert);

                    } // end Creation
                    index++;
                }
            }
            s.lods.push_back(l);
            mesh.m_Surfaces.push_back(s);
        }
    }
/*
    if(pMesh->type == MESH_ANIMATED) {
        pMesh->jCount			= bones.size();
        pMesh->aAnim->priority	= new char[pMesh->jCount];

        pMesh->jArray			= new joint_data[pMesh->jCount];
        for(int s = 0; s < pMesh->jCount; s++) {
            KFbxNode *bone		= bones[s]->GetLink();
            KFbxNode *parent	= bone->GetParent();
            int j;
            for(j = 0; j < pMesh->jCount; j++)
                if(bones[j]->GetLink() == parent) break;

            // Getting Joint priority (Using with animation blending)
            int prop	= 100;
            pMesh->aAnim->priority[s]		= prop;

            if(j == bones.size()) {
                pMesh->jArray[s].parent		= 0; // Root Joint
                pMesh->jArray[s].iparent	= ROOT;
            } else {
                pMesh->jArray[s].parent		= &pMesh->jArray[j];
                pMesh->jArray[s].iparent	= j;
            }

            memset(pMesh->jArray[s].name, 0, 32);
            strncpy(pMesh->jArray[s].name, bone->GetName(), 31);

            int proxy	= 0;
            pMesh->jArray[s].proxy		= proxy;

            int emitter	= 0;
            pMesh->jArray[s].emitter	= emitter;
        }
    }
*/
}

void FBXConverter::importAFAnimation(KFbxScene *lScene) {
/*
    KTime gPeriod, gStart, gStop, gCurrent;
    gPeriod.SetTime(0, 0, 0, 1, 0, lScene->GetGlobalSettings().GetTimeMode());

    KTimeSpan lTimeLineTimeSpan;
    lScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);
    gStart			= lTimeLineTimeSpan.GetStart();
    gStop			= lTimeLineTimeSpan.GetStop();

    pMesh->aAnim.fCount = static_cast<int>((gStop - gStart).GetSecondDouble() / gPeriod.GetSecondDouble());
    pMesh->aAnim.frames	= new ASFrame * [pMesh->jCount];

    int joint;
    for(joint = 0; joint < pMesh->jCount; joint++) {
        KFbxNode *bone	= pMesh->bones[joint]->GetLink();
        pMesh->aAnim.frames[joint]	= new ASFrame[pMesh->aAnim.fCount + 1];

        int cur_frame	= 0;
        for(gCurrent = gStart; gCurrent < gStop; gCurrent += gPeriod) {
            KFbxXMatrix mTransform	= bone->EvaluateGlobalTransform(gCurrent);

            Matrix3 transform;
            transform.mat[0]	= mTransform.mData[0].mData[0];
            transform.mat[1]	= mTransform.mData[0].mData[1];
            transform.mat[2]	= mTransform.mData[0].mData[2];

            transform.mat[3]	= mTransform.mData[1].mData[0];
            transform.mat[4]	= mTransform.mData[1].mData[1];
            transform.mat[5]	= mTransform.mData[1].mData[2];

            transform.mat[6]	= mTransform.mData[2].mData[0];
            transform.mat[7]	= mTransform.mData[2].mData[1];
            transform.mat[8]	= mTransform.mData[2].mData[2];

            Matrix3 m;
            Vector3 vec;
            Quaternion  quat;

            KFbxVector4	p		= mTransform.GetT();
            vec					= invert * Vector3(p.GetAt(0), p.GetAt(1), p.GetAt(2));
            pMesh->aAnim.frames[joint][cur_frame].vector		= vec;

            m					= transform;
            m.orthonormalize();

            m					= invert * m;
            m					= m.inverse();

            quat				= Quaternion (m);
            pMesh->aAnim.frames[joint][cur_frame].quaternion	= quat;

            cur_frame++;
        }
    }

    for(joint = 0; joint < pMesh->jCount; joint++) {
        for(int frame = 0; frame < pMesh->aAnim.fCount; frame++) {
            int iparent	= pMesh->jArray[joint].iparent;
            Vector3 parent	= Vector3(0, 0, 0);

            if(iparent != -1) {
                parent	= pMesh->aAnim.frames[iparent][frame].vector;
            }

            pMesh->aAnim.frames[joint][frame].relative	= pMesh->aAnim.frames[joint][frame].vector - parent;
        }
    }
*/
}

void FBXConverter::saveAnimation(const string &dst) {
/*
    FILE *fp	= fopen(filename, "wb");
    if(fp) {
        fprintf(fp, AB_SIGNATURE);

        fwrite(&pMesh->aAnim.fCount,	sizeof(short), 1, fp);
        // Save joint animation priority
        fwrite(pMesh->aAnim.priority,	sizeof(char) * pMesh->jCount,	1, fp);
        // Save count of joint animation keys
//		fwrite(pMesh->aAnim.kCount,		sizeof(short) * pMesh->jCount,	1, fp);

        for(int b = 0; b < pMesh->jCount; b++) {
            // Save name of bone
            fwrite(pMesh->jArray[b].name, 32, 1, fp);
            for (int cur_frame = 0; cur_frame < pMesh->aAnim.fCount; cur_frame++) { // pMesh->aAnim.kCount[b]
//				fwrite(&pMesh->aAnim.frames[b][cur_frame].fPos,			sizeof(short),			1, fp);
                fwrite(&pMesh->aAnim.frames[b][cur_frame].quaternion,	sizeof(Quaternion ),	1, fp);

                if(ABSOLUTE_POS) {
                    fwrite(&pMesh->aAnim.frames[b][cur_frame].vector,	sizeof(Vector3),		1, fp);
                } else {
                    fwrite(&pMesh->aAnim.frames[b][cur_frame].relative,	sizeof(Vector3),		1, fp);
                }
            }
        }
        fclose(fp);
    }
*/
}
