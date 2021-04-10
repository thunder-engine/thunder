#include "renderrtsystem.h"

#include <components/actor.h>
#include <components/transform.h>
#include <components/camera.h>
#include <components/scene.h>
#include <components/meshrender.h>
#include <components/baselight.h>

#include <resources/mesh.h>

#include <tracer/worker.h>

#include <analytics/profiler.h>

#include <log.h>

#include <algorithm>

#define MAX_DEPTH 33 // 9

RenderRTSystem::RenderRTSystem(Engine *engine) :
        RenderSystem(),
        m_DepthIndex(0),
        m_pEngine(engine),
        m_BvhDirty(true) {
    PROFILE_FUNCTION();

}

RenderRTSystem::~RenderRTSystem() {
    PROFILE_FUNCTION();

}

bool RenderRTSystem::init() {
    PROFILE_FUNCTION();

    return true;
}

const char *RenderRTSystem::name() const {
    return "RenderRT";
}

void RenderRTSystem::update(Scene *scene) {
    PROFILE_FUNCTION();

    Camera *camera = Camera::current();
    if(camera) {
        // Build BVH
        if(m_BvhDirty) {
            m_Components.clear();
            combineComponents(scene);

            m_Vertices.clear();
            m_Normals.clear();
            m_Uvs.clear();
            m_Triangles.clear();

            for(auto it : m_Components) {
                MeshRender *mesh = dynamic_cast<MeshRender *>(it);
                if(mesh) {
                    insertMesh(mesh);
                }
            }
            composeBVH(BASIC);

            m_BvhDirty = false;
        }


    }
}

int RenderRTSystem::threadPolicy() const {
    return Main;
}

void RenderRTSystem::composeBVH(int mode) {
    // uvUnwrap task must be placed here
    if(mode == BAKING) {
        // Build UV tree
        TriangleList list;
        copy(m_Triangles.begin(), m_Triangles.end(), back_inserter(list));
        m_UVTree.left   = -1;
        m_UVTree.escape = -1;

        int index;
        buildTriangleTree(&m_UVTree, list, index, 0, true);
    }

    // Build triangle tree
    m_TriangleTree.left   = -1;
    m_TriangleTree.escape = -1;

    TriangleList list;
    copy(m_Triangles.begin(), m_Triangles.end(), back_inserter(list));
    m_Triangles.clear();

    m_DepthIndex = 0;
    buildTriangleTree(&m_TriangleTree, list, m_DepthIndex, 0);
}

const BvhVector &RenderRTSystem::bvh() const {
    return m_BVH;
}

void RenderRTSystem::combineComponents(Object *object) {
    for(auto &it : object->getChildren()) {
        Object *child = it;
        MeshRender *comp = dynamic_cast<MeshRender *>(child);
        if(comp) {
            if(comp->isEnabled()) {
                m_Components.push_back(comp);
            }
        } else {
            Actor *actor = dynamic_cast<Actor *>(child);
            if(actor && !actor->isEnabled()) {
                continue;
            }
            combineComponents(child);
        }
    }
}

void RenderRTSystem::insertMesh(MeshRender *mesh) {
    if(mesh) {
        Matrix4 model = mesh->actor()->transform()->worldTransform();

        Mesh *m = mesh->mesh();
        if(m) {
            Lod *lod = m->lod(0);

            // Geometry batching
            Vector3Vector &vert = lod->vertices();
            if(!vert.empty()) {
                uint32_t lastVertex = m_Vertices.size();
                m_Vertices.insert(m_Vertices.end(), vert.begin(), vert.end());
                for(uint32_t v = 0; v < vert.size(); v++) {
                    m_Vertices[lastVertex + v] = model * m_Vertices[lastVertex + v];
                }
            }

            Vector3Vector &norm = lod->normals();
            if(!norm.empty()) {
                Matrix3 rot = model.rotation();
                uint32_t lastNormal = m_Normals.size();
                m_Normals.insert(m_Normals.end(), norm.begin(), norm.end());
                for(uint32_t n = 0; n < norm.size(); n++) {
                    m_Normals[lastNormal + n] = rot * m_Normals[lastNormal + n];
                }
            }

            Vector2Vector &uv = lod->uv0();
            if(!norm.empty()) {
                m_Uvs.insert(m_Uvs.end(), uv.begin(), uv.end());
            }

            TriangleList list;
            IndexVector &index = lod->indices();
            for(uint32_t i = 0; i < index.size(); i += 3) {
                TriangleData t;
                t.instance = mesh;
                t.v[0] = index[i];
                t.v[1] = index[i + 1];
                t.v[2] = index[i + 2];

                list.push_back(t);
            }
            m_Triangles.insert(m_Triangles.end(), list.begin(), list.end());
        }
    }
}

