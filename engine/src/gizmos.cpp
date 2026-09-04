#include "gizmos.h"

#include "components/camera.h"
#include "components/transform.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "commandbuffer.h"

Mesh *Gizmos::s_wire = nullptr;
Mesh *Gizmos::s_solid = nullptr;

MaterialInstance *Gizmos::s_gizmoMaterial = nullptr;

Material *Gizmos::s_spriteMaterial = nullptr;

std::unordered_map<TString, Gizmos::SpriteBatches> Gizmos::s_sprites;

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
    if(s_gizmoMaterial == nullptr) {
        Material *m = Engine::loadResource<Material>(".embedded/gizmo.shader");
        if(m) {
            s_gizmoMaterial = m->createInstance();
        }
    }
    if(s_spriteMaterial == nullptr) {
        s_spriteMaterial = Engine::loadResource<Material>(".embedded/DefaultSprite.shader");
    }

    if(s_wire == nullptr) {
        s_wire = Engine::objectCreate<Mesh>("Gizmo Lines Batch");
        s_wire->makeDynamic();
        s_wire->setTopology(Mesh::Lines);
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
            buffer->drawMesh(it.second.mesh, 0, Material::Translucent, *it.second.material);
        }
    }
}
/*!
    \internal
    Draws the solid batch using the provided command buffer.
*/
void Gizmos::drawSolidBatch(CommandBuffer *buffer) {
    if(!s_solid->isEmpty()) {
        s_solid->recalcBounds();
        buffer->drawMesh(s_solid, 0, Material::Translucent, *s_gizmoMaterial);
    }
}
/*!
    \internal
    Draws the wireframe batch using the provided command buffer.
*/
void Gizmos::drawWireBatch(CommandBuffer *buffer) {
    if(!s_wire->isEmpty()) {
        s_wire->recalcBounds();
        buffer->drawMesh(s_wire, 0, Material::Translucent, *s_gizmoMaterial);
    }
}
/*!
    Draws a solid box with specified \a center, \a size and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this box.
*/
void Gizmos::drawSolidBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 *transform) {
    Vector3 min(center - size * 0.5f);
    Vector3 max(center + size * 0.5f);

    drawSolidTriangles({
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, min.y, max.z),
        Vector3(min.x, min.y, max.z),

        Vector3(min.x, max.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)
    },
    {0, 1, 2, 0, 2, 3, // bottom
     4, 6, 5, 4, 7, 6, // top
     0, 1, 5, 0, 5, 4, // front
     3, 6, 2, 3, 7, 6, // back
     0, 7, 3, 0, 4, 7, // left
     1, 6, 2, 1, 5, 6, // right
    },
    color, transform);
}
/*!
    Draws an billboard icon at the specified \a center with the given \a size, \a color.
    Parameter \a name will be used to set a texture to render.
*/
void Gizmos::drawIcon(const Vector3 &center, const Vector2 &size, const TString &name, const Vector4 &color) {
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
            Vector4 color(1.0f);
            batch.material->setVector4("mainColor", &color);
            batch.material->setTexture("mainTexture", Engine::loadResource<Texture>(name));
            s_sprites[name] = batch;
        }
    }
}
/*!
    Draws a \a mesh with a specified \a color and \a transform.
*/
void Gizmos::drawSolidMesh(Mesh &mesh, const Vector4 &color, const Matrix4 *transform) {
    drawSolidTriangles(mesh.vertices(), mesh.indices(), color, transform);
}
/*!
    Draws a solid sphere with specified \a center, \a radius and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this sphere.
*/
void Gizmos::drawSolidSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 *transform) {
    static const uint32_t steps = 12;
    static const float stackStep = PI / (float)steps;
    static const float sectorStep = 2.0f * stackStep;

    Vector3Vector vertices;
    vertices.reserve((steps + 1) * (steps + 1));

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

    IndexVector indices;
    indices.reserve((steps + 1) * (steps + 1) * 6);

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

    drawSolidTriangles(vertices, indices, color, transform);
}
/*!
    Draws a solid sector in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameters \a start and \a angle allows to specify angles to draw a sector in degrees.
    Parameter \a transform can be used to move, rotate and scale this arc.
*/
void Gizmos::drawSolidSector(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 *transform) {
    Vector3Vector points = Mathf::pointsArc(center, Quaternion(), radius, start, angle, 180, true);
    size_t size = points.size();

    IndexVector indices;
    indices.resize((size - 1) * 3);
    for(int i = 0; i < size - 1; i++) {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i;
        indices[i * 3 + 2] = i+1;
    }

    drawSolidTriangles(points, indices, color, transform);
}
/*!
    Draws solid triangles connecting specified \a points and \a color in 3D space.
    Parameter \a indices specifies relations between points.
    Parameter \a transform can be used to move, rotate and scale this structure.
*/
void Gizmos::drawSolidTriangles(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, const Matrix4 *transform) {
    batchMesh(points, indices, color, s_solid, transform);
}
/*!
    Draws lines connecting specified \a points and \a color in 3D space.
    Parameter \a indices specifies relations between points.
    Parameter \a transform can be used to move, rotate and scale this structure.
*/
void Gizmos::drawLines(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, const Matrix4 *transform) {
    batchMesh(points, indices, color, s_wire, transform);
}
/*!
    Draws a wire arc in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameters \a start and \a angle allows to specify angles to draw a sector in degrees.
    Parameter \a transform can be used to move, rotate and scale this arc.
*/
void Gizmos::drawWireArc(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 *transform) {
    Vector3Vector points = Mathf::pointsArc(center, Quaternion(), radius, start, angle, 180);
    size_t size = points.size() - 1;

    IndexVector indices;
    indices.resize(size * 2);
    for(int i = 0; i < size; i++) {
        indices[i * 2] = i;
        indices[i * 2 + 1] = i+1;
    }

    drawLines(points, indices, color, transform);
}
/*!
    Draws a wire circle in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this circle.
*/
void Gizmos::drawWireCircle(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 *transform) {
    drawWireArc(center, radius, 0, 360, color, transform);
}
/*!
    Draws a wire rectangle in the 3D space with the specified \a center, \a size and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this rectangle.
*/
void Gizmos::drawWireRectangle(const Vector3 &center, const Vector2 &size, const Vector4 &color, const Matrix4 *transform) {
    Vector2 min(Vector2(center.x, center.y) - size * 0.5f);
    Vector2 max(Vector2(center.x, center.y) + size * 0.5f);

    drawLines({
        Vector3(min.x, min.y, center.z),
        Vector3(max.x, min.y, center.z),
        Vector3(max.x, max.y, center.z),
        Vector3(min.x, max.y, center.z)
    },
    { 0, 1, 1, 2, 2, 3, 3, 0 }, color, transform);
}
/*!
    Draws a wire box in the 3D space with the specified \a center, \a size and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this box.
*/
void Gizmos::drawWireBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 *transform) {
    Vector3 min(center - size * 0.5f);
    Vector3 max(center + size * 0.5f);

    drawLines({
        Vector3(min.x, min.y, min.z),
        Vector3(max.x, min.y, min.z),
        Vector3(max.x, min.y, max.z),
        Vector3(min.x, min.y, max.z),

        Vector3(min.x, max.y, min.z),
        Vector3(max.x, max.y, min.z),
        Vector3(max.x, max.y, max.z),
        Vector3(min.x, max.y, max.z)
    },
    {   0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7
    }, color, transform);
}
/*!
    Draws a wireframe version of the specified \a mesh and \a color in 3D space.
    Parameter \a transform can be used to move, rotate and scale this mesh.
*/
void Gizmos::drawWireMesh(Mesh &mesh, const Vector4 &color, const Matrix4 *transform) {
    batchMesh(mesh.vertices(), mesh.indices(), color, s_wire, transform);
}
/*!
    Draws a wire sphere in the 3D space with the specified \a center, \a radius and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this sphere.
*/
void Gizmos::drawWireSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 *transform) {
    drawWireCircle(center, radius, color, transform);
    Matrix4 t = Matrix4(Quaternion(Vector3(1, 0, 0), 90).toMatrix());
    if(transform) {
        t = *transform * t;
    }
    drawWireCircle(center, radius, color, &t);
    t = Matrix4(Quaternion(Vector3(0, 0, 1), 90).toMatrix());
    if(transform) {
        t = *transform * t;
    }
    drawWireCircle(center, radius, color, &t);
}
/*!
    Draws a wire capsule in the 3D space with the specified \a center, \a radius, \a height and \a color in the 3D space.
    Parameter \a transform can be used to move, rotate and scale this capsule.
*/
void Gizmos::drawWireCapsule(const Vector3 &center, float radius, float height, const Vector4 &color, const Matrix4 *transform) {
    A_UNUSED(center);
    float half = height * 0.5f - radius;
    {
        Vector3 cap(0, half, 0);
        Matrix4 t = Matrix4(cap, Quaternion(), Vector3(1.0f));
        if(transform) {
            t = *transform * t;
        }
        drawWireCircle(Vector3(), radius, color, &t);

        t = Matrix4(cap, Quaternion(Vector3(-90,  0, 0)), Vector3(1.0f));
        if(transform) {
            t = *transform * t;
        }
        drawWireArc(Vector3(), radius, 0, 180, color, &t);

        t = Matrix4(cap, Quaternion(Vector3(-90, 90, 0)), Vector3(1.0f));
        if(transform) {
            t = *transform * t;
        }
        drawWireArc(Vector3(), radius, 0, 180, color, &t);
    }
    {
        Vector3 cap(0,-half, 0);
        Matrix4 t = Matrix4(cap, Quaternion(), Vector3(1.0f));
        if(transform) {
            t = *transform * t;
        }
        drawWireCircle(Vector3(), radius, color, &t);

        t = Matrix4(cap, Quaternion(Vector3(90,  0, 0)), Vector3(1.0f));
        if(transform) {
            t = *transform * t;
        }
        drawWireArc(Vector3(), radius, 0, 180, color, &t);

        t = Matrix4(cap, Quaternion(Vector3(90, 90, 0)), Vector3(1.0f));
        if(transform) {
            t = *transform * t;
        }
        drawWireArc(Vector3(), radius, 0, 180, color, &t);
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
/*!
    \internal
*/
void Gizmos::batchMesh(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, Mesh *mesh, const Matrix4 *transform) {
    size_t size = mesh->vertices().size();

    if(transform) {
        Vector3Vector vertexVector(points);
        for(auto &v : vertexVector) {
            v = *transform * v;
        }
        mesh->vertices().insert(mesh->vertices().end(), vertexVector.begin(), vertexVector.end());
    } else {
        mesh->vertices().insert(mesh->vertices().end(), points.begin(), points.end());
    }

    IndexVector indexVector(indices);
    for(auto &it : indexVector) {
        it += size;
    }
    mesh->indices().insert(mesh->indices().end(), indexVector.begin(), indexVector.end());
    mesh->colors().insert(mesh->colors().end(), points.size(), color);
}
