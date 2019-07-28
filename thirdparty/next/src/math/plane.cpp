#include "math/amath.h"

/*!
    \class Plane
    \brief The Plane class represents a plane in 3D space.
    \since Next 1.0
    \inmodule Math

    A Plane is a flat, 2D surface that extends infinitely far in 3D space.

    \sa Vector3
*/

/*!
    Default constructor.
*/
Plane::Plane() :
    d(1.0f) {

}
/*!
    Cunstructs a Plane by three points \a v1, \a v2 and \a v3
*/
Plane::Plane(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3)  {
    Vector3 aux1, aux2;
    aux1    = v2 - v1;
    aux2    = v3 - v1;
    normal  = aux1.cross(aux2);
    //normal.normalize();
    point   = v1;
    d       = normal.dot(point);
}
/*!
    Calculate a squared distance between \a point and this Plane
*/
areal Plane::sqrDistance(const Vector3 &point) const {
    return normal.dot(point) - d;
}
