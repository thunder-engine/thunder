#include "pipelinetasks/shadowmap.h"

#include "engine.h"

#include "commandbuffer.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "components/arealight.h"
#include "components/directlight.h"
#include "components/spotlight.h"
#include "components/pointlight.h"

#include "utils/atlas.h"
#include "resources/rendertarget.h"
#include "resources/material.h"

#include <float.h>

#define SIDES 6
#define MAX_LODS 4

#define SPLIT_WEIGHT 0.95f // 0.75f

namespace {
    const char *shadowmap("graphics.shadowmap");

    const char *shadowMap("shadowMap");

    const char *uniLod("lod");
    const char *uniMatrix("matrix");
    const char *uniTiles("tiles");
    const char *uniBias("bias");
    const char *uniPlaneDistance("planeDistance");
    const char *uniShadows("shadows");
};

ShadowMap::ShadowMap() :
        m_bias(0.0f),
        m_shadowResolution(4096) {

    setName("ShadowMap");

    Engine::setValue(shadowmap, true);

    m_scale[0]  = 0.5f;
    m_scale[5]  = 0.5f;
    m_scale[10] = 0.5f;

    m_scale[12] = 0.5f;
    m_scale[13] = 0.5f;
    m_scale[14] = 0.5f;

    m_directions = {Quaternion(Vector3(0, 1, 0),-90),
                    Quaternion(Vector3(0, 1, 0), 90),
                    Quaternion(Vector3(1, 0, 0), 90),
                    Quaternion(Vector3(1, 0, 0),-90),
                    Quaternion(Vector3(0, 1, 0),180),
                    Quaternion()};
}

void ShadowMap::exec() {
    CommandBuffer *buffer = m_context->buffer();

    buffer->beginDebugMarker("ShadowMap");
    cleanShadowCache();

    RenderList &components = m_context->sceneRenderables();
    for(auto &it : m_context->sceneLights()) {
        BaseLight *base = static_cast<BaseLight *>(it);

        auto instance = base->material();
        if(instance) {
            float shadows = base->castShadows() ? 1.0f : 0.0f;
            instance->setFloat(uniShadows, &shadows);
        }

        if(base->castShadows()) {
            switch(base->lightType()) {
            case BaseLight::DirectLight: directLightUpdate(static_cast<DirectLight *>(base), components); break;
            case BaseLight::AreaLight: areaLightUpdate(static_cast<AreaLight *>(base), components); break;
            case BaseLight::PointLight: pointLightUpdate(static_cast<PointLight *>(base), components); break;
            case BaseLight::SpotLight: spotLightUpdate(static_cast<SpotLight *>(base), components); break;
            default: break;
            }
        }
    }

    m_context->cameraReset();
    buffer->endDebugMarker();
}

void ShadowMap::areaLightUpdate(AreaLight *light, const RenderList &components) {
    Transform *t = light->transform();

    int32_t x[SIDES], y[SIDES], w[SIDES], h[SIDES];
    RenderTarget *shadowTarget = requestShadowTiles(light->uuid(), 1, x, y, w, h, SIDES);

    float zNear = 0.1f;
    float zFar = light->radius();
    Matrix4 crop(Matrix4::perspective(90.0f, 1.0f, zNear, zFar));

    Vector3 position(t->worldTransform().position());

    Matrix4 wp;
    wp.translate(position);

    Vector4 tiles[SIDES];
    Matrix4 matrix[SIDES];

    uint32_t pageSize = Texture::maxTextureSize();

    CommandBuffer *buffer = m_context->buffer();
    buffer->setRenderTarget(shadowTarget);

    for(int32_t i = 0; i < m_directions.size(); i++) {
        Matrix4 mat((wp * Matrix4(m_directions[i].toMatrix())).inverse());
        matrix[i] = m_scale * crop * mat;

        tiles[i] = Vector4(static_cast<float>(x[i]) / pageSize,
                           static_cast<float>(y[i]) / pageSize,
                           static_cast<float>(w[i]) / pageSize,
                           static_cast<float>(h[i]) / pageSize);

        auto frustum = Camera::frustum(false, 90.0f, 1.0f, position, m_directions[i], zNear, zFar);
        // Draw in the depth buffer from position of the light source
        RenderList culled;
        m_context->frustumCulling(frustum, components, culled);

        GroupList list;
        filterByLayer(culled, list, Material::Shadowcast);
        GroupList groups;
        group(list, groups);

        if(!groups.empty()) {
            buffer->setViewProjection(mat, crop);
            buffer->setViewport(x[i], y[i], w[i], h[i]);

            for(auto &it : groups) {
                it.instance->setInstanceBuffer(&it.buffer);
                buffer->drawMesh(it.mesh, it.subMesh, Material::Shadowcast, *it.instance);
                it.instance->setInstanceBuffer(nullptr);
            }
        }
    }

    auto instance = light->material();
    if(instance) {
        Vector4 bias(m_bias);

        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles, tiles, SIDES);
        instance->setVector4(uniBias, &bias);
        instance->setTexture(shadowMap, shadowTarget->depthAttachment());
    }
}

