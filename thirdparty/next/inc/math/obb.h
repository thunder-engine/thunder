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

#ifndef OBB_H
#define OBB_H

#include "vector3.h"
#include "quaternion.h"

class Matrix4;

class NEXT_LIBRARY_EXPORT OBBox {
public:
    OBBox();
    OBBox(const Vector3 &center, const Vector3 &size);
    OBBox(const Vector3 &center, const Vector3 &size, const Quaternion &rotation);

    OBBox &operator=(const OBBox &value);

    const OBBox operator*(areal factor) const;
    const OBBox operator*(const Vector3 &vector) const;

    void box(Vector3 &min, Vector3 &max) const;
    void setBox(const Vector3 &min, const Vector3 &max);

public:
    Vector3 center;
    Vector3 size;
    Quaternion rotation;

};

#endif // OBB_H
