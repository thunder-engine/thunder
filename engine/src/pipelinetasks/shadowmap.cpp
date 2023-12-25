#include "pipelinetasks/shadowmap.h"

#include "engine.h"

#include "commandbuffer.h"
#include "pipelinecontext.h"
#include "material.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/camera.h"

#include "components/arealight.h"
#include "components/directlight.h"
#include "components/spotlight.h"
#include "components/pointlight.h"

#include "resources/atlas.h"
#include "resources/rendertarget.h"

#include <float.h>

#define SIDES 6
#define MAX_LODS 4

#define SPLIT_WEIGHT 0.95f // 0.75f

#define SHADOW_MAP  "shadowMap"

namespace {
    const char *shadowmap("graphics.shadowmap");

    const char *uniLod = "lod";
    const char *uniMatrix = "matrix";
    const char *uniTiles = "tiles";
    const char *uniBias = "bias";
    const char *uniPlaneDistance = "planeDistance";
    const char *uniShadows = "shadows";
};

ShadowMap::ShadowMap() :
        m_bias(0.0f),
        m_shadowResolution(4096) {
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

void ShadowMap::exec(PipelineContext &context) {
    CommandBuffer *buffer = context.buffer();
    buffer->beginDebugMarker("ShadowMap");
    cleanShadowCache();

    list<Renderable *> &components = context.sceneComponents();
    for(auto &it : context.sceneLights()) {
        BaseLight *base = static_cast<BaseLight *>(it);

        auto instance = base->material();
        if(instance) {
            float shadows = base->castShadows() ? 1.0f : 0.0;
            instance->setFloat(uniShadows, &shadows);
        }

        if(base->castShadows()) {
            switch(base->lightType()) {
            case BaseLight::DirectLight: directLightUpdate(context, static_cast<DirectLight *>(base), components, *context.currentCamera()); break;
            case BaseLight::AreaLight: areaLightUpdate(context, static_cast<AreaLight *>(base), components); break;
            case BaseLight::PointLight: pointLightUpdate(context, static_cast<PointLight *>(base), components); break;
            case BaseLight::SpotLight: spotLightUpdate(context, static_cast<SpotLight *>(base), components); break;
            default: break;
            }
        }
    }

    buffer->resetViewProjection();
    buffer->endDebugMarker();
}

void ShadowMap::areaLightUpdate(PipelineContext &context, AreaLight *light, list<Renderable *> &components) {
    CommandBuffer *buffer = context.buffer();
    Transform *t = light->transform();

    int32_t x[SIDES], y[SIDES], w[SIDES], h[SIDES];
    RenderTarget *shadowTarget = requestShadowTiles(context, light->uuid(), 1, x, y, w, h, SIDES);

    float zNear = 0.1f;
    float zFar = light->radius();
    Matrix4 crop(Matrix4::perspective(90.0f, 1.0f, zNear, zFar));

    Matrix4 wt(t->worldTransform());
    Vector3 position(wt[12], wt[13], wt[14]);

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

        buffer->enableScissor(x[i], y[i], w[i], h[i]);
        buffer->clearRenderTarget();
        buffer->disableScissor();

        buffer->setViewProjection(mat, crop);
        buffer->setViewport(x[i], y[i], w[i], h[i]);

        AABBox bb;
        auto corners = Camera::frustumCorners(false, 90.0f, 1.0f, position, m_directions[i], zNear, zFar);
        RenderList filter = context.frustumCulling(corners, components, bb);
        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }

    auto instance = light->material();
    if(instance) {
        Vector4 bias(m_bias);

        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles, tiles, SIDES);
        instance->setVector4(uniBias, &bias);
        instance->setTexture(SHADOW_MAP, shadowTarget->depthAttachment());
    }
}

void ShadowMap::directLightUpdate(PipelineContext &context, DirectLight *light, list<Renderable *> &components, const Camera &camera) {
    CommandBuffer *buffer = context.buffer();

    float nearPlane = camera.nearPlane();

    Matrix4 p(camera.projectionMatrix());

    float split = SPLIT_WEIGHT;
    float farPlane = camera.farPlane();
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

    Transform *cameraTransform = camera.transform();
    Vector3 cameraPos(cameraTransform->worldPosition());
    Quaternion cameraRot(cameraTransform->worldQuaternion());

    bool orthographic = camera.orthographic();
    float sigma = (camera.orthographic()) ? camera.orthoSize() : camera.fov();
    ratio = camera.ratio();

    int32_t x[MAX_LODS], y[MAX_LODS], w[MAX_LODS], h[MAX_LODS];
    RenderTarget *shadowTarget = requestShadowTiles(context, light->uuid(), 0, x, y, w, h, MAX_LODS);

    Vector4 tiles[MAX_LODS];
    Matrix4 matrix[MAX_LODS];

    buffer->setRenderTarget(shadowTarget);
    for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        float dist = distance[lod];
        auto points = Camera::frustumCorners(orthographic, sigma, ratio, cameraPos, cameraRot, nearPlane, dist);

        nearPlane = dist;

        AABBox box;
        box.setBox(points.data(), 8);
        box *= rot.rotation();

        AABBox bb;
        auto corners = Camera::frustumCorners(true, box.extent.y * 2.0f, 1.0f, box.center, lightRot, -FLT_MAX, FLT_MAX);
        RenderList filter(context.frustumCulling(corners, components, bb));

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

        buffer->enableScissor(x[lod], y[lod], w[lod], h[lod]);
        buffer->clearRenderTarget();

        buffer->setViewProjection(view, crop);
        buffer->setViewport(x[lod], y[lod], w[lod], h[lod]);

        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }

        buffer->disableScissor();
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
        instance->setTexture(SHADOW_MAP, shadowTarget->depthAttachment());
    }
}

