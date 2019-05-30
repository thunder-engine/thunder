/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2017 Evgeny Prikazchikov
*/

#ifndef AABB_H_HEADER_INCLUDED
#define AABB_H_HEADER_INCLUDED

#include <stdint.h>

#include "vector3.h"

class Matrix4;

class NEXT_LIBRARY_EXPORT AABBox {
public:
    AABBox                      ();
    AABBox                      (const Vector3 &center, const Vector3 &size);

    const AABBox                operator*                   (areal factor) const;
    const AABBox                operator*                   (const Vector3 &vector) const;
    const AABBox                operator*                   (const Matrix4 &matrix) const;

    void                        encapsulate                 (const Vector3 &point);
    void                        encapsulate                 (const AABBox &box);

    bool                        intersect                   (const Vector3 &position, areal radius) const;

    void                        box                         (Vector3 &min, Vector3 &max) const;
    void                        setBox                      (const Vector3 &min, const Vector3 &max);
    void                        setBox                      (const Vector3 *points, uint32_t number);

    Vector3                     center;
    Vector3                     size;
};

#endif /* AABB_H_HEADER_INCLUDED */
