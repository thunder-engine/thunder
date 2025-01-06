/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
*/

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
    Assignment operator.
    The \a value will be assigned to this object.
*/
AABBox &AABBox::operator=(const AABBox &value) {
    center = value.center;
    extent = value.extent;
    radius = value.radius;

    return *this;
}
/*!
    Returns true in case of AABBox is valid; otherwise returns false.
*/
bool AABBox::isValid() const {
    return radius > 0.0f;
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
    Returns a copy of this box, multiplied by the given rotation \a matrix.
*/
const AABBox AABBox::operator*(const Matrix3 &matrix) const {
    AABBox result;

    Vector3 min, max;
    box(min, max);

    Vector3 p[8]  = {
        matrix * Vector3(min.x, max.y, min.z),
        matrix * Vector3(min.x, max.y, max.z),
        matrix * Vector3(max.x, max.y, max.z),
        matrix * Vector3(max.x, max.y, min.z),

        matrix * Vector3(min.x, min.y, min.z),
        matrix * Vector3(min.x, min.y, max.z),
        matrix * Vector3(max.x, min.y, max.z),
        matrix * Vector3(max.x, min.y, min.z)
    };

    min = Vector3(MIN(p[0].x, MIN(p[1].x, MIN(p[2].x, MIN(p[4].x, MIN(p[5].x, MIN(p[6].x, p[7].x)))))),
                  MIN(p[0].y, MIN(p[1].y, MIN(p[2].y, MIN(p[4].y, MIN(p[5].y, MIN(p[6].y, p[7].y)))))),
                  MIN(p[0].z, MIN(p[1].z, MIN(p[2].z, MIN(p[4].z, MIN(p[5].z, MIN(p[6].z, p[7].z)))))));

    max = Vector3(MAX(p[0].x, MAX(p[1].x, MAX(p[2].x, MAX(p[4].x, MAX(p[5].x, MAX(p[6].x, p[7].x)))))),
                  MAX(p[0].y, MAX(p[1].y, MAX(p[2].y, MAX(p[4].y, MAX(p[5].y, MAX(p[6].y, p[7].y)))))),
                  MAX(p[0].z, MAX(p[1].z, MAX(p[2].z, MAX(p[4].z, MAX(p[5].z, MAX(p[6].z, p[7].z)))))));

    result.setBox(min, max);

    return result;
}
/*!
    Returns a copy of this box, multiplied by the given transform \a matrix.
*/
const AABBox AABBox::operator*(const Matrix4 &matrix) const {
    AABBox result = *this * matrix.rotation();

    result.center.x += matrix.mat[12];
    result.center.y += matrix.mat[13];
    result.center.z += matrix.mat[14];

    return result;
}
/*!
    Multiplies this box by the given rotation \a matrix, and returns a reference to this vector.
*/
AABBox &AABBox::operator*=(const Matrix3 &matrix) {
    return *this = *this * matrix;
}
/*!
    Multiplies this box by the given transform \a matrix, and returns a reference to this vector.
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
