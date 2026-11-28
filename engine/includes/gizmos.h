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

#ifndef GIZMOS_H
#define GIZMOS_H

#include <amath.h>

#include <engine.h>

class Mesh;
class Sprite;
class Material;
class MaterialInstance;

class CommandBuffer;

class ENGINE_EXPORT Gizmos {
public:
    static void drawIcon(const Vector3 &center, const Vector2 &size, const TString &name, const Vector4 &color);

    static void drawSolidSector(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawSolidBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawSolidMesh(Mesh &mesh, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawSolidSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawSolidTriangles(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, const Matrix4 *transform = nullptr);

    static void drawLines(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireArc(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireCircle(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireRectangle(const Vector3 &center, const Vector2 &size, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireMesh(Mesh &mesh, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireCylinder(const Vector3 &center, float radius, float height, const Vector4 &color, const Matrix4 *transform = nullptr);
    static void drawWireCapsule(const Vector3 &center, float radius, float height, const Vector4 &color, const Matrix4 *transform = nullptr);

private:
    friend class GizmoRender;

    static void init();

    static void clear();
    static void drawSpriteBatch(CommandBuffer *buffer);
    static void drawSolidBatch(CommandBuffer *buffer);
    static void drawWireBatch(CommandBuffer *buffer);

    static void batchMesh(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, Mesh *mesh, const Matrix4 *transform = nullptr);

protected:
    struct SpriteBatches {
        Mesh *mesh;
        MaterialInstance *material;
    };
    static std::unordered_map<TString, SpriteBatches> s_sprites;

    static Matrix4 s_view;
    static Matrix4 s_projection;

    static Mesh *s_wire;
    static Mesh *s_solid;

    static Material *s_spriteMaterial;

    static MaterialInstance *s_gizmoMaterial;

};

#endif // GIZMOS_H
