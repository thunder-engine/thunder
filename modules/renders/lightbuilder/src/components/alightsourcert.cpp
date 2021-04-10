#include "components/alightsourcert.h"

#include <components/actor.h>

#include "tracer/worker.h"

void ALightSourceRT::setRotation(const Quaternion &value) {
    m_Dir = value * Vector3(1.0f, 0.0f, 0.0f);
    m_Dir.normalize();
}

void ALightSourceRT::backTrace(Vector3 &pos, const Vector3 &n, const int index, Vector3 &result) {
    //setRotation(AQuaternion::euler(45.0f, 0.0f, 0.0f));

    //Ray ray;
    //switch(m_Type) {
    //    case ALightSource::DIRECT: {
    //        ray.dir = m_Dir;
    //        ray.pos = -ray.dir * 128.0f;
    //    } break;
    //    case ALightSource::POINT: {
    //        ray.pos = actor().position();
    //        ray.dir = pos - ray.pos;
    //        ray.dir.normalize();
    //    } break;
    //    case ALightSource::SPOT: {
    //    } break;
    //    default: break;
    //}
    //
    //if(!m_Shadows || Worker::rayTrace(scene, ray, 0, 0, 0) == index) {
    //    result  = MIN(Vector3(1.0f), MAX(Vector3(1.0) * m_Brightness * n.dot(ray.dir), result));
    //}
}
