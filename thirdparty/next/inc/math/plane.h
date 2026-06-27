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

#ifndef PLANE_H
#define PLANE_H

#include "vector3.h"

class NEXT_LIBRARY_EXPORT Plane {
public:
    Plane();
    Plane(const Vector3 &pos, const Vector3 &norm);
    Plane(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3);
    Plane(const Plane&) = default;

    Plane &operator=(const Plane &value);

public:
    Vector3 normal;
    Vector3 point;
    float d;

};

#endif // PLANE_H