void ShadowMap::directLightUpdate(DirectLight *light, const RenderList &components) {
    const Camera *camera = m_context->currentCamera();



    float nearPlane = camera->nearPlane();

    Matrix4 p(camera->projectionMatrix());

    float split = SPLIT_WEIGHT;
    float farPlane = camera->farPlane();
    float ratio = farPlane / nearPlane;

    Vector4 distance;
    Vector4 planeDistance;
    for(int i = 0; i < MAX_LODS; i++) {
        float f = (i + 1) / static_cast<float>(MAX_LODS);
        float l = nearPlane * powf(ratio, f);
        float u = nearPlane + (farPlane - nearPlane) * f;
        float val = MIX(u, l, split);
        distance[i] = val;
        Vector4 depth = p * Vector4(0.0f, 0.0f, -val * 2.0f - 1.0f, 1.0f);
        planeDistance[i] = depth.z / depth.w;
    }

    Transform *lightTransform = light->transform();
    Quaternion lightRot(lightTransform->worldQuaternion());
    Matrix4 rot(Matrix4(lightRot.toMatrix()).inverse());

    Transform *cameraTransform = camera->transform();
    Vector3 cameraPos(cameraTransform->worldPosition());
    Quaternion cameraRot(cameraTransform->worldQuaternion());

    bool orthographic = camera->orthographic();
    float sigma = (orthographic) ? camera->orthoSize() : camera->fov();
    ratio = camera->ratio();

    int32_t x[MAX_LODS], y[MAX_LODS], w[MAX_LODS], h[MAX_LODS];
    RenderTarget *shadowTarget = requestShadowTiles(light->uuid(), 0, x, y, w, h, MAX_LODS);

    Vector4 tiles[MAX_LODS];
    Matrix4 matrix[MAX_LODS];

    CommandBuffer *buffer = m_context->buffer();
    buffer->setRenderTarget(shadowTarget);

    for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        float dist = distance[lod];
        auto points = Camera::frustumCorners(orthographic, sigma, ratio, cameraPos, cameraRot, nearPlane, dist);

        nearPlane = dist;

        AABBox box;
        box.setBox(points.data(), 8);
        box *= lightRot;

        AABBox bb;
        RenderList culled;
        m_context->frustumCulling(Camera::frustum(true, box.extent.y * 2.0f, 1.0f, box.center, lightRot, -1000.0f, 1000.0f), components, culled, &bb);

        float radius = MAX(box.radius, bb.radius);

        Matrix4 m;
        m.translate(-box.center - lightRot * Vector3(0.0f, 0.0f, radius));
        Matrix4 view(rot * m);
        Matrix4 crop(Matrix4::ortho(-box.extent.x, box.extent.x,
                                    -box.extent.y, box.extent.y,
                                     0.0f, radius * 2.0f));

        uint32_t pageSize = Texture::maxTextureSize();

        matrix[lod] = m_scale * crop * view;
        tiles[lod] = Vector4(static_cast<float>(x[lod]) / pageSize,
                             static_cast<float>(y[lod]) / pageSize,
                             static_cast<float>(w[lod]) / pageSize,
                             static_cast<float>(h[lod]) / pageSize);

        GroupList list;
        filterByLayer(culled, list, Material::Shadowcast);
        GroupList groups;
        group(list, groups);

        if(!groups.empty()) {
            buffer->setViewProjection(view, crop);
            buffer->setViewport(x[lod], y[lod], w[lod], h[lod]);

            // Draw in the depth buffer from position of the light source
            for(auto &it : groups) {
                it.instance->setInstanceBuffer(&it.buffer);
                buffer->drawMesh(it.mesh, it.subMesh, Material::Shadowcast, *it.instance);
                it.instance->setInstanceBuffer(nullptr);
            }
        }
    }

    auto instance = light->material();
    if(instance) {
        Vector4 bias(m_bias);

        const float biasModifier = 0.5f;
        for(int32_t lod = 0; lod < MAX_LODS; lod++) {
            bias[lod] *= 1.0 / (planeDistance[lod] * biasModifier);
        }

        instance->setMatrix4(uniMatrix, matrix, MAX_LODS);
        instance->setVector4(uniTiles, tiles, MAX_LODS);
        instance->setVector4(uniBias, &bias);
        instance->setVector4(uniPlaneDistance, &planeDistance);
        instance->setTexture(shadowMap, shadowTarget->depthAttachment());
    }
}

