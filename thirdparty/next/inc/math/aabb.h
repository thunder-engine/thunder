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

#ifndef AABB_H
#define AABB_H

#include <stdint.h>

#include "vector3.h"
#include "plane.h"

class Matrix3;
class Matrix4;
class Quaternion;

class NEXT_LIBRARY_EXPORT AABBox {
public:
    AABBox();
    AABBox(const Vector3 &center, const Vector3 &extent);
    AABBox(const AABBox&) = default;

    AABBox &operator=(const AABBox &value);

    bool isValid() const;

    bool operator==(const AABBox &box) const;
    bool operator!=(const AABBox &box) const;

    const AABBox operator*(areal factor) const;
    const AABBox operator*(const Vector3 &vector) const;
    const AABBox operator*(const Matrix3 &matrix) const;
    const AABBox operator*(const Matrix4 &matrix) const;
    const AABBox operator*(const Quaternion &quaternion) const;

    AABBox &operator*=(const Matrix3 &matrix);
    AABBox &operator*=(const Matrix4 &matrix);
    AABBox &operator*=(const Quaternion &quaternion);

    void encapsulate(const Vector3 &position, areal radius = 0.0f);
    void encapsulate(const AABBox &box);

    bool intersect(const Vector3 &position, areal radius) const;
    bool intersect(const Plane &plane) const;

    void box(Vector3 &min, Vector3 &max) const;
    void setBox(const Vector3 &min, const Vector3 &max);
    void setBox(const Vector3 *points, uint32_t number);

public:
    Vector3 center;
    Vector3 extent;
    float radius;

};

#endif // AABB_H
