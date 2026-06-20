/*
    This file is part of Thunder Next.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "math/amath.h"

/*!
    \class OBBox
    \brief The OBBox class represents a Oriented Bounding Box in 3D space.
    \since Next 1.0
    \inmodule Math

    An arbitrarily oriented bounded volume in space in the form of a rectangular parallelepiped (Bounding Box).
    Unlike AABB, OBB rotates with the object and does not change its size.
    Collision testing with OBB is somewhat more complicated and slower than AABB, but more often it is more preferable.
    Oriented Bounding Box represented by \a center of box, \a size and \a rotation quaternion.

    \sa Vector3, AABBox
*/
/*!
    Constructs an bounding box with center (0, 0, 0), size (1, 1, 1) and identity rotation.
*/
OBBox::OBBox() :
        center(0.0),
        extent(1.0),
        rotation(Quaternion()),
        radius(extent.length()) {
}
/*!
    Constructs a bounding box with \a center, \a extent and identity rotation.
*/
OBBox::OBBox(const Vector3 &center, const Vector3 &extent) :
        center(center),
        extent(extent),
        rotation(Quaternion()),
        radius(extent.length()) {
}
/*!
    Constructs a bounding box with \a center, \a extent and \a rotation.
*/
OBBox::OBBox(const Vector3 &center, const Vector3 &extent, const Quaternion &rotation) :
        center(center),
        extent(extent),
        rotation(rotation),
        radius(extent.length()) {
}
/*!
    Assignment operator.
    The \a value will be assigned to this object.
*/
OBBox &OBBox::operator=(const OBBox &value) {
    center = value.center;
    extent = value.extent;
    rotation = value.rotation;

    return *this;
}
/*!
    Returns a copy of this vector, multiplied by the given \a factor.
*/
const OBBox OBBox::operator*(areal factor) const {
    return OBBox(center * factor, extent * factor, rotation);
}
/*!
    Returns a copy of this vector, multiplied by the given \a vector.
*/
const OBBox OBBox::operator*(const Vector3 &vector) const {
    return OBBox(center * vector, extent * vector, rotation);
}
/*!
    Returns \a min and \a max points of bounding box as output arguments.
*/
void OBBox::box(Vector3 &min, Vector3 &max) const {
    min = center - extent * 0.5;
    max = min + extent;
}
/*!
    Set curent bounding box by \a min and \a max points.
*/
void OBBox::setBox(const Vector3 &min, const Vector3 &max) {
    extent = max - min;
    center = min + extent * 0.5;
}
