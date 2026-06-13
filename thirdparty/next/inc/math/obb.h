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

#ifndef OBB_H
#define OBB_H

#include "vector3.h"
#include "quaternion.h"

class Matrix4;

class NEXT_LIBRARY_EXPORT OBBox {
public:
    OBBox();
    OBBox(const Vector3 &center, const Vector3 &extent);
    OBBox(const Vector3 &center, const Vector3 &extent, const Quaternion &rotation);

    OBBox &operator=(const OBBox &value);

    const OBBox operator*(areal factor) const;
    const OBBox operator*(const Vector3 &vector) const;

    void box(Vector3 &min, Vector3 &max) const;
    void setBox(const Vector3 &min, const Vector3 &max);

public:
    Vector3 center;
    Vector3 extent;
    Quaternion rotation;

    float radius;

};

#endif // OBB_H
