#include "math/amath.h"

#include <float.h>
#include <vector>

/*!
    \class AABBox
    \brief The AABBox class represents a Axis Aligned Bounding Box in 3D space.
    \since Next 1.0
    \inmodule Math

    Bounded volume in space in the form of a rectangular parallelepiped, with a period parallel to the coordinate axes in the world system.
    When the object rotates, the AABB changes its dimensions, but it always remains oriented along the coordinate axes.
    Axis Aligned Bounding Box represented by \a center of box and \a extent.

    \sa Vector3, OBBox
*/
/*!
    Constructs an bounding box with center (0, 0, 0) and extent (0.5, 0.5, 0.5).
*/
AABBox::AABBox() :
        center(0.0f),
        extent(0.5f),
        radius(extent.length()) {
}
/*!
    Constructs a bounding box with \a center and \a extent.
*/
AABBox::AABBox(const Vector3 &center, const Vector3 &extent) :
        center(center),
        extent(extent),
        radius(extent.length()) {
}
/*!
    Returns true in case of AABBox is valid; otherwise returns false.
*/
bool AABBox::isValid() const {
    return (extent.x >= 0.0f) && (extent.y >= 0.0f) && (extent.z >= 0.0f);
}
/*!
    Grow the AABBox to encapsulate a spehere with \a position and \a radius.
*/
void AABBox::encapsulate(const Vector3 &position, areal radius) {
    Vector3 bb[2];
    box(bb[0], bb[1]);

    bb[0].x = MIN(bb[0].x, position.x - radius);
    bb[0].y = MIN(bb[0].y, position.y - radius);
    bb[0].z = MIN(bb[0].z, position.z - radius);

    bb[1].x = MAX(bb[1].x, position.x + radius);
    bb[1].y = MAX(bb[1].y, position.y + radius);
    bb[1].z = MAX(bb[1].z, position.z + radius);

    setBox(bb[0], bb[1]);
}
/*!
    Grow the AABBox to encapsulate the \a aabb.
*/
void AABBox::encapsulate(const AABBox &aabb) {
    if(!isValid()) {
        *this = aabb;
        return;
    }
    Vector3 bb0[2];
    box(bb0[0], bb0[1]);

    Vector3 bb1[2];
    aabb.box(bb1[0], bb1[1]);

    std::vector<Vector3> points = { Vector3(bb1[0].x, bb1[0].y, bb1[0].z),
                                    Vector3(bb1[1].x, bb1[0].y, bb1[0].z),
                                    Vector3(bb1[0].x, bb1[1].y, bb1[0].z),
                                    Vector3(bb1[0].x, bb1[0].y, bb1[1].z),
                                    Vector3(bb1[1].x, bb1[1].y, bb1[0].z),
                                    Vector3(bb1[0].x, bb1[1].y, bb1[1].z),
                                    Vector3(bb1[1].x, bb1[1].y, bb1[1].z),
                                    Vector3(bb1[1].x, bb1[0].y, bb1[1].z)};

    for(auto &it : points) {
        bb0[0].x = MIN(bb0[0].x, it.x);
        bb0[0].y = MIN(bb0[0].y, it.y);
        bb0[0].z = MIN(bb0[0].z, it.z);

        bb0[1].x = MAX(bb0[1].x, it.x);
        bb0[1].y = MAX(bb0[1].y, it.y);
        bb0[1].z = MAX(bb0[1].z, it.z);
    }

    setBox(bb0[0], bb0[1]);
}
/*!
    Returns true if this bounding box intersects the given sphere at \a position and \a radius; otherwise returns false.
*/
bool AABBox::intersect(const Vector3 &position, areal radius) const {
    Vector3 min, max;
    box(min, max);

    areal d = 0;

    for(int i = 0; i < 3; i++) {
        if (position[i] < min[i]) {
            areal s = position[i] - min[i];
            d += s * s;
        } else if (position[i] > max[i]) {
            areal s = position[i] - max[i];
            d += s * s;
        }
    }
    return d <= radius * radius;
}
/*!
    Returns true if this bounding box intersects the given \a count of \a planes; otherwise returns false.
*/
bool AABBox::intersect(const Plane *planes, areal count) const {
    for(int32_t i = 0; i < count; i++) {
        float d = planes[i].sqrDistance(center);
        float r = extent.dot(planes[i].normal.abs());
        if(d + r < 0.0f) {
            return false;
        }
    }
    return true;
}
/*!
    Returns true if this bounding box is equal to given bounding \a box; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool AABBox::operator==(const AABBox &box) const {
    return (center == box.center) && (extent == box.extent);
}
/*!
    Returns true if this bounding box is NOT equal to given bounding \a box; otherwise returns false.
    This operator uses an exact floating-point comparison.
*/
bool AABBox::operator!=(const AABBox &box) const {
    return (center != box.center) || (extent != box.extent);
}
/*!
    Returns a copy of this box, multiplied by the given \a factor.
*/
const AABBox AABBox::operator*(areal factor) const {
    return AABBox(center * factor, extent * factor);
}
/*!
    Returns a copy of this box, multiplied by the given \a vector.
*/
const AABBox AABBox::operator*(const Vector3 &vector) const {
    return AABBox(center * vector, extent * vector);
}
/*!
    Returns a copy of this box, multiplied by the given \a matrix.
*/
const AABBox AABBox::operator*(const Matrix4 &matrix) const {
    AABBox result;

    Vector3 min = -extent;
    Vector3 max = extent;

    Matrix3 rot = matrix.rotation();
    Vector3 rotPoints[4]  = {
        (rot * Vector3(min.x, max.y, min.z)).abs(),
        (rot * Vector3(min.x, max.y, max.z)).abs(),
        (rot * Vector3(max.x, max.y, max.z)).abs(),
        (rot * Vector3(max.x, max.y, min.z)).abs()
    };

    result.center = Vector3(matrix[12], matrix[13], matrix[14]) + center;
    result.extent = Vector3(MAX(rotPoints[0].x, MAX(rotPoints[1].x, MAX(rotPoints[2].x, rotPoints[3].x))),
                            MAX(rotPoints[0].y, MAX(rotPoints[1].y, MAX(rotPoints[2].y, rotPoints[3].y))),
                            MAX(rotPoints[0].z, MAX(rotPoints[1].z, MAX(rotPoints[2].z, rotPoints[3].z))));
    result.radius = result.extent.length();
    return result;
}
/*!
    Multiplies this box by the given \a matrix, and returns a reference to this vector.
*/
AABBox &AABBox::operator*=(const Matrix4 &matrix) {
    return *this = *this * matrix;
}
/*!
    Returns \a min and \a max points of bounding box as output arguments.
*/
void AABBox::box(Vector3 &min, Vector3 &max) const {
    min = center - extent;
    max = center + extent;
}
/*!
    Set current bounding box by \a min and \a max points.
*/
void AABBox::setBox(const Vector3 &min, const Vector3 &max) {
    extent = (max - min) * 0.5f;
    center = min + extent;
    radius = extent.length();
}
/*!
    Set curent bounding box by provided array of \a points and \a number of them.
*/
void AABBox::setBox(const Vector3 *points, uint32_t number) {
    Vector3 bb[2]   = {Vector3(FLT_MAX), Vector3(-FLT_MAX)};
    for(uint32_t i = 0; i < number; i++) {
        bb[0].x = MIN(bb[0].x, points[i].x);
        bb[0].y = MIN(bb[0].y, points[i].y);
        bb[0].z = MIN(bb[0].z, points[i].z);

        bb[1].x = MAX(bb[1].x, points[i].x);
        bb[1].y = MAX(bb[1].y, points[i].y);
        bb[1].z = MAX(bb[1].z, points[i].z);
    }
    setBox(bb[0], bb[1]);
}