void ShadowMap::pointLightUpdate(PipelineContext &context, PointLight *light, list<Renderable *> &components) {
    CommandBuffer *buffer = context.buffer();
    Transform *t = light->transform();

    int32_t x[SIDES], y[SIDES], w[SIDES], h[SIDES];
    RenderTarget *shadowTarget = requestShadowTiles(context, light->uuid(), 1, x, y, w, h, SIDES);

    float zNear = 0.1f;
    float zFar = light->attenuationRadius();
    Matrix4 crop(Matrix4::perspective(90.0f, 1.0f, zNear, zFar));

    Matrix4 wt(t->worldTransform());
    Vector3 position(wt[12], wt[13], wt[14]);

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

        buffer->enableScissor(x[i], y[i], w[i], h[i]);
        buffer->clearRenderTarget();
        buffer->disableScissor();

        buffer->setViewProjection(mat, crop);
        buffer->setViewport(x[i], y[i], w[i], h[i]);

        AABBox bb;
        auto corners = Camera::frustumCorners(false, 90.0f, 1.0f, position, m_directions[i], zNear, zFar);
        RenderList filter = context.frustumCulling(corners, components, bb);

        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }

    auto instance = light->material();
    if(instance) {
        Vector4 bias(m_bias);

        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles,  tiles, SIDES);
        instance->setVector4(uniBias, &bias);
        instance->setTexture(SHADOW_MAP, shadowTarget->depthAttachment());
    }
}

void ShadowMap::spotLightUpdate(PipelineContext &context, SpotLight *light, list<Renderable *> &components) {
    CommandBuffer *buffer = context.buffer();
    Transform *t = light->transform();

    Quaternion q(t->worldQuaternion());
    Matrix4 wt(t->worldTransform());
    Matrix4 rot(wt.inverse());

    Vector3 position(wt[12], wt[13], wt[14]);

    float zNear = 0.1f;
    float zFar = light->attenuationDistance();
    Matrix4 crop(Matrix4::perspective(light->outerAngle(), 1.0f, zNear, zFar));

    int32_t x = 0;
    int32_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
    RenderTarget *shadowTarget = requestShadowTiles(context, light->uuid(), 1, &x, &y, &w, &h, 1);

    buffer->setRenderTarget(shadowTarget);
    buffer->enableScissor(x, y, w, h);
    buffer->clearRenderTarget();
    buffer->disableScissor();

    buffer->setViewProjection(rot, crop);
    buffer->setViewport(x, y, w, h);

    AABBox bb;
    auto corners = Camera::frustumCorners(false, light->outerAngle() * 2.0f, 1.0f, position, q, zNear, zFar);
    RenderList filter = context.frustumCulling(corners, components, bb);

    // Draw in the depth buffer from position of the light source
    for(auto it : filter) {
        it->draw(*buffer, CommandBuffer::SHADOWCAST);
    }
    buffer->resetViewProjection();

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
        instance->setTexture(SHADOW_MAP, shadowTarget->depthAttachment());
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

RenderTarget *ShadowMap::requestShadowTiles(PipelineContext &context, uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count) {
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
        Texture *map = Engine::objectCreate<Texture>(string("shadowAtlas ") + to_string(m_shadowPages.size()));
        map->setFormat(Texture::Depth);
        map->setWidth(pageSize);
        map->setHeight(pageSize);
        map->setDepthBits(24);

        context.addTextureBuffer(map);

        target = Engine::objectCreate<RenderTarget>();
        target->setDepthAttachment(map);

        AtlasNode *root = new AtlasNode;

        root->w = pageSize;
        root->h = pageSize;

        m_shadowPages[target] = root;

        sub = root->insert(width * columns, height * rows);
    }

    vector<AtlasNode *> tiles;
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
    return target;
}
