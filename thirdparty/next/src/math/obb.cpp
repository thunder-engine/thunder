#include "math/amath.h"

/*!
    \class OBBox
    \brief The OBBox class represents a Oriented Bounding Box in 3D space.
    \since Next 1.0
    \inmodule Math

    An arbitrarily oriented bounded volume in space in the form of a rectangular parallelepiped (Bounding Box).
    Unlike AABB, OBB rotates with the object and does not change its size.
    Collision testing with OBB is somewhat more complicated and slower than AABB, but more often it is more preferable.
    Oriented Bounding Box represented by \p center of box, \p size and \p rotation quaternion.

    \sa Vector3, AABBox
*/
/*!
    Constructs an bounding box with \p center (0, 0, 0), \p size (1, 1, 1) and identity rotation.
*/
OBBox::OBBox() :
        center(0.0),
        size(1.0),
        rotation(Quaternion()) {
}
/*!
    Constructs a bounding box with \a center, \a size and identity rotation.
*/
OBBox::OBBox(const Vector3 &center, const Vector3 &size) :
        center(center),
        size(size),
        rotation(Quaternion()) {
}
/*!
    Constructs a bounding box with \a center, \a size and \a rotation.
*/
OBBox::OBBox(const Vector3 &center, const Vector3 &size, const Quaternion &rotation) :
        center(center),
        size(size),
        rotation(rotation) {
}
/*!
    Returns a copy of this vector, multiplied by the given \a factor.
*/
const OBBox OBBox::operator*(areal factor) {
    return OBBox(center * factor, size * factor, rotation);
}
/*!
    Returns a copy of this vector, multiplied by the given \a vector.
*/
const OBBox OBBox::operator*(const Vector3 &vector) {
    return OBBox(center * vector, size * vector, rotation);
}
/*!
    Returns \a min and \a max points of bounding box as output arguments.
*/
void OBBox::box(Vector3 &min, Vector3 &max) const {
    min     = center - size * 0.5;
    max     = min + size;
}
/*!
    Set curent bounding box by \a min and \a max points.
*/
void OBBox::setBox(const Vector3 &min, const Vector3 &max) {
    size    = max - min;
    center  = min + size * 0.5;
}
