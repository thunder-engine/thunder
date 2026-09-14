/*
    This file is part of Thunder Engine.

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
#ifndef HANDLETOOLS_H
#define HANDLETOOLS_H

#include <amath.h>

#include <editor.h>

class Camera;

class EDITOR_EXPORT HandleTools {
public:
    static float distanceToPoint(const Matrix4 &matrix, const Vector3 &point, const Vector2 &screen, bool world = true);

    static float distanceToPath(const Matrix4 &matrix, const Vector3Vector &points, const Vector2 &screen, bool world = true);

    static float distanceToMesh(const Matrix4 &matrix, const IndexVector &indices, const Vector3Vector &vertices, const Vector2 &screen, bool world = true);

    static bool pointInRect(const Matrix4 &matrix, const Vector3 &tl, const Vector3 &br, const Vector2 &screen, bool world = true);
};

#endif // HANDLETOOLS_H
