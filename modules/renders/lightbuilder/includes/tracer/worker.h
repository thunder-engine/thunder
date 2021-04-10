#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "scheduler.h"

#include "renderrtsystem.h"

class Worker : public QObject {
    Q_OBJECT
public:
    Worker(Scheduler *parent, TaskData *task);

    static int rayTrace(RenderRTSystem *system, Ray &ray, Vector3 *p, Vector3 *n, int depth, bool uv = false);
    static void rayTraceNode(RenderRTSystem *system, Ray ray, BvhNodeData *node, Vector3 *p, float &dist, TriangleData **result, bool uv = false);

public slots:
    void doWork();

signals:
    void finished(int);
    void updated(QRect r);

private:
    inline void processCPU(Vector4 *rgba);

    // Base algorithm
    Vector3 backTrace(int index, const Vector3 &p, const Vector3 &n);
    void backTraceLights(Vector3 &result, const int index, const Vector3 &p, const Vector3 &n);

    // Photon maping algorithm
    Vector3 collectPhotons(int index, const Vector3 &p, const Vector3 &n);
    void collectPhotonsTree(Vector3 &e, const Vector3 &p, const Vector3 &n, float l, int d, PhotonNodeData *node);

    Vector3 calcRadiance(Ray &ray, bool uv);

    static Vector3 triangleWeights(const Vector3 &point, const Vector3 &v1, const Vector3 &v2, const Vector3 &v3);
    static Vector3 texureColor(const Vector3 &point, Vector3 &v1, Vector3 &v2, Vector3 &v3, Vector2 &t1, Vector2 &t2, Vector2 &t3);

private:
    TaskData *pTask;

    Scheduler *pParent;

    PhotonList mCollected;

    float mAmbient;

    bool mPhotonAuto;
    float mPhotonRadius;
    int mPhotonCount;
    float mPhotonExposure;

    QRect mRect;
};

#endif // WORKER_H
