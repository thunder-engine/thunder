#include "tracer/worker.h"

#include <QDebug>

#include <components/actor.h>
#include <components/camera.h>

#include "components/alightsourcert.h"

#include "renderrtsystem.h"

inline float fresnel(float VdotN, float eta) {
    float sqr_eta       = eta * eta;
    float etaCos        = eta * VdotN;
    float sqr_etaCos    = etaCos * etaCos;
    float one_minSqrEta = 1.0f - sqr_eta;
    float value         = etaCos - sqrt(one_minSqrEta + sqr_etaCos);
    value              *= value / one_minSqrEta;
    return MIN(1.0f, value * value);
}

struct StackData {
    Ray     ray;
    int     diff;
    int     refl;
    int     refr;
    Vector3 power;
};

Worker::Worker(Scheduler *parent, TaskData *task) {
    pParent  = parent;
    pTask    = task;

    mRect    = QRect(pTask->posX, pTask->posY, pTask->sizeX, pTask->sizeY);
    mAmbient = 0.2f;

    mPhotonAuto     = pParent->photonCollectAuto();
    mPhotonRadius   = pParent->photonCollectRadius();
    mPhotonCount    = pParent->photonCollectCount();
    mPhotonExposure = pParent->photonSamplesPhotons() * 0.010f;
}

void Worker::doWork() {
    QTime time;
    time.restart();

    Vector4 *rgba = new Vector4[pTask->sizeX * pTask->sizeY];
    memset(rgba, 0, sizeof(Vector4) * pTask->sizeX * pTask->sizeY);

    // Process task
    processCPU(rgba);

    delete []rgba;

    pTask->state = TASK_DONE;

    qDebug("Task done. Time spend: %0.1f sec.", time.elapsed() / 1000.0);

    emit finished(pTask->device);
}

void Worker::processCPU(Vector4 *rgba) {
    Vector4 *hdr  = pParent->hdr();

    uint32_t w = pParent->width();
    uint32_t h = pParent->height();
    int spp = pParent->spp();

    float aperture    = 1.0f / pParent->cameraFNumber();
    float cameraFocal = pParent->cameraFocal();
    int cameraSPP     = pParent->cameraSPP();
    bool cameraDOF    = pParent->cameraDOF();

    bool baking       = pParent->baking();

    Camera *camera = Camera::current();

    for(int s = 1; s <= spp; s++) {
        for(uint32_t y = 0; y < pTask->sizeY; y++) {
            for(uint32_t x = 0; x < pTask->sizeX; x++) {
                Vector4 result;

                if(pTask->state == TASK_DROP) {
                    emit finished(pTask->device);
                    return;
                }

                float u  = pTask->posX + x;
                float v  = pTask->posY + y;
                float pu = u / static_cast<float>(w);
                float pv = v / static_cast<float>(h);

                Ray ray;
                if(baking) {
                    Vector3 dir( (pu * 0.5f) * (static_cast<float>(w) / static_cast<float>(h)),
                                -(pv * 0.5f - 0.5f) * (static_cast<float>(h) / static_cast<float>(w)),
                                  1.0f);

                    ray = Ray(Vector3(dir.x, dir.y, 0.0f), dir);
                } else {
                    ray = camera->castRay(pu, pv);
                }

                for(int i = 0; i < cameraSPP; i++) {
                    Ray r = ray;

                    if(cameraDOF) {
                        float r1 = rand() / float(RAND_MAX) * 2.0f - 1.0f;
                        float r2 = rand() / float(RAND_MAX) * 2.0f - 1.0f;

                        Vector3 aim = ray.pos + ray.dir * cameraFocal;
                        // Camera jittering
                        Vector3 pos = ray.pos + Vector3(r1 * (aperture * 0.5f), r2 * (aperture * 0.5f), 0.0f);
                        Vector3 dir = aim - pos;
                        dir.normalize();
                        r = Ray(pos, dir);
                    }

                    result += Vector4(calcRadiance(r, baking), 1.0f) / float(cameraSPP);
                }

                uint32_t pos = x + y * pTask->sizeX;
                rgba[pos] += result;
            }
        }
        pTask->processed = s;

        for(uint32_t y = 0; y < pTask->sizeY; y++) {
            for(uint32_t x = 0; x < pTask->sizeX; x++) {
                uint32_t p   = x + y * pTask->sizeX;
                uint32_t pos = (pTask->posX + x) + (pTask->posY + y) * w;
                hdr[pos] = Vector4(rgba[p].x, rgba[p].y, rgba[p].z, rgba[p].w) * (1.0f / pTask->processed);
            }
        }

        emit updated(mRect);
    }
}

