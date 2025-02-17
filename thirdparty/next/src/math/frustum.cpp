#include "math/frustum.h"

#include "math/aabb.h"

Frustum::Frustum() {

}

bool Frustum::contains(const AABBox &bb) const {
    return isOnOrForwardPlane(m_top, bb) &&
           isOnOrForwardPlane(m_bottom, bb) &&
           isOnOrForwardPlane(m_left, bb) &&
           isOnOrForwardPlane(m_right, bb) &&
           isOnOrForwardPlane(m_near, bb) &&
           isOnOrForwardPlane(m_far, bb);
}

bool Frustum::isOnOrForwardPlane(const Plane &plane, const AABBox &bb) const {
    float d = plane.normal.dot(bb.center) - plane.d;

    if(d < -bb.radius) {
        return false;
    }
    return true;
}