void ShadowMap::pointLightUpdate(PointLight *light, const RenderList &components) {
    CommandBuffer *buffer = m_context->buffer();
    Transform *t = light->transform();

    int32_t x[SIDES], y[SIDES], w[SIDES], h[SIDES];
    RenderTarget *shadowTarget = requestShadowTiles(light->uuid(), 1, x, y, w, h, SIDES);

    float zNear = 0.1f;
    float zFar = light->attenuationRadius();
    Matrix4 crop(Matrix4::perspective(90.0f, 1.0f, zNear, zFar));

    Vector3 position(t->worldTransform().position());

    Matrix4 wp;
    wp.translate(position);

    Vector4 tiles[SIDES];
    Matrix4 matrix[SIDES];

    uint32_t pageSize = Texture::maxTextureSize();

    buffer->setRenderTarget(shadowTarget);

    for(int32_t i = 0; i < m_directions.size(); i++) {
        Matrix4 mat((wp * Matrix4(m_directions[i].toMatrix())).inverse());
        matrix[i] = m_scale * crop * mat;

        tiles[i] = Vector4(static_cast<float>(x[i]) / pageSize,
                           static_cast<float>(y[i]) / pageSize,
                           static_cast<float>(w[i]) / pageSize,
                           static_cast<float>(h[i]) / pageSize);

        RenderList culled;
        m_context->frustumCulling(Camera::frustum(false, 90.0f, 1.0f, position, m_directions[i], zNear, zFar), components, culled);

        GroupList list;
        filterByLayer(culled, list, Material::Shadowcast);
        GroupList groups;
        group(list, groups);

        if(!groups.empty()) {
            buffer->setViewProjection(mat, crop);
            buffer->setViewport(x[i], y[i], w[i], h[i]);

            // Draw in the depth buffer from position of the light source
            for(auto &it : groups) {
                it.instance->setInstanceBuffer(&it.buffer);
                buffer->drawMesh(it.mesh, it.subMesh, Material::Shadowcast, *it.instance);
                it.instance->setInstanceBuffer(nullptr);
            }
        }
    }

    auto instance = light->material();
    if(instance) {
        Vector4 bias(m_bias);

        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles,  tiles, SIDES);
        instance->setVector4(uniBias, &bias);
        instance->setTexture(shadowMap, shadowTarget->depthAttachment());
    }
}