Vector3 Worker::backTrace(int index, const Vector3 &p, const Vector3 &n) {
    Vector3 result = Vector3(mAmbient);

    backTraceLights(result, index, p, n);

    return result;
}

void Worker::backTraceLights(Vector3 &result, const int index, const Vector3 &p, const Vector3 &n) {
    //for(auto &it : components) {
    //    ALightSourceRT *light   = dynamic_cast<ALightSourceRT *>(it.second);
    //    if(light) {
    //        light->backTrace(scene, pos, n, index, result);
    //    } else {
    //        backTraceLights(it.second->getChildren(), result, scene, index, pos, n);
    //    }
    //}
}

Vector3 Worker::collectPhotons(int index, const Vector3 &p, const Vector3 &n) {
    Vector3 energy;

    mCollected.clear();
    collectPhotonsTree(energy, p, n, mPhotonRadius, 1, nullptr); // &pScene->mPhotonTree

    if(mPhotonAuto && !mCollected.empty()) {
        auto it = mCollected.begin();
        while(it != mCollected.end()) {
            float weight = MAX(0.0f, -n.dot((*it).dir));
            weight *= (1.0f - sqrt((*it).distance)) / mPhotonExposure;
            energy += (*it).rgb * weight;

			++it;
        }
        //float distance  = mCollected.back().length;
        //energy /= 4 * PI * (distance * distance) * mPhotonCount;
    }

    return energy;
}

void Worker::collectPhotonsTree(Vector3 &e, const Vector3 &p, const Vector3 &n, float l, int d, PhotonNodeData *node) {
    if(node && node->aabb.intersect(p, l)) {
        auto it = node->i.begin();
        while(it != node->i.end()) {
            Vector3 d = p - (*it).pos;
            float distance = d.dot(d);

            if(!mPhotonAuto) {
                if(distance < l) {
                    float weight = MAX(0.0f, -n.dot((*it).dir));
                    weight *= (1.0f - sqrt(distance)) / mPhotonExposure;
                    e      += (*it).rgb * weight;
                }
            } else {
                auto i = mCollected.begin();
                while(i != mCollected.end()) {
                    if(distance < (*i).distance)
                        break;

					++i;
                }
                PhotonData data = (*it);
                data.distance   = distance;
                mCollected.insert(i, data);

                if(mCollected.size() > mPhotonCount) {
                    mCollected.pop_back();
                }
            }

			++it;
        }

        collectPhotonsTree(e, p, n, l, d + 1, node->left);
        collectPhotonsTree(e, p, n, l, d + 1, node->right);
    }
}

