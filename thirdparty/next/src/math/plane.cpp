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

    Copyright: 2008-2025 Evgeniy Prikazchikov
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
    d = normal.dot(pos);
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
