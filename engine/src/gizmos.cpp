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

void Gizmos::clear() {
    s_wire->clear();
    s_solid->clear();

    for(auto &it : s_sprites) {
        it.second.mesh->clear();
    }
}

void Gizmos::drawSpriteBatch(CommandBuffer *buffer) {
    for(auto &it : s_sprites) {
        if(!it.second.mesh->isEmpty()) {
            buffer->drawMesh(Matrix4(), it.second.mesh, 0, CommandBuffer::TRANSLUCENT, it.second.material);
        }
    }
}

void Gizmos::drawSolidBatch(CommandBuffer *buffer) {
    if(!s_solid->isEmpty()) {
        buffer->drawMesh(Matrix4(), s_solid, 0, CommandBuffer::TRANSLUCENT, s_solidMaterial);
    }
}

void Gizmos::drawWireBatch(CommandBuffer *buffer) {
    if(!s_wire->isEmpty()) {
        buffer->drawMesh(Matrix4(), s_wire, 0, CommandBuffer::TRANSLUCENT, s_wireMaterial);
    }
}

void Gizmos::drawBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 &transform) {
    Vector3 min(center - size * 0.5f);
    Vector3 max(center + size * 0.5f);

    Mesh mesh;
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

void Gizmos::drawIcon(const Vector3 &center, const Vector2 &size, const string &name, const Vector4 &color, const Matrix4 &transform) {
    Matrix4 model(center, Quaternion(), Vector3(size, size.x));
    Matrix4 q = model * Matrix4(Camera::current()->transform()->quaternion().toMatrix());

    Mesh mesh;
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

void Gizmos::drawMesh(Mesh &mesh, const Vector4 &color, const Matrix4 &transform) {
    Mesh m;
    m.setVertices(mesh.vertices());
    m.setIndices(mesh.indices());
    m.setNormals(mesh.normals());
    m.setTangents(mesh.tangents());
    m.setUv0(mesh.uv0());
    m.setColors(Vector4Vector(m.vertices().size(), color));

    s_solid->batchMesh(m, &transform);
}

void Gizmos::drawSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform) {

}

void Gizmos::drawDisk(const Vector3 &center, float radius, float start, float angle, const Vector4 &color, const Matrix4 &transform) {
    Mesh mesh;
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

void Gizmos::drawLines(const Vector3Vector &points, const IndexVector &indices, const Vector4 &color, const Matrix4 &transform) {
    Mesh mesh;
    mesh.setVertices(points);
    mesh.setIndices(indices);
    mesh.setColors(Vector4Vector(points.size(), color));

    s_wire->batchMesh(mesh, &transform);
}

void Gizmos::drawArc(const Vector3 &center, float radius, float from, float to, const Vector4 &color, const Matrix4 &transform) {
    Mesh mesh;
    mesh.setVertices(Mathf::pointsArc(Quaternion(), radius, from, to, 180));
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

void Gizmos::drawCircle(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform) {
    drawArc(center, radius, 0, 360, color, transform);
}

void Gizmos::drawRectangle(const Vector3 &center, const Vector2 &size, const Vector4 &color, const Matrix4 &transform) {
    Vector2 min(Vector2(center.x, center.y) - size * 0.5f);
    Vector2 max(Vector2(center.x, center.y) + size * 0.5f);

    Mesh mesh;
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

void Gizmos::drawWireBox(const Vector3 &center, const Vector3 &size, const Vector4 &color, const Matrix4 &transform) {
    Vector3 min(center - size * 0.5f);
    Vector3 max(center + size * 0.5f);

    Mesh mesh;
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

void Gizmos::drawWireMesh(Mesh &mesh, const Vector4 &color, const Matrix4 &transform) {
    Mesh m;
    m.setVertices(mesh.vertices());
    m.setIndices(mesh.indices());
    m.setColors(Vector4Vector(m.vertices().size(), color));

    s_wire->batchMesh(m, &transform);
}

void Gizmos::drawWireSphere(const Vector3 &center, float radius, const Vector4 &color, const Matrix4 &transform) {
    drawCircle(center, radius, color, transform);
    Matrix4 t = transform * Matrix4(Quaternion(Vector3(1, 0, 0), 90).toMatrix());
    drawCircle(center, radius, color, t);
    t = transform * Matrix4(Quaternion(Vector3(0, 0, 1), 90).toMatrix());
    drawCircle(center, radius, color, t);
}

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
