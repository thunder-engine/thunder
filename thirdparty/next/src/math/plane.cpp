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
    Assignment operator.
    The \a value will be assigned to this object.
*/
Plane &Plane::operator=(const Plane &value) {
    d = value.d;
    normal = value.normal;
    point = value.point;

    return *this;
}

Plane::Plane(const Vector3 &pos, const Vector3 &norm) {
    normal = norm;
    normal.normalize();
    point = pos;
    d = normal.dot(point);
}
/*!
    Cunstructs a Plane by three points \a v1, \a v2 and \a v3
*/
Plane::Plane(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3)  {
    Vector3 aux1, aux2;
    aux1 = v2 - v1;
    aux2 = v3 - v1;
    normal = aux1.cross(aux2);
    normal.normalize();
    point = v1;
    d = normal.dot(point);
}