void RenderRTSystem::buildTriangleTree(BvhNodeData *node, TriangleList &list, int &index, int depth, bool uv) {
    if(node) {
        int axis = depth % ((uv) ? 2 : 3);

        AABBox box = calcBox(list, uv);
        box.box(node->min, node->max);

        float mid = box.center[axis];
        if(depth < MAX_DEPTH && list.size() > 10) {
            depth++;

            BvhNodeData *r = new BvhNodeData;
            r->left   = -1;
            r->escape = -1;
            TriangleList right;

            BvhNodeData *l = new BvhNodeData;
            l->left   = -1;
            l->escape = -1;
            TriangleList left;

            Vector3 v1, v2, v3;

            auto it = list.begin();
            while(it != list.end()) {
                if(!uv) {
                    v1 = m_Vertices[(*it).v[0]];
                    v2 = m_Vertices[(*it).v[1]];
                    v3 = m_Vertices[(*it).v[2]];
                } else {
                    v1 = Vector3(m_Uvs[(*it).v[0]], 0.0f);
                    v2 = Vector3(m_Uvs[(*it).v[1]], 0.0f);
                    v3 = Vector3(m_Uvs[(*it).v[2]], 0.0f);
                }

                if(v1[axis] < mid && v2[axis] < mid && v3[axis] < mid) {
                    left.push_back(*it);
                    it = list.erase(it);
                } else {
                    if(v1[axis] >= mid && v2[axis] >= mid && v3[axis] >= mid) {
                        right.push_back(*it);
                        it = list.erase(it);
                    } else { // Keep in parent
                        ++it;
                    }
                }
            }

            if(!right.empty() || !left.empty()) {
                r->escape  = node->escape;
                int32_t i  = -1;
                buildTriangleTree(r, right, i, depth, uv);

                l->escape  = i;
                buildTriangleTree(l, left,  i, depth, uv);
                node->left = i;
            }
        }
        node->offset = m_Triangles.size();
        node->count = list.size();
        m_Triangles.insert(m_Triangles.end(), list.begin(), list.end());
        index = m_BVH.size();
        m_BVH.push_back(*node);
    }
}

AABBox RenderRTSystem::calcBox(TriangleList &list, bool uv) {
    Vector3 v1, v2, v3;

    Vector3 bb[2] = {Vector3(FLT_MAX), Vector3(-FLT_MAX)};

    for(auto it : list) {
        if(!uv) {
            v1 = m_Vertices[it.v[0]];
            v2 = m_Vertices[it.v[1]];
            v3 = m_Vertices[it.v[2]];
        } else {
            v1 = Vector3(m_Uvs[it.v[0]], 0.0f);
            v2 = Vector3(m_Uvs[it.v[1]], 0.0f);
            v3 = Vector3(m_Uvs[it.v[2]], 0.0f);
        }

        bb[0].x = MIN(bb[0].x, v1.x);
        bb[0].x = MIN(bb[0].x, v2.x);
        bb[0].x = MIN(bb[0].x, v3.x);

        bb[0].y = MIN(bb[0].y, v1.y);
        bb[0].y = MIN(bb[0].y, v2.y);
        bb[0].y = MIN(bb[0].y, v3.y);

        if(!uv) {
            bb[0].z = MIN(bb[0].z, v1.z);
            bb[0].z = MIN(bb[0].z, v2.z);
            bb[0].z = MIN(bb[0].z, v3.z);
        } else {
            bb[0].z = -0.5f;
        }
        bb[1].x = MAX(bb[1].x, v1.x);
        bb[1].x = MAX(bb[1].x, v2.x);
        bb[1].x = MAX(bb[1].x, v3.x);

        bb[1].y = MAX(bb[1].y, v1.y);
        bb[1].y = MAX(bb[1].y, v2.y);
        bb[1].y = MAX(bb[1].y, v3.y);

        if(!uv) {
            bb[1].z = MAX(bb[1].z, v1.z);
            bb[1].z = MAX(bb[1].z, v2.z);
            bb[1].z = MAX(bb[1].z, v3.z);
        } else {
            bb[1].z = 0.5f;
        }
    }
    AABBox result;
    result.setBox(bb[0], bb[1]);

    return result;
}

