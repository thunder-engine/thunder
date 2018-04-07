#include "math/amath.h"
/*!
    \class AABBox
    \brief The AABBox class represents a Axis Aligned Bounding Box in 3D space.
    \since Next 1.0
    \inmodule Math

    Bounded volume in space in the form of a rectangular parallelepiped, with a period parallel to the coordinate axes in the world system.
    When the object rotates, the AABB changes its dimensions, but it always remains oriented along the coordinate axes.
    Axis Aligned Bounding Box represented by \a center of box and \a size.

    \sa Vector3, OBBox
*/
/*!
    Constructs an bounding box with center (0, 0, 0) and size (1, 1, 1).
*/
AABBox::AABBox() :
        center(0.0),
        size(1.0) {
}
/*!
    Constructs a bounding box with \a center and \a size.
*/
AABBox::AABBox(const Vector3 &center, const Vector3 &size) :
        center(center),
        size(size) {
}
/*!
    Returns true if this bounding box intersects the given sphere at \a position and \a radius; otherwise returns false.
*/
bool AABBox::intersect(const Vector3 &position, areal radius) const {
    Vector3 min, max;
    box(min, max);

    areal d = 0;
    areal s = 0;

    for(int i = 0; i < 3; i++) {
        if (position[i] < min[i]) {
            s   = position[i] - min[i];
            d  += s * s;
        } else if (position[i] > max[i]) {
            s   = position[i] - max[i];
            d  += s * s;
        }
    }
    return d <= radius * radius;
}
/*!
    Returns a copy of this vector, multiplied by the given \a factor.
*/
const AABBox AABBox::operator*(areal factor) const {
    return AABBox(center * factor, size * factor);
}
/*!
    Returns a copy of this vector, multiplied by the given \a vector.
*/
const AABBox AABBox::operator*(const Vector3 &vector) const {
    return AABBox(center * vector, size * vector);
}
/*!
    Returns a copy of this vector, multiplied by the given \a matrix.
*/
const AABBox AABBox::operator*(const Matrix4 &matrix) const {
    return AABBox(center * Vector3(matrix[0], matrix[5], matrix[10]) + Vector3(matrix[12], matrix[13], matrix[14]),
                 size * Vector3(matrix[0], matrix[5], matrix[10]));
}
/*!
    Returns \a min and \a max points of bounding box as output arguments.
*/
void AABBox::box(Vector3 &min, Vector3 &max) const {
    min     = center - size * 0.5;
    max     = min + size;
}
/*!
    Set curent bounding box by \a min and \a max points.
*/
void AABBox::setBox(const Vector3 &min, const Vector3 &max) {
    size    = max - min;
    center  = min + size * 0.5;
}