void ShadowMap::spotLightUpdate(SpotLight *light, const RenderList &components) {
    CommandBuffer *buffer = m_context->buffer();
    Transform *t = light->transform();

    Quaternion q(t->worldQuaternion());
    Matrix4 wt(t->worldTransform());
    Matrix4 rot(wt.inverse());

    Vector3 position(wt.position());

    float zNear = 0.1f;
    float zFar = light->attenuationDistance();
    Matrix4 crop(Matrix4::perspective(light->outerAngle(), 1.0f, zNear, zFar));

    int32_t x = 0;
    int32_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
    RenderTarget *shadowTarget = requestShadowTiles(light->uuid(), 1, &x, &y, &w, &h, 1);

    RenderList culled;
    m_context->frustumCulling(Camera::frustum(false, light->outerAngle() * 2.0f, 1.0f, position, q, zNear, zFar), components, culled);

    GroupList list;
    filterByLayer(culled, list, Material::Shadowcast);
    GroupList groups;
    group(list, groups);

    if(!groups.empty()) {
        buffer->setRenderTarget(shadowTarget);

        buffer->setViewProjection(rot, crop);
        buffer->setViewport(x, y, w, h);

        // Draw in the depth buffer from position of the light source
        for(auto &it : groups) {
            it.instance->setInstanceBuffer(&it.buffer);
            buffer->drawMesh(it.mesh, it.subMesh, Material::Shadowcast, *it.instance);
            it.instance->setInstanceBuffer(nullptr);
        }
    }

    auto instance = light->material();
    if(instance) {
        Vector4 bias(m_bias);

        uint32_t pageSize = Texture::maxTextureSize();
        Matrix4 matrix(m_scale * crop * rot);
        Vector4 tiles(static_cast<float>(x) / pageSize,
                      static_cast<float>(y) / pageSize,
                      static_cast<float>(w) / pageSize,
                      static_cast<float>(h) / pageSize);

        instance->setMatrix4(uniMatrix, &matrix);
        instance->setVector4(uniTiles,  &tiles);
        instance->setVector4(uniBias, &bias);
        instance->setTexture(shadowMap, shadowTarget->depthAttachment());
    }
}

void ShadowMap::cleanShadowCache() {
    for(auto tiles = m_tiles.begin(); tiles != m_tiles.end(); ) {
        bool outdate = false;
        for(auto &it : tiles->second.second) {
            if(it->dirty == true) {
                outdate = true;
                break;
            }
        }
        if(outdate) {
            for(auto &it : tiles->second.second) {
                delete it;
            }
            tiles = m_tiles.erase(tiles);
        } else {
            ++tiles;
        }
    }
    /// \todo This activity leads to crash
    //for(auto &it : m_shadowPages) {
    //    it.second->clean();
    //}

    for(auto &tile : m_tiles) {
        for(auto &it : tile.second.second) {
            it->dirty = true;
        }
    }
}

RenderTarget *ShadowMap::requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count) {
    auto tile = m_tiles.find(id);
    if(tile != m_tiles.end()) {
        for(uint32_t i = 0; i < count; i++) {
            AtlasNode *node = tile->second.second[i];
            x[i] = node->x;
            y[i] = node->y;
            w[i] = node->w;
            h[i] = node->h;
            node->dirty = false;
        }
        return tile->second.first;
    }

    int32_t width = (m_shadowResolution >> lod);
    int32_t height = (m_shadowResolution >> lod);

    uint32_t columns = MAX(count / 2, 1);
    uint32_t rows = count / columns;

    RenderTarget *target = nullptr;
    AtlasNode *sub = nullptr;

    for(auto page : m_shadowPages) {
        target = page.first;
        AtlasNode *root = page.second;

        sub = root->insert(width * columns, height * rows);
        if(sub) {
            break;
        }
    }

    if(sub == nullptr) {
        uint32_t pageSize = Texture::maxTextureSize();
        Texture *map = Engine::objectCreate<Texture>(std::string("shadowAtlas ") + std::to_string(m_shadowPages.size()));
        map->setFormat(Texture::Depth);
        map->setDepthBits(24);
        map->setFlags(Texture::Render);

        map->resize(pageSize, pageSize);

        m_context->addTextureBuffer(map);

        target = Engine::objectCreate<RenderTarget>();
        target->setDepthAttachment(map);
        target->setClearFlags(RenderTarget::ClearDepth);

        AtlasNode *root = new AtlasNode;

        root->w = pageSize;
        root->h = pageSize;

        m_shadowPages[target] = root;

        sub = root->insert(width * columns, height * rows);
    }

    std::vector<AtlasNode *> tiles;
    for(uint32_t i = 0; i < count; i++) {
        AtlasNode *node = sub->insert(width, height);
        if(node) {
            x[i] = node->x;
            y[i] = node->y;
            w[i] = node->w;
            h[i] = node->h;
            node->fill = true;
            tiles.push_back(node);
        }
    }
    if(tiles.size() == count) {
        m_tiles[id] = make_pair(target, tiles);
    }

    target->setRenderArea(x[0], y[0], width * columns, height * rows);
    return target;
}