Vector3 Worker::calcRadiance(Ray &ray, bool uv) {
    Vector3 res(1.0f);
    list<StackData> stack;

    int diffuse = pParent->diffuse();
    uint32_t first  = pParent->firstBounce();
    uint32_t second = pParent->secondBounce();

    StackData data;
    data.ray   = ray;
    data.diff  = 0;
    data.refl  = 0;
    data.refr  = 0;
    data.power = Vector3(1.0f);
    stack.push_front(data);
    while(!stack.empty()) {
        Vector3 pos;
        Vector3 n;
        data    = stack.front();
        stack.pop_front();

        float minPower = 0.001f;
        if(data.power.x < minPower || data.power.y < minPower || data.power.z < minPower ||
           data.diff >= diffuse || data.refl >= 10 || data.refr >= 10) {
            res = Vector3();
            continue;
        }

        /// \todo return this
        int index = -1;//rayTrace(scene, data.ray, &pos, &n, data.diff, uv);
        if(index > -1) {
            //AMaterial *mat = &scene->mMaterials[prim->material];

            bool inside = false;
            if(data.ray.dir.dot(n) > 0) {
                n       = -n;
                inside  = true;
            }

            Vector3 diff(1.0f);//  = mat->diffuse;
            Vector3 emis;//  = mat->emissive;
            float refl = 0.0f;// mat->reflection;
            float refr = 0.0f;// mat->refraction;
            float ior  = 2.8f;

            float c = 1.0f;
            if(refr > 0.0f) {
                c = fresnel(n.dot(data.ray.pos - pos), 1.0f / ior);
            }

            uint32_t mode = (data.diff == 0) ? first : second;
            switch(mode) {
                case BACK_TRACING: {
                    if(refr > 0.0f) {
                        StackData d;
                        d.ray   = data.ray.refract(n, pos, (inside) ? ior : 1.0);
                        d.diff  = data.diff;
                        d.refl  = data.refl;
                        d.refr  = data.refr + 1;
                        d.power = data.power * refr * (1.0f - c);
                        stack.push_front(d);
                    }
                    if(refl > 0.0f) {
                        StackData d;
                        d.ray   = data.ray.reflect(n, pos);
                        d.diff  = data.diff;
                        d.refl  = data.refl + 1;
                        d.refr  = data.refr;
                        d.power = data.power * refl * c;
                        stack.push_front(d);
                    }

                    Vector3 f   = diff * (1.0f - MIX(refr, refl, c));
                    Vector3 rgb = backTrace(index, pos, n) * f;

                    if(data.refl > 0 || data.refr > 0) {
                        res = res + rgb * data.power;
                    } else {
                        res = res * rgb * data.power;
                    }
                } break;

                case PHOTON_MAPPING: {
                    res = res * collectPhotons(index, pos, n);
                } break;

                case PATH_TRACING: {
                    // Add next bounce to stack
                    if(RANGE(0.0f, 1.0f) < refr * (1.0f - c)) { // REFRACT
                        StackData d;
                        d.ray   = data.ray.refract(n, pos, (inside) ? ior : 1.0);
                        d.diff  = data.diff;
                        d.refl  = data.refl;
                        d.refr  = data.refr + 1;
                        d.power = data.power * refr * (1.0f - c);
                        stack.push_front(d);
                    } else {
                        if(RANGE(0.0f, 1.0f) < refl * c) { // REFLECT
                            StackData d;
                            d.ray   = data.ray.reflect(n, pos);
                            d.diff  = data.diff;
                            d.refl  = data.refl + 1;
                            d.refr  = data.refr;
                            d.power = data.power * refl * c;
                            stack.push_front(d);
                        } else { // DIFFUSE
                            // Calc radiance
                            res = emis + res * diff;

                            if(emis.x > 0.0f || emis.y > 0.0f || emis.z > 0.0f) { // We don't need to create a new bounce
                                continue;
                            }

                            StackData d;
                            d.ray   = data.ray.diffuse(n, pos, 0.0f, 1.0f);
                            d.diff  = data.diff + 1;
                            d.refl  = data.refl;
                            d.refr  = data.refr;
                            d.power = data.power;
                            stack.push_front(d);
                        }
                    }
                } break;

                default: break;
            }
        } else {
            res = Vector3(0.0f);
        }
    }

    return res;
}