void RenderRTSystem::emitPhotons() {
    m_Photons.clear();

    /// \todo: Return random seed
    //r.seed(mPhotonsRandomSeed);

    int photonsSamples = 100;

    for(const auto &it : m_Components) {
        BaseLight *light = dynamic_cast<BaseLight *>(it);
        if(light) {
            for(int i = 0; i < photonsSamples; i++) {
                int b = 1;
                Vector3 dir = RANGE(Vector3(-1.0), Vector3(1.0));
                dir.normalize();

                Vector3 eye; // = light->eye;
                Ray ray(eye, dir);

                Vector3 rgb(1.0); // Light color
                Vector3 pos;
                Vector3 n;

                // RayTrace primitives
                int index = Worker::rayTrace(this, ray, &pos, &n, 0);
                while(index > -1 && b <= 5) {
                    //Material *mat = &m_pScene->mMaterials[prim->material];

                    Vector3 diff;// = mat->diffuse;
                    float refr = 0.0f;// mat->refraction;
                    float refl = 0.0f;// mat->reflection
                    float ior  = 2.8f;

                    rgb.x = MIN(diff.x, rgb.x);
                    rgb.y = MIN(diff.y, rgb.y);
                    rgb.z = MIN(diff.z, rgb.z);

                    bool inside = false;
                    if(ray.dir.dot(n) > 0) {
                        n       = -n;
                        inside  = true;
                    }
/*
                    // Shadow photon
                    Vector3 old = ray.pos;
                    ray.pos = pos;
                    Vector3 sp;
                    if(Worker::rayTrace(m_pScene, ray, &sp) > -1) {
                        // Store data
                        photon_data p;
                        p.dir = ray.dir;
                        p.pos = sp;
                        p.rgb = Vector3(-0.25);
                        mPhotons.push_back(p);
                    }
                    ray.pos = old;
*/
                    float c = 1.0;//fresnel(n.dot(ray.pos - pos), 1.0 / ior);

                    if(RANGE(0.0f, 1.0f) < refr * (1.0f - c)) {
                        ray = ray.refract(n, pos, (inside) ? ior : 1.0f);
                    } else if(RANGE(0.0f, 1.0f) < refl * c) {
                        ray = ray.reflect(n, pos);
                    } else {
                        rgb *= 1.0f / sqrt(static_cast<float>(b));
                        // Store data
                        PhotonData p;
                        p.dir = ray.dir;
                        p.pos = pos;
                        p.rgb = rgb;
                        m_Photons.push_back(p);

                        ray = ray.diffuse(n, pos, 0.0f, 1.0f);
                    }
                    // Trace next bounce
                    index = Worker::rayTrace(this, ray, &pos, &n, 0);
                    b++;
                }
            }
        }
    }
}

void RenderRTSystem::buildPhotonsTree(PhotonNodeData *node, int depth) {
    if(node) {
        int axis = depth % 3;

        Vector3 min, max;
        auto it = node->i.begin();
        while(it != node->i.end()) {
            min.x = MIN(min.x, (*it).pos.x);
            min.y = MIN(min.y, (*it).pos.y);
            min.z = MIN(min.z, (*it).pos.z);

            max.x = MAX(max.x, (*it).pos.x);
            max.y = MAX(max.y, (*it).pos.y);
            max.z = MAX(max.z, (*it).pos.z);

            ++it;
        }
        node->aabb.setBox(min, max);

        float mid = node->aabb.center[axis];

        if(depth < MAX_DEPTH && node->i.size() > 10) {
            depth++;

            PhotonNodeData *r = new PhotonNodeData;
            r->left  = nullptr;
            r->right = nullptr;

            PhotonNodeData *l = new PhotonNodeData;
            l->left  = nullptr;
            l->right = nullptr;

            it = node->i.begin();
            while(it != node->i.end()) {
                if((*it).pos[axis] < mid) {
                    l->i.push_back(*it);
                } else {
                    r->i.push_back(*it);
                }
                it  = node->i.erase(it);
            }

            if(!l->i.empty())
                node->left  = l;
            else
                delete l;

            if(!r->i.empty())
                node->right = r;
            else
                delete r;

            buildPhotonsTree(node->left,  depth);
            buildPhotonsTree(node->right, depth);
        }
    }
}
