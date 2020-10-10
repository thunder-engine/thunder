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

#ifndef RAY_H_HEADER_INCLUDED
#define RAY_H_HEADER_INCLUDED

#include "vector3.h"

class Plane;
class AABBox;

class NEXT_LIBRARY_EXPORT Ray {
public:
    Ray                         ();
    Ray                         (const Vector3 &position, const Vector3 &direction);

    bool                        operator==                  (const Ray &ray) const;
    bool                        operator!=                  (const Ray &ray) const;

    bool                        intersect                   (const Vector3 &position, areal radius, Vector3 *pt);
    bool                        intersect                   (const Plane &plane, Vector3 *pt, bool back = false);
    bool                        intersect                   (const AABBox &box, Vector3 *pt);
    bool                        intersect                   (const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, Vector3 *pt, bool back = false);

    Ray                         reflect                     (const Vector3 &normal, const Vector3 &point);
    Ray                         refract                     (const Vector3 &normal, const Vector3 &point, areal ior);
    Ray                         diffuse                     (const Vector3 &normal, const Vector3 &point, areal min, areal max);

    Vector3                     pos;
    Vector3                     dir;
};

#endif // RAY_H_HEADER_INCLUDED
