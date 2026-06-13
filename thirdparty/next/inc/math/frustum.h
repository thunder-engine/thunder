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

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "plane.h"

class AABBox;
class OBBox;

class NEXT_LIBRARY_EXPORT Frustum {
public:
    Frustum();

    bool contains(const AABBox &bb) const;
    bool contains(const OBBox &bb) const;

    bool isOnOrForwardPlane(const Plane &plane, const AABBox &bb) const;
    bool isOnOrForwardPlane(const Plane &plane, const OBBox &bb) const;

public:
    Plane m_top;
    Plane m_bottom;
    Plane m_left;
    Plane m_right;
    Plane m_near;
    Plane m_far;

};

#endif // FRUSTUM_H