int Worker::rayTrace(RenderRTSystem *system, Ray &ray, Vector3 *p, Vector3 *n, int depth, bool uv) {
    int32_t result = -1;
    float dist = FLT_MAX; // Camera far plane

    const BvhVector &bvh = system->bvh();

    if(!bvh.empty()) {
        TriangleData *triangle = nullptr;
        const BvhNodeData *current = nullptr; /// \todo &bvh[system->mIndex];

        while(current) {
            AABBox box;
            box.setBox(current->min, current->max);
            bool intersect  = ray.intersect(box, nullptr);
            if(intersect) {
                /// \todo rayTraceNode(system, ray, current, p, dist, &triangle, uv);
            }

            if(current->left > -1 && intersect) {
                current = &bvh[current->left];
            } else if(current->escape > -1) {
                current = &bvh[current->escape];
            } else {
                break;
            }
        }

        if(triangle) {
            /// \todo return this
            //result  = idFunction(triangle->instance->actor().reference());
            //if(n) {
            //    Vector3 v0(triangle->v[0].xyz);
            //    Vector3 v1(triangle->v[1].xyz);
            //    Vector3 v2(triangle->v[2].xyz);
            //
            //    Vector3 d = triangleWeights(*p, v0, v1, v2);
            //   *n = triangle->v[0].n * d.x + triangle->v[1].n * d.y + triangle->v[2].n * d.z;
            //    n->normalize();
            //}
        }
    }
    return result;
}

void Worker::rayTraceNode(RenderRTSystem *system, Ray ray, BvhNodeData *node, Vector3 *p, float &dist, TriangleData **result, bool uv) {
    Vector3 pos;
    for(uint32_t i = 0; i < node->count; i++) {
        /// \todo return this
        //TriangleData *t = &system->mTriangles[node->offset + i]; /// \todo: UV Triangle list
        //
        //Vector3 v0(t->v[0].xyz);
        //Vector3 v1(t->v[1].xyz);
        //Vector3 v2(t->v[2].xyz);
        //
        //if(uv) {
        //    Vector3 u0(t->v[0].uv, 1.0);
        //    Vector3 u1(t->v[1].uv, 1.0);
        //    Vector3 u2(t->v[2].uv, 1.0);
        //
        //    if(ray.intersect(u0, u1, u2, &pos, true)) {
        //        Vector3 d = triangleWeights(pos, u0, u1, u2);
        //
        //        if(p) {
        //           *p   = v0 * d.x + v1 * d.y + v2 * d.z;
        //        }
        //        *result = t;
        //    }
        //} else {
        //    if(ray.intersect(v0, v1, v2, &pos)) {
        //        Vector3 d = pos - ray.pos;
        //        float l = d.dot(d);
        //        if(l < dist) {
        //            dist = l;
        //            if(p) {
        //               *p = pos;
        //            }
        //            *result = t;
        //        }
        //    }
        //}
    }
}

Vector3 Worker::triangleWeights(const Vector3 &point, const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
    Vector3 ve1(v2 - v1);
    Vector3 ve2(v3 - v2);
    Vector3 ve3(v1 - v3);

    Vector3 p1(point - v1);
    Vector3 p2(point - v2);
    Vector3 p3(point - v3);

    float s1 = p2.cross(ve2).length();
    float s2 = p3.cross(ve3).length();
    float s3 = p1.cross(ve1).length();

    float sum = s1 + s2 + s3;

    return Vector3(s1 / sum, s2 / sum, s3 / sum);
}

Vector3 Worker::texureColor(const Vector3 &point, Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector2 &t1, Vector2 &t2, Vector2 &t3) {
    Vector3 d   = triangleWeights(point, v1, v2, v3);

    Vector2 pos = t1 * d.x + t2 * d.y + t3 * d.z;
/*
    int x = (int)(xf * canvas->w) % (canvas->w + 1);
    int y = (int)(yf * canvas->h) % (canvas->h + 1);
*/
    return Vector3(1.0);
}
