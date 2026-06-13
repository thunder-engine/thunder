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

#ifndef RAY_H
#define RAY_H

#include "vector3.h"

class Plane;
class AABBox;
class Object;

class NEXT_LIBRARY_EXPORT Ray {
public:
    struct NEXT_LIBRARY_EXPORT Hit {
        Vector3 normal;
        Vector3 point;

        Object *object = nullptr;

        float distance = 0.0f;
    };

public:
    Ray();
    Ray(const Vector3 &position, const Vector3 &direction);

    Ray &operator=(const Ray &value);

    bool operator==(const Ray &ray) const;
    bool operator!=(const Ray &ray) const;

    bool intersect(const Vector3 &position, areal radius, Hit *hit);
    bool intersect(const Plane &plane, Hit *hit, bool back = false);
    bool intersect(const AABBox &box, Hit *hit);
    bool intersect(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, Hit *hit, bool back = false);

    Ray reflect(const Vector3 &normal, const Vector3 &point);
    Ray refract(const Vector3 &normal, const Vector3 &point, areal ior);
    Ray diffuse(const Vector3 &normal, const Vector3 &point, areal min, areal max);

public:
    Vector3 pos;
    Vector3 dir;

};

#endif // RAY_H
