#include "tracer/workergpu.h"

//#include <QDebug>
//
//#include <components/actor.h>
//#include <components/camera.h>
//
//#include "components/alightsourcert.h"
//
//#include <CL/cl.h>
//
//typedef vector<cl_device_id> DeviceVector;
//typedef vector<cl_context>   ContextVector;
//typedef vector<cl_program>   ProgramVector;
//
//inline float fresnel(float VdotN, float eta) {
//    float sqr_eta       = eta * eta;
//    float etaCos        = eta * VdotN;
//    float sqr_etaCos    = etaCos * etaCos;
//    float one_minSqrEta = 1.0f - sqr_eta;
//    float value         = etaCos - sqrt(one_minSqrEta + sqr_etaCos);
//    value              *= value / one_minSqrEta;
//    return MIN(1.0f, value * value);
//}
//
//struct StackData {
//    Ray     ray;
//    int     diff;
//    int     refl;
//    int     refr;
//    Vector3 power;
//};
//
//Worker::Worker(Scheduler *parent, TaskData *task) {
//    pParent  = parent;
//    pTask    = task;
//
//    mRect    = QRect(pTask->pos.x, pTask->pos.y, pTask->size.x, pTask->size.y);
//    mAmbient = 0.2f;
//
//    mPhotonAuto     = pParent->photonCollectAuto();
//    mPhotonRadius   = pParent->photonCollectRadius();
//    mPhotonCount    = pParent->photonCollectCount();
//    mPhotonExposure = pParent->photonSamplesPhotons() * 0.010f;
//}
//
//void Worker::doWork() {
//    QTime time;
//    time.restart();
//
//    Vector4 *rgba = new Vector4[static_cast<uint32_t>(pTask->size.x) * static_cast<uint32_t>(pTask->size.y)];
//    memset(rgba, 0, sizeof(Vector4) * static_cast<uint32_t>(pTask->size.x) * static_cast<uint32_t>(pTask->size.y));
//
//    // Process task
//    if(pTask->device > -1) {
//        processGPU(rgba);
//    } else {
//        processCPU(rgba);
//    }
//
//    delete []rgba;
//
//    pTask->state    = TASK_DONE;
//
//    qDebug("Task done. Time spend: %0.1f sec.", time.elapsed() / 1000.0);
//
//    emit finished(pTask->device);
//}
//
//void Worker::processCPU(Vector4 *rgba) {
//    Vector4 *hdr  = pParent->hdr();
//
//    int w   = pParent->width();
//    int h   = pParent->height();
//    int spp = pParent->spp();
//
//    float aperture    = 1.0f / pParent->cameraFNumber();
//    float cameraFocal = pParent->cameraFocal();
//    int cameraSPP     = pParent->cameraSPP();
//    bool cameraDOF    = pParent->cameraDOF();
//
//    bool baking       = pParent->baking();
//
//    ACamera *camera   = pParent->camera();
//
//    for(int s = 1; s <= spp; s++) {
//        for(int y = 0; y < (int)pTask->size.y; y++) {
//            for(int x = 0; x < (int)pTask->size.x; x++) {
//                Vector4 result;
//
//                if(pTask->state == TASK_DROP) {
//                    qDebug("Task drop.");
//
//                    emit finished(pTask->device);
//                    return;
//                }
//
//                float u  = pTask->pos.x + x;
//                float v  = pTask->pos.y + y;
//                float pu = u / static_cast<float>(w);
//                float pv = v / static_cast<float>(h);
//
//                Ray ray;
//                if(baking) {
//                    Vector3 dir( (pu * 0.5f) * (static_cast<float>(w) / static_cast<float>(h)),
//                                -(pv * 0.5f - 0.5f) * (static_cast<float>(h) / static_cast<float>(w)),
//                                  1.0f);
//
//                    ray = Ray(Vector3(dir.x, dir.y, 0.0f), dir);
//                } else {
//                    ray = camera->castRay(pu, pv);
//                }
//
//                for(int i = 0; i < cameraSPP; i++) {
//                    Ray r = ray;
//
//                    if(cameraDOF) {
//                        float r1 = rand() / float(RAND_MAX) * 2.0f - 1.0f;
//                        float r2 = rand() / float(RAND_MAX) * 2.0f - 1.0f;
//
//                        Vector3 aim = ray.pos + ray.dir * cameraFocal;
//                        // Camera jittering
//                        Vector3 pos = ray.pos + Vector3(r1 * (aperture * 0.5f), r2 * (aperture * 0.5f), 0.0f);
//                        Vector3 dir = aim - pos;
//                        dir.normalize();
//                        r = Ray(pos, dir);
//                    }
//
//                    result += Vector4(calcRadiance(r, baking), 1.0f) / float(cameraSPP);
//                }
//
//                int pos    = x + y * pTask->size.x;
//                rgba[pos] += result;
//            }
//        }
//        pTask->processed = s;
//
//        for(uint32_t y = 0; y < pTask->size.y; y++) {
//            for(uint32_t x = 0; x < pTask->size.x; x++) {
//                int p    = x + y * pTask->size.x;
//                int pos  = (pTask->pos.x + x) + (pTask->pos.y + y) * w;
//                hdr[pos] = Vector4(rgba[p].x, rgba[p].y, rgba[p].z, rgba[p].w) * (1.0f / pTask->processed);
//            }
//        }
//
//        emit updated(mRect);
//    }
//}
//
//void Worker::processGPU(Vector4 *rgba) {
//    Vector4 *hdr  = pParent->hdr();
//
//    ASceneRT *scene = pParent->scene();
//
//    int w          = pParent->width();
//    int h          = pParent->height();
//    int spp        = pParent->spp();
//
//    char first     = pParent->firstBounce();
//    char second    = pParent->secondBounce();
//
//    ocl_task_data task;
//    task.pos       = pTask->pos;
//    task.size      = Vector2(w, h);
//    task.mode      = (first << 4) | second;
//    task.trisCount = scene->mTriangles.size();
//    task.bvhBegin  = scene->mIndex;
//
//    ACamera *camera = pParent->camera();
//
//    ocl_camera_data ocl_camera;
//    ocl_camera.eye = Vector4(camera->actor().position(), camera->fov());
//    //ocl_camera.pos  = Vector4(camera->actor().rotation(), camera->ratio()); // \todo return rotation
//    ocl_camera.up  = Vector4(Vector3(0.0f, 1.0f, 0.0f), camera->farPlane());
//
//    cl_kernel kernel;
//    cl_command_queue queue;
//    cl_mem tsk;
//    cl_mem cam;
//    cl_mem mtl;
//    cl_mem pol;
//    cl_mem bvh;
//    cl_mem dst;
//    vector<ocl_triangle_data> triangles;
//    vector<ocl_material_data> materials;
//
//    cl_context context = pParent->context(pTask->device);
//    cl_program program = pParent->program(pTask->device);
//
//    size_t szGlobalSize[2]  = {pTask->size.x, pTask->size.y};
//
//    cl_int error = CL_SUCCESS;
//    kernel  = clCreateKernel(program, "doWork", &error);
//    if(error != CL_SUCCESS) {
//        qDebug("Failed to create a kernel: %d", error);
//        return;
//    }
//
//    uint32_t count;
//    {
//        count = 1; // scene->mMaterials.size()
//        if(count > 0) {
//            materials.resize(count);
//        }
//
//        for(uint32_t i = 0; i < count; i++) {
//            materials[i].diffuse  = Vector4(1.0f, 1.0f, 1.0f, 0.0f); //Vector4(scene->mMaterials[i].diffuse, scene->mMaterials[i].reflection);
//            materials[i].emissive = Vector4(0.0f); //Vector4(scene->mMaterials[i].emissive, scene->mMaterials[i].refraction);
//        }
//    }
//    {
//        if(task.trisCount > 0) {
//            triangles.resize(task.trisCount);
//        }
//
//        for(uint32_t i = 0; i < task.trisCount; i++) {
//            TriangleData *t = &scene->mTriangles[i];
//
//            triangles[i].v0 = t->v[0].xyz;
//            triangles[i].v1 = t->v[1].xyz;
//            triangles[i].v2 = t->v[2].xyz;
//
//            triangles[i].n0 = Vector4(t->v[0].n, 0.0f);
//            triangles[i].n1 = Vector4(t->v[1].n, 0.0f);
//            triangles[i].n2 = Vector4(t->v[2].n, 0.0f);
//
//            triangles[i].u0 = t->v[0].uv;
//            triangles[i].u1 = t->v[1].uv;
//            triangles[i].u2 = t->v[2].uv;
//
//            triangles[i].material   = 0;
//            triangles[i].reserved   = 0;
//        }
//    }
//
//    tsk = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(ocl_task_data), nullptr, nullptr);
//    cam = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(ocl_camera_data), nullptr, nullptr);
//    mtl = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(ocl_material_data)  * materials.size(),  nullptr, nullptr);
//    pol = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(ocl_triangle_data)  * triangles.size(),  nullptr, nullptr);
//    bvh = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(bvh_node_data)  * scene->mBVH.size(),  nullptr, nullptr);
//    dst = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(Vector4) * szGlobalSize[0] * szGlobalSize[1], nullptr, nullptr);
//
//    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&tsk);
//    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&cam);
//    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&pol);
//    clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&bvh);
//    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&mtl);
//    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&dst);
//
//    queue = clCreateCommandQueue(context, pParent->device(pTask->device), 0, &error);
//    if(error != CL_SUCCESS) {
//        qDebug("Failed to create queue: %d", error);
//        return;
//    }
//
//    clEnqueueWriteBuffer(queue, mtl, CL_TRUE, 0, sizeof(ocl_material_data) * materials.size(), (materials.empty()) ? nullptr : &materials[0],  0, nullptr, nullptr);
//    clEnqueueWriteBuffer(queue, pol, CL_TRUE, 0, sizeof(ocl_triangle_data) * triangles.size(), (triangles.empty()) ? nullptr : &triangles[0],  0, nullptr, nullptr);
//    clEnqueueWriteBuffer(queue, bvh, CL_TRUE, 0, sizeof(bvh_node_data) * scene->mBVH.size(), &scene->mBVH[0], 0, nullptr, nullptr);
//
//    clEnqueueWriteBuffer(queue, cam, CL_TRUE, 0, sizeof(ocl_camera_data), &camera, 0, nullptr, nullptr);
//
//    for(int s = 1; s <= spp; s++) {
//        if(pTask->state == TASK_DROP) {
//            qDebug("Task drop.");
//
//            clReleaseMemObject(tsk);
//            clReleaseMemObject(cam);
//            clReleaseMemObject(pol);
//            clReleaseMemObject(bvh);
//            clReleaseMemObject(mtl);
//            //clReleaseMemObject(dst);
//
//            clReleaseCommandQueue(queue);
//            clReleaseKernel(kernel);
//
//            emit finished(pTask->device);
//            return;
//        }
//
//        task.seed = rand();
//
//        clEnqueueWriteBuffer(queue, tsk, CL_TRUE, 0, sizeof(ocl_task_data), &task, 0, nullptr, nullptr);
//
//        error = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, szGlobalSize, nullptr, 0, nullptr, nullptr);
//        if(error != CL_SUCCESS) {
//            qDebug("Failed to launch kernel: %d", error);
//            return;
//        }
//
//        error = clEnqueueReadBuffer(queue, dst, CL_TRUE, 0, sizeof(Vector4) * szGlobalSize[0] * szGlobalSize[1], rgba, 0, nullptr, nullptr);
//        if(error != CL_SUCCESS) {
//            qDebug("Failed to fetch result: %d", error);
//            return;
//        }
//
//        pTask->processed = s;
//
//        for(uint32_t y = 0; y < pTask->size.y; y++) {
//            for(uint32_t x = 0; x < pTask->size.x; x++) {
//                int p    = x + y * pTask->size.x;
//                int pos  = (pTask->pos.x + x) + (pTask->pos.y + y) * w;
//                hdr[pos] = Vector4(rgba[p].x, rgba[p].y, rgba[p].z, rgba[p].w) * (1.0 / pTask->processed);
//            }
//        }
//
//        emit updated(mRect);
//    }
//
//    clReleaseMemObject(tsk);
//    clReleaseMemObject(cam);
//    clReleaseMemObject(pol);
//    clReleaseMemObject(bvh);
//    clReleaseMemObject(mtl);
//    //clReleaseMemObject(dst);
//
//    clReleaseCommandQueue(queue);
//    clReleaseKernel(kernel);
//}
//
//Vector3 Worker::backTrace(int index, const Vector3 &pos, const Vector3 &n) {
//    Vector3 result    = Vector3(mAmbient);
//
//    ASceneRT *scene     = pParent->scene();
//    backTraceLights(scene->getChildren(), result, scene, index, pos, n);
//
//    return result;
//}
//
//void Worker::backTraceLights(AObject::objectsMap &components, Vector3 &result, ASceneRT *scene, const int index, const Vector3 &pos, const Vector3 &n) {
//    for(auto &it : components) {
//        ALightSourceRT *light   = dynamic_cast<ALightSourceRT *>(it.second);
//        if(light) {
//            light->backTrace(scene, pos, n, index, result);
//        } else {
//            backTraceLights(it.second->getChildren(), result, scene, index, pos, n);
//        }
//    }
//}
//
//Vector3 Worker::collectPhotons(int i, const Vector3 &p, const Vector3 &n) {
//    Vector3 energy;
//
//    ASceneRT *pScene   = pParent->scene();
//
//    mCollected.clear();
//    collectPhotonsTree(energy, p, n, mPhotonRadius, 1, &pScene->mPhotonTree);
//
//    if(mPhotonAuto && !mCollected.empty()) {
//        auto it = mCollected.begin();
//        while(it != mCollected.end()) {
//            float weight = MAX(0.0f, -n.dot((*it).dir));
//            weight      *= (1.0 - sqrt((*it).distance)) / mPhotonExposure;
//            energy      += (*it).rgb * weight;
//
//			++it;
//        }
//        //float distance  = mCollected.back().length;
//        //energy /= 4 * PI * (distance * distance) * mPhotonCount;
//    }
//
//    return energy;
//}
//
//void Worker::collectPhotonsTree(Vector3 &e, const Vector3 &p, const Vector3 &n, float l, int d, PhotonNodeData *node) {
//    if(node && node->aabb.intersect(p, l)) {
//        auto it = node->i.begin();
//        while(it != node->i.end()) {
//            Vector3 d = p - (*it).pos;
//            float distance = d.dot(d);
//
//            if(!mPhotonAuto) {
//                if(distance < l) {
//                    float weight = MAX(0.0f, -n.dot((*it).dir));
//                    weight      *= (1.0 - sqrt(distance)) / mPhotonExposure;
//                    e           += (*it).rgb * weight;
//                }
//            } else {
//                auto i = mCollected.begin();
//                while(i != mCollected.end()) {
//                    if(distance < (*i).distance)
//                        break;
//
//					++i;
//                }
//                PhotonData data = (*it);
//                data.distance   = distance;
//                mCollected.insert(i, data);
//
//                if(mCollected.size() > mPhotonCount) {
//                    mCollected.pop_back();
//                }
//            }
//
//			++it;
//        }
//
//        collectPhotonsTree(e, p, n, l, d + 1, node->left);
//        collectPhotonsTree(e, p, n, l, d + 1, node->right);
//    }
//}
//
//Vector3 Worker::calcRadiance(Ray &ray, bool uv) {
//    Vector3 res(1.0f);
//    list<StackData> stack;
//
//    ASceneRT *scene = pParent->scene();
//    int diffuse     = pParent->diffuse();
//
//    char first      = pParent->firstBounce();
//    char second     = pParent->secondBounce();
//
//    StackData data;
//    data.ray    = ray;
//    data.diff   = 0;
//    data.refl   = 0;
//    data.refr   = 0;
//    data.power  = Vector3(1.0);
//    stack.push_front(data);
//    while(!stack.empty()) {
//        Vector3 pos;
//        Vector3 n;
//        data    = stack.front();
//        stack.pop_front();
//
//        float minPower  = 0.001f;
//
//        if(data.power.x < minPower || data.power.y < minPower || data.power.z < minPower ||
//           data.diff >= diffuse || data.refl >= 10 || data.refr >= 10) {
//            res = Vector3();
//            continue;
//        }
//
//        int index   = rayTrace(scene, data.ray, &pos, &n, data.diff, uv);
//        if(index > -1) {
//            //AMaterial *mat = &scene->mMaterials[prim->material];
//
//            bool inside = false;
//            if(data.ray.dir.dot(n) > 0) {
//                n       = -n;
//                inside  = true;
//            }
//
//            Vector3 diff(1.0f);//  = mat->diffuse;
//            Vector3 emis;//  = mat->emissive;
//            float refl  = 0.0f;// mat->reflection;
//            float refr  = 0.0f;// mat->refraction;
//            float ior   = 2.8f;
//
//            float c     = 1.0;
//            if(refr > 0.0) {
//                c       = fresnel(glm::dot(n, data.ray.pos - pos), 1.0 / ior);
//            }
//
//            uint8_t mode  = (data.diff == 0) ? first : second;
//            switch(mode) {
//                case BACK_TRACING: {
//                    if(refr > 0.0f) {
//                        StackData d;
//                        d.ray   = data.ray.refract(n, pos, (inside) ? ior : 1.0, (inside) ? 1.0 : ior);
//                        d.diff  = data.diff;
//                        d.refl  = data.refl;
//                        d.refr  = data.refr + 1;
//                        d.power = data.power * refr * (1.0 - c);
//                        stack.push_front(d);
//                    }
//                    if(refl > 0.0f) {
//                        StackData d;
//                        d.ray   = data.ray.reflect(n, pos);
//                        d.diff  = data.diff;
//                        d.refl  = data.refl + 1;
//                        d.refr  = data.refr;
//                        d.power = data.power * refl * c;
//                        stack.push_front(d);
//                    }
//
//                    Vector3 f   = diff * (1.0f - MIX(refr, refl, c));
//                    Vector3 rgb = backTrace(index, pos, n) * f;
//
//                    if(data.refl > 0 || data.refr > 0) {
//                        res = res + rgb * data.power;
//                    } else {
//                        res = res * rgb * data.power;
//                    }
//                } break;
//
//                case PHOTON_MAPPING: {
//                    res = res * collectPhotons(index, pos, n);
//                } break;
//
//                case PATH_TRACING: {
//                    // Add next bounce to stack
//                    if(RANGE(0.0f, 1.0f) < refr * (1.0f - c)) { // REFRACT
//                        StackData d;
//                        d.ray       = data.ray.refract(n, pos, (inside) ? ior : 1.0, (inside) ? 1.0 : ior);
//                        d.diff      = data.diff;
//                        d.refl      = data.refl;
//                        d.refr      = data.refr + 1;
//                        d.power     = data.power * refr * (1.0 - c);
//                        stack.push_front(d);
//                    } else {
//                        if(RANGE(0.0f, 1.0f) < refl * c) { // REFLECT
//                            stack_data d;
//                            d.ray       = data.ray.reflect(n, pos);
//                            d.diff      = data.diff;
//                            d.refl      = data.refl + 1;
//                            d.refr      = data.refr;
//                            d.power     = data.power * refl * c;
//                            stack.push_front(d);
//                        } else { // DIFFUSE
//                            // Calc radiance
//                            res         = emis + res * diff;
//
//                            if(emis.x > 0.0f || emis.y > 0.0f || emis.z > 0.0f) {// We don't need to create a new bounce
//                                continue;
//                            }
//
//                            stack_data d;
//                            d.ray       = data.ray.diffuse(n, pos, 0.0f, 1.0f);
//                            d.diff      = data.diff + 1;
//                            d.refl      = data.refl;
//                            d.refr      = data.refr;
//                            d.power     = data.power;
//                            stack.push_front(d);
//                        }
//                    }
//                } break;
//
//                default: break;
//            }
//        } else {
//            res = Vector3(0.0f);
//        }
//    }
//
//    return res;
//}
//
//Vector3 Worker::triangleWeights(const Vector3 &point, const Vector3 &v1, const Vector3 &v2, const Vector3 &v3) {
//    Vector3 ve1(v2 - v1);
//    Vector3 ve2(v3 - v2);
//    Vector3 ve3(v1 - v3);
//
//    Vector3 p1(point - v1);
//    Vector3 p2(point - v2);
//    Vector3 p3(point - v3);
//
//    float s1 = p2.cross(ve2).length();
//    float s2 = p3.cross(ve3).length();
//    float s3 = p1.cross(ve1).length();
//
//    float sum = s1 + s2 + s3;
//
//    return Vector3(s1 / sum, s2 / sum, s3 / sum);
//}
//
//Vector3 Worker::texureColor(const Vector3 &point, Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector2 &t1, Vector2 &t2, Vector2 &t3) {
//    Vector3 d     = triangleWeights(point, v1, v2, v3);
//
//    Vector2 pos   = t1 * d.x + t2 * d.y + t3 * d.z;
///*
//    int x = (int)(xf * canvas->w) % (canvas->w + 1);
//    int y = (int)(yf * canvas->h) % (canvas->h + 1);
//*/
//    return Vector3(1.0);
//}
//
//int Worker::rayTrace(ASceneRT *scene, ARay &ray, Vector3 *p, Vector3 *n, int depth, bool uv) {
//    int32_t result  = -1;
//    float dist  = FLT_MAX; // Camera far plane
//
//    hash<string> idFunction;
//
//    if(!scene->mBVH.empty()) {
//        TriangleData *triangle = 0;
//        bvh_node_data *current  = &scene->mBVH[scene->mIndex];
//
//        while(current) {
//            AABox box;
//            box.set_box(Vector3(current->min), Vector3(current->max));
//            bool intersect  = ray.intersect(box, 0);
//            if(intersect) {
//                rayTraceNode(ray, current, scene, p, dist, &triangle, uv);
//            }
//
//            if(current->left > -1 && intersect) {
//                current = &scene->mBVH[current->left];
//            } else if(current->escape > -1) {
//                current = &scene->mBVH[current->escape];
//            } else {
//                break;
//            }
//        }
//
//        if(triangle) {
//            result  = idFunction(triangle->instance->actor().reference());
//            if(n) {
//                Vector3 v0(triangle->v[0].xyz);
//                Vector3 v1(triangle->v[1].xyz);
//                Vector3 v2(triangle->v[2].xyz);
//
//                Vector3 d = triangleWeights(*p, v0, v1, v2);
//               *n   = glm::normalize(triangle->v[0].n * d.x + triangle->v[1].n * d.y + triangle->v[2].n * d.z);
//            }
//        }
//    }
//    return result;
//}
//
//inline void Worker::rayTraceNode(ARay ray, bvh_node_data *node, ASceneRT *scene, Vector3 *p, float &dist, triangle_data **result, bool uv) {
//    Vector3 pos;
//    for(uint32_t i = node->offset; i < node->offset + node->count; i++) {
//        triangle_data *t = &scene->mTriangles[i]; /// \todo: UV Triangle list
//
//        Vector3 v0(t->v[0].xyz);
//        Vector3 v1(t->v[1].xyz);
//        Vector3 v2(t->v[2].xyz);
//
//        if(uv) {
//            Vector3 u0(t->v[0].uv, 1.0);
//            Vector3 u1(t->v[1].uv, 1.0);
//            Vector3 u2(t->v[2].uv, 1.0);
//
//            if(ray.intersect(u0, u1, u2, &pos, true)) {
//                Vector3 d = triangleWeights(pos, u0, u1, u2);
//
//                if(p) {
//                   *p   = v0 * d.x + v1 * d.y + v2 * d.z;
//                }
//                *result = t;
//            }
//        } else {
//            if(ray.intersect(v0, v1, v2, &pos)) {
//                Vector3 d = pos - ray.pos;
//                float l     = glm::dot(d, d);
//                if(l < dist) {
//                    dist    = l;
//                    if(p) {
//                       *p   = pos;
//                    }
//                    *result = t;
//                }
//            }
//        }
//    }
//}
