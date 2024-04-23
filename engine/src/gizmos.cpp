#include "gizmos.h"

#include "components/camera.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "commandbuffer.h"

#define OVERRIDE "texture0"

Mesh *Gizmos::s_wire = nullptr;
Mesh *Gizmos::s_solid = nullptr;

MaterialInstance *Gizmos::s_wireMaterial = nullptr;
MaterialInstance *Gizmos::s_solidMaterial = nullptr;

Material *Gizmos::s_spriteMaterial = nullptr;

struct SpriteBatches {
    Mesh *mesh;
    MaterialInstance *material;
};
static unordered_map<string, SpriteBatches> s_sprites;

/*!
    \class Gizmos
    \brief The Gizmos class provides functions to draw various graphical primitives for debugging purposes in a 3D space.
    \inmodule Engine

    \note Gizmos can be drawn only in Editor.

    The Gizmos class provides a collection of static methods to draw various shapes and primitives for debugging in a 3D space.
    Users can use these methods to visualize different elements during development and debugging.
*/

/*!
    \internal
    Initializes static member variables by loading necessary resources.
*/
void Gizmos::init() {
    if(s_wireMaterial == nullptr) {
        Material *m = Engine::loadResource<Material>(".embedded/gizmo.shader");
        if(m) {
            s_wireMaterial = m->createInstance();
        }
    }
    if(s_solidMaterial == nullptr) {
        Material *m = Engine::loadResource<Material>(".embedded/solid.shader");
        if(m) {
            s_solidMaterial = m->createInstance();
        }
    }
    if(s_spriteMaterial == nullptr) {
        s_spriteMaterial = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
    }

    if(s_wire == nullptr) {
        s_wire = Engine::objectCreate<Mesh>("Gizmo Lines Batch");
        s_wire->makeDynamic();
    }
    if(s_solid == nullptr) {
        s_solid = Engine::objectCreate<Mesh>("Gizmo Solid Batch");
        s_solid->makeDynamic();
    }
}
/*!
    Clears the content of wireframe, solid, and sprite batches.
*/
void Gizmos::clear() {
    s_wire->clear();
    s_solid->clear();

    for(auto &it : s_sprites) {
        it.second.mesh->clear();
    }
}
/*!
    \internal
    Draws the sprite batch using the provided command buffer.
*/
void Gizmos::drawSpriteBatch(CommandBuffer *buffer) {
    for(auto &it : s_sprites) {
        if(!it.second.mesh->isEmpty()) {
            buffer->drawMesh(Matrix4(), it.second.mesh, 0, CommandBuffer::TRANSLUCENT, it.second.material);
        }
    }
}
/*!
    \internal
    Draws the solid batch using the provided command buffer.
*/
void Gizmos::drawSolidBatch(CommandBuffer *buffer) {
    if(!s_solid->isEmpty()) {
        buffer->drawMesh(Matrix4(), s_solid, 0, CommandBuffer::TRANSLUCENT, s_solidMaterial);
    }
}
/*!
    \internal
    Draws the wireframe batch using the provided command buffer.
*/
void Gizmos::drawWireBatch(CommandBuffer *buffer) {
    if(!s_wire->isEmpty()) {
        buffer->drawMesh(Matrix4(), s_wire, 0, CommandBuffer::TRANSLUCENT, s_wireMaterial);
    }
}
/*!
    Draws a solid box with specified \a center, \a size and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this box.
*/
void Gizmos::drawBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 &transform) {
    Vector3 min(center - size * 0.5f);
    Vector3 max(center + size * 0.5f);

    static Mesh mesh;
    mesh.setVertices({
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, min.y, max.z),
        Vector3(min.x, min.y, max.z),

        Vector3(min.x, max.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)
    });
    mesh.setIndices({0, 1, 2, 0, 2, 3, // bottom
                     4, 6, 5, 4, 7, 6, // top
                     0, 1, 5, 0, 5, 4, // front
                     3, 6, 2, 3, 7, 6, // back
                     0, 7, 3, 0, 4, 7, // left
                     1, 6, 2, 1, 5, 6, // right
                    });
    mesh.setColors(Vector4Vector(mesh.vertices().size(), color));

    s_solid->batchMesh(mesh, &transform);
}
/*!
    Draws an billboard icon at the specified \a center with the given \a size, \a color, and \a transform.
    Parameter \a name will be used to set a texture to render.
*/
void Gizmos::drawIcon(const Vector3 &center, const Vector2 &size, const string &name, const Vector4 &color, const Matrix4 &transform) {
    Matrix4 model(center, Quaternion(), Vector3(size, size.x));
    Matrix4 q = model * Matrix4(Camera::current()->transform()->quaternion().toMatrix());

    static Mesh mesh;
    mesh.setIndices({0, 1, 2, 0, 2, 3});
    mesh.setVertices({Vector3(-0.5f,-0.5f, 0.0f),
                      Vector3(-0.5f, 0.5f, 0.0f),
                      Vector3( 0.5f, 0.5f, 0.0f),
                      Vector3( 0.5f,-0.5f, 0.0f)});
    mesh.setUv0({Vector2(0.0f, 0.0f),
                 Vector2(0.0f, 1.0f),
                 Vector2(1.0f, 1.0f),
                 Vector2(1.0f, 0.0f)});
    mesh.setColors(Vector4Vector(4, color));

    auto it = s_sprites.find(name);
    if(it != s_sprites.end()) {
        it->second.mesh->batchMesh(mesh, &q);
    } else {
        if(s_spriteMaterial) {
            SpriteBatches batch;
            batch.mesh = Engine::objectCreate<Mesh>(name);
            batch.mesh->makeDynamic();
            batch.mesh->batchMesh(mesh, &q);
            batch.material = s_spriteMaterial->createInstance();
            batch.material->setTexture(OVERRIDE, Engine::loadResource<Texture>(name));
            s_sprites[name] = batch;
        }
    }
}
/*!
    Draws a \a mesh with a specified \a color and \a transform.
*/
void Gizmos::drawMesh(Mesh &mesh, const Vector4 &color, const Matrix4 &transform) {
    static Mesh m;
    m.setVertices(mesh.vertices());
    m.setIndices(mesh.indices());
    m.setNormals(mesh.normals());
    m.setTangents(mesh.tangents());
    m.setUv0(mesh.uv0());
    m.setColors(Vector4Vector(m.vertices().size(), color));

    s_solid->batchMesh(m, &transform);
}
/*!
    Draws a solid sphere with specified \a center, \a radius and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this sphere.
*/
void Gizmos::drawSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform) {
    const uint32_t steps = 12;

    float stackStep = PI / (float)steps;
    float sectorStep = 2.0f * stackStep;

    Vector3Vector vertices;
    vertices.reserve((steps + 1) * (steps + 1));

    IndexVector indices;

    for(uint32_t i = 0; i <= steps; i++) {
        float stackAngle = PI / 2 - i * stackStep;
        float xz = radius * cosf(stackAngle);
        float y = radius * sinf(stackAngle);

        for(uint32_t j = 0; j <= steps; j++) {
            float sectorAngle = j * sectorStep;

            float x = xz * cosf(sectorAngle);
            float z = xz * sinf(sectorAngle);

            vertices.push_back(center + Vector3(x, y, z));
        }
    }

    for(uint32_t i = 0; i < steps; i++) {
        uint32_t k1 = i * (steps + 1);
        uint32_t k2 = k1 + steps + 1;

        for(uint32_t j = 0; j < steps; j++, k1++, k2++) {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }

    static Mesh mesh;
    mesh.setVertices(vertices);
    mesh.setIndices(indices);
    mesh.setColors(Vector4Vector(mesh.vertices().size(), color));

    s_solid->batchMesh(mesh, &transform);
}
/*!
    Draws a solid arc in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameters \a start and \a angle allows to specify angles to draw a sector in degrees.
    Parameter \a transform can be used to move, rotate and scale this arc.
*/
void Gizmos::drawSolidArc(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 &transform) {
    static Mesh mesh;
    mesh.setVertices(Mathf::pointsArc(Quaternion(), radius, start, angle, 180, true));
    size_t size = mesh.vertices().size();

    IndexVector indices;
    indices.resize((size - 1) * 3);
    for(int i = 0; i < size - 1; i++) {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i;
        indices[i * 3 + 2] = i+1;
    }
    mesh.setIndices(indices);
    mesh.setColors(Vector4Vector(size, color));

    Matrix4 t(transform);
    t[12] += center.x;
    t[13] += center.y;
    t[14] += center.z;

    s_solid->batchMesh(mesh, &t);
}
/*!
    Draws lines connecting specified \a points and \a color in 3D space.
    Parameter \a indices specifies relations between points.
    Parameter \a transform can be used to move, rotate and scale this structure.
*/
void Gizmos::drawLines(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, const Matrix4 &transform) {
    static Mesh mesh;
    mesh.setVertices(points);
    mesh.setIndices(indices);
    mesh.setColors(Vector4Vector(points.size(), color));

    s_wire->batchMesh(mesh, &transform);
}
/*!
    Draws a wire arc in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameters \a start and \a angle allows to specify angles to draw a sector in degrees.
    Parameter \a transform can be used to move, rotate and scale this arc.
*/
void Gizmos::drawArc(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 &transform) {
    static Mesh mesh;
    mesh.setVertices(Mathf::pointsArc(Quaternion(), radius, start, angle, 180));
    size_t size = mesh.vertices().size();

    IndexVector indices;
    indices.resize((size - 1) * 2);
    for(int i = 0; i < size - 1; i++) {
        indices[i * 2] = i;
        indices[i * 2 + 1] = i+1;
    }
    mesh.setIndices(indices);
    mesh.setColors(Vector4Vector(size, color));

    Matrix4 t(transform);
    t[12] += center.x;
    t[13] += center.y;
    t[14] += center.z;

    s_wire->batchMesh(mesh, &t);
}
/*!
    Draws a wire circle in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this circle.
*/
void Gizmos::drawCircle(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform) {
    drawArc(center, radius, 0, 360, color, transform);
}
/*!
    Draws a wire rectangle in the 3D space with the specified \a center, \a size and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this rectangle.
*/
void Gizmos::drawRectangle(const Vector3 &center, const Vector2 &size, const Vector4 &color, const Matrix4 &transform) {
    Vector2 min(Vector2(center.x, center.y) - size * 0.5f);
    Vector2 max(Vector2(center.x, center.y) + size * 0.5f);

    static Mesh mesh;
    mesh.setVertices({
        Vector3(min.x, min.y, center.z),
        Vector3(max.x, min.y, center.z),
        Vector3(max.x, max.y, center.z),
        Vector3(min.x, max.y, center.z)
    });
    mesh.setIndices({
        0, 1, 1, 2, 2, 3, 3, 0
    });
    mesh.setColors(Vector4Vector(mesh.vertices().size(), color));

    s_wire->batchMesh(mesh, &transform);
}
/*!
    Draws a wire box in the 3D space with the specified \a center, \a size and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this box.
*/
void Gizmos::drawWireBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 &transform) {
    Vector3 min(center - size * 0.5f);
    Vector3 max(center + size * 0.5f);

    static Mesh mesh;
    mesh.setVertices({
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, min.y, max.z),
        Vector3(min.x, min.y, max.z),

        Vector3(min.x, max.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)
    });
    mesh.setIndices({0, 1, 1, 2, 2, 3, 3, 0,
                     4, 5, 5, 6, 6, 7, 7, 4,
                     0, 4, 1, 5, 2, 6, 3, 7});
    mesh.setColors(Vector4Vector(mesh.vertices().size(), color));

    s_wire->batchMesh(mesh, &transform);
}
/*!
    Draws a wireframe version of the specified \a mesh and \a color in 3D space.
    Parameter \a transform can be used to move, rotate and scale this mesh.
*/
void Gizmos::drawWireMesh(Mesh &mesh, const Vector4 &color, const Matrix4 &transform) {
    static Mesh m;
    m.setVertices(mesh.vertices());
    m.setIndices(mesh.indices());
    m.setColors(Vector4Vector(m.vertices().size(), color));

    s_wire->batchMesh(m, &transform);
}
/*!
    Draws a wire sphere in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this sphere.
*/
void Gizmos::drawWireSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform) {
    drawCircle(center, radius, color, transform);
    Matrix4 t = transform * Matrix4(Quaternion(Vector3(1, 0, 0), 90).toMatrix());
    drawCircle(center, radius, color, t);
    t = transform * Matrix4(Quaternion(Vector3(0, 0, 1), 90).toMatrix());
    drawCircle(center, radius, color, t);
}
/*!
    Draws a wire capsule in the 3D space with the specified \a center, \a radius, \a height and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this capsule.
*/
void Gizmos::drawWireCapsule(const Vector3 &center, float radius, float height, const Vector4 &color, const Matrix4 &transform) {
    float half = height * 0.5f - radius;
    {
        Vector3 cap(0, half, 0);
        Matrix4 t;
        t = transform * Matrix4(cap, Quaternion(), Vector3(1.0f));
        drawCircle(Vector3(), radius, color, t);

        t = transform * Matrix4(cap, Quaternion(Vector3(-90,  0, 0)), Vector3(1.0f));
        drawArc(Vector3(), radius, 0, 180, color, t);

        t = transform * Matrix4(cap, Quaternion(Vector3(-90, 90, 0)), Vector3(1.0f));
        drawArc(Vector3(), radius, 0, 180, color, t);
    }
    {
        Vector3 cap(0,-half, 0);
        Matrix4 t(transform);
        t = transform * Matrix4(cap, Quaternion(), Vector3(1.0f));
        drawCircle(Vector3(), radius, color, t);

        t = transform * Matrix4(cap, Quaternion(Vector3(90,  0, 0)), Vector3(1.0f));
        drawArc(Vector3(), radius, 0, 180, color, t);

        t = transform * Matrix4(cap, Quaternion(Vector3(90, 90, 0)), Vector3(1.0f));
        drawArc(Vector3(), radius, 0, 180, color, t);
    }

    Vector3Vector points = { Vector3( radius, half, 0),
                             Vector3( radius,-half, 0),
                             Vector3(-radius, half, 0),
                             Vector3(-radius,-half, 0),
                             Vector3( 0, half, radius),
                             Vector3( 0,-half, radius),
                             Vector3( 0, half,-radius),
                             Vector3( 0,-half,-radius)};

    IndexVector indices = {0, 1, 2, 3, 4, 5, 6, 7};

    drawLines(points, indices, color, transform);
}
