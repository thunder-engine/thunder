#ifndef WORKER_H
#define WORKER_H

#include <QObject>
/*
#include "scheduler.h"

/// \note: OpenCL align float3 to float4

struct ocl_material_data {
    Vector4           diffuse;
    Vector4           emissive;
    //float           reflection;
    //float           refraction;
};

struct ocl_task_data {
    Vector2           pos;
    Vector2           size;

    int               seed;
    int               mode;
    int               bvhBegin;

    uint32_t          trisCount;
};

struct ocl_camera_data {
    Vector4           eye;
    Vector4           pos;
    Vector4           reserved;
    Vector4           up;
};

struct ocl_triangle_data {
    Vector4           v0;
    Vector4           v1;
    Vector4           v2;

    Vector4           n0;
    Vector4           n1;
    Vector4           n2;

    Vector2           u0;
    Vector2           u1;
    Vector2           u2;

    uint32_t          material;

    uint32_t          reserved;
};

class Worker : public QObject {
    Q_OBJECT
public:
    Worker                  (Scheduler *parent, TaskData *task);

    static int              rayTrace                        (ASceneRT *pScene, ARay &ray, Vector3 *p, Vector3 *n, int depth, bool uv = false);
    static inline void      rayTraceNode                    (Ray ray, bvh_node_data *node, ASceneRT *scene, Vector3 *p, float &dist, triangle_data **result, bool uv = false);

public slots:
    void                    doWork                          ();

signals:
    void                    finished                        (int);
    void                    updated                         (QRect r);

    cl_device_id &device    (uint32_t id) { return mDevices[id]; }
    cl_context   &context   (uint32_t id) { return mContexts[id]; }
    cl_program   &program   (uint32_t id) { return mPrograms[id]; }

private:
    inline void             processCPU                      (Vector4 *rgba);

    inline void             processGPU                      (Vector4 *rgba);

    // Base algorithm
    Vector3                 backTrace                       (int index, const Vector3 &pos, const Vector3 &n);
    void                    backTraceLights                 (AObject::objectsMap &components, Vector3 &result, ASceneRT *scene, const int index, const Vector3 &pos, const Vector3 &n);

    // Photon maping algorithm
    Vector3                 collectPhotons                  (int i, const Vector3 &p, const Vector3 &n);
    void                    collectPhotonsTree              (Vector3 &e, const Vector3 &p, const Vector3 &n, float l, int d, PhotonNodeData *node);

    Vector3                 calcRadiance                    (Ray &ray, bool uv);

    static Vector3          triangleWeights                 (const Vector3 &point, const Vector3 &v1, const Vector3 &v2, const Vector3 &v3);
    static Vector3          texureColor                     (const Vector3 &point, Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector2 &t1, Vector2 &t2, Vector2 &t3);

    TaskData               *pTask;

    Scheduler              *pParent;

    photon_list             mCollected;

    float                   mAmbient;

    bool                    mPhotonAuto;
    float                   mPhotonRadius;
    int                     mPhotonCount;
    float                   mPhotonExposure;

    QRect                   mRect;

    DeviceVector  mDevices;
    ContextVector mContexts;
    ProgramVector mPrograms;
};
*/
#endif // WORKER_H
