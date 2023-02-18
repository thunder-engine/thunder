#include "pipelinepasses/shadowmap.h"

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

#define SM_RESOLUTION_DEFAULT 2048

#define SHADOW_MAP  "shadowMap"

namespace {
    const char *shadowmap("graphics.shadowmap");

    const char *uniLod       = "uni.lod";
    const char *uniMatrix    = "uni.matrix";
    const char *uniTiles     = "uni.tiles";
};

ShadowMap::ShadowMap() {

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

Texture *ShadowMap::draw(Texture *source, PipelineContext *context) {
    cleanShadowCache();

    list<Renderable *> &components = context->sceneComponents();
    for(auto &it : context->sceneLights()) {
        BaseLight *base = static_cast<BaseLight *>(it);
        if(base->castShadows()) {
            switch(base->lightType()) {
            case BaseLight::DirectLight: directLightUpdate(context, static_cast<DirectLight *>(base), components, *context->currentCamera()); break;
            case BaseLight::AreaLight: areaLightUpdate(context, static_cast<AreaLight *>(base), components); break;
            case BaseLight::PointLight: pointLightUpdate(context, static_cast<PointLight *>(base), components); break;
            case BaseLight::SpotLight: spotLightUpdate(context, static_cast<SpotLight *>(base), components); break;
            default: break;
            }
        }
    }
    context->buffer()->resetViewProjection();

    return source;
}

uint32_t ShadowMap::layer() const {
    return CommandBuffer::SHADOWCAST;
}

void ShadowMap::areaLightUpdate(PipelineContext *context, AreaLight *light, list<Renderable *> &components) {
    CommandBuffer *buffer = context->buffer();
    Transform *t = light->transform();

    int32_t x[SIDES], y[SIDES], w[SIDES], h[SIDES];
    RenderTarget *shadowMap = requestShadowTiles(light->uuid(), 1, x, y, w, h, SIDES);

    float zNear = 0.1f;
    float zFar = light->radius();
    Matrix4 crop = Matrix4::perspective(90.0f, 1.0f, zNear, zFar);

    Matrix4 wt = t->worldTransform();
    Vector3 position(wt[12], wt[13], wt[14]);

    Matrix4 wp;
    wp.translate(position);

    Vector4 tiles[SIDES];
    Matrix4 matrix[SIDES];

    uint32_t pageSize = Texture::maxTextureSize();

    buffer->setRenderTarget(shadowMap);
    for(int32_t i = 0; i < m_directions.size(); i++) {
        Matrix4 mat = (wp * Matrix4(m_directions[i].toMatrix())).inverse();
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
        RenderList filter = context->frustumCulling(corners, components, bb);
        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }

    auto instance = light->material();
    if(instance) {
        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles, tiles, SIDES);
        instance->setTexture(SHADOW_MAP, shadowMap->depthAttachment());
    }
}

void ShadowMap::directLightUpdate(PipelineContext *context, DirectLight *light, list<Renderable *> &components, Camera &camera) {
    CommandBuffer *buffer = context->buffer();
    Vector4 distance;

    float nearPlane = camera.nearPlane();

    Matrix4 p = buffer->projection();

    float split = SPLIT_WEIGHT;
    float farPlane = camera.farPlane();
    float ratio = farPlane / nearPlane;

    Vector4 normalizedDistance;
    for(int i = 0; i < MAX_LODS; i++) {
        float f = (i + 1) / static_cast<float>(MAX_LODS);
        float l = nearPlane * powf(ratio, f);
        float u = nearPlane + (farPlane - nearPlane) * f;
        float val = MIX(u, l, split);
        distance[i] = val;
        Vector4 depth = p * Vector4(0.0f, 0.0f, -val * 2.0f - 1.0f, 1.0f);
        normalizedDistance[i] = depth.z / depth.w;
    }

    Transform *lightTransform = light->transform();
    Quaternion q = lightTransform->worldQuaternion();
    Matrix4 rot = Matrix4(q.toMatrix()).inverse();

    Transform *cameraTransform = camera.transform();
    Vector3 wPosition = cameraTransform->worldPosition();
    Quaternion wRotation = cameraTransform->worldQuaternion();

    bool orthographic = camera.orthographic();
    float sigma = (camera.orthographic()) ? camera.orthoSize() : camera.fov();
    ratio = camera.ratio();

    int32_t x[MAX_LODS], y[MAX_LODS], w[MAX_LODS], h[MAX_LODS];
    RenderTarget *shadowMap = requestShadowTiles(light->uuid(), 0, x, y, w, h, MAX_LODS);

    Vector4 tiles[MAX_LODS];
    Matrix4 matrix[MAX_LODS];

    buffer->setRenderTarget(shadowMap);
    for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        float dist = distance[lod];
        auto points = Camera::frustumCorners(orthographic, sigma, ratio, wPosition, wRotation, nearPlane, dist);

        nearPlane = dist;

        AABBox box;
        box.setBox(&(points.at(0)), 8);
        box *= rot;

        Vector3 min, max;
        box.box(min, max);

        Vector3 size(max - min);
        Vector3 pos(min + size * 0.5f);

        min.z = -FLT_MAX;
        max.z = FLT_MAX;

        AABBox bb;
        auto corners = Camera::frustumCorners(true, max.y - min.y, 1.0f, pos, q, min.z, max.z);
        RenderList filter = context->frustumCulling(corners, components, bb);

        min.z = -bb.radius; /// \todo Negative values are bad for Vulkan
        max.z = bb.radius;

        Matrix4 crop = Matrix4::ortho(min.x, max.x, min.y, max.y, min.z, max.z);

        uint32_t pageSize = Texture::maxTextureSize();

        matrix[lod] = m_scale * crop * rot;
        tiles[lod] = Vector4(static_cast<float>(x[lod]) / pageSize,
                             static_cast<float>(y[lod]) / pageSize,
                             static_cast<float>(w[lod]) / pageSize,
                             static_cast<float>(h[lod]) / pageSize);

        buffer->enableScissor(x[lod], y[lod], w[lod], h[lod]);
        buffer->clearRenderTarget();
        buffer->disableScissor();

        buffer->setViewProjection(rot, crop);
        buffer->setViewport(x[lod], y[lod], w[lod], h[lod]);

        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
    }

    auto instance = light->material();
    if(instance) {
        instance->setMatrix4(uniMatrix, matrix, MAX_LODS);
        instance->setVector4(uniTiles, tiles,  MAX_LODS);
        instance->setVector4(uniLod, &normalizedDistance);
        instance->setTexture(SHADOW_MAP, shadowMap->depthAttachment());
    }
}

void ShadowMap::pointLightUpdate(PipelineContext *context, PointLight *light, list<Renderable *> &components) {
    CommandBuffer *buffer = context->buffer();
    Transform *t = light->transform();

    int32_t x[SIDES], y[SIDES], w[SIDES], h[SIDES];
    RenderTarget *shadowMap = requestShadowTiles(light->uuid(), 1, x, y, w, h, SIDES);

    float zNear = 0.1f;
    float zFar = light->attenuationRadius();
    Matrix4 crop = Matrix4::perspective(90.0f, 1.0f, zNear, zFar);

    Matrix4 wt(t->worldTransform());
    Vector3 position(wt[12], wt[13], wt[14]);

    Matrix4 wp;
    wp.translate(position);

    Vector4 tiles[SIDES];
    Matrix4 matrix[SIDES];

    uint32_t pageSize = Texture::maxTextureSize();

    buffer->setRenderTarget(shadowMap);
    for(int32_t i = 0; i < m_directions.size(); i++) {
        Matrix4 mat = (wp * Matrix4(m_directions[i].toMatrix())).inverse();
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
        RenderList filter = context->frustumCulling(corners, components, bb);

        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }

    auto instance = light->material();
    if(instance) {
        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles,  tiles, SIDES);
        instance->setTexture(SHADOW_MAP, shadowMap->depthAttachment());
    }
}

void ShadowMap::spotLightUpdate(PipelineContext *context, SpotLight *light, list<Renderable *> &components) {
    CommandBuffer *buffer = context->buffer();
    Transform *t = light->transform();

    Quaternion q(t->worldQuaternion());
    Matrix4 wt(t->worldTransform());
    Matrix4 rot(wt.inverse());

    Vector3 position(wt[12], wt[13], wt[14]);

    float zNear = 0.1f;
    float zFar = light->attenuationDistance();
    Matrix4 crop = Matrix4::perspective(light->outerAngle() * 2.0f, 1.0f, zNear, zFar);

    int32_t x, y, w, h;
    RenderTarget *shadowMap = requestShadowTiles(light->uuid(), 1, &x, &y, &w, &h, 1);

    uint32_t pageSize = Texture::maxTextureSize();
    Matrix4 matrix = m_scale * crop * rot;
    Vector4 tiles = Vector4(static_cast<float>(x) / pageSize,
                            static_cast<float>(y) / pageSize,
                            static_cast<float>(w) / pageSize,
                            static_cast<float>(h) / pageSize);

    buffer->setRenderTarget(shadowMap);
    buffer->enableScissor(x, y, w, h);
    buffer->clearRenderTarget();
    buffer->disableScissor();

    buffer->setViewProjection(rot, crop);
    buffer->setViewport(x, y, w, h);

    AABBox bb;
    auto corners = Camera::frustumCorners(false, light->outerAngle() * 2.0f, 1.0f, position, q, zNear, zFar);
    RenderList filter = context->frustumCulling(corners, components, bb);

    // Draw in the depth buffer from position of the light source
    for(auto it : filter) {
        it->draw(*buffer, CommandBuffer::SHADOWCAST);
    }
    buffer->resetViewProjection();

    auto instance = light->material();
    if(instance) {
        instance->setMatrix4(uniMatrix, &matrix);
        instance->setVector4(uniTiles,  &tiles);
        instance->setTexture(SHADOW_MAP, shadowMap->depthAttachment());
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

    int32_t width = (SM_RESOLUTION_DEFAULT >> lod);
    int32_t height = (SM_RESOLUTION_DEFAULT >> lod);

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
        Texture *map = Engine::objectCreate<Texture>();
        map->setFormat(Texture::Depth);
        map->setWidth(pageSize);
        map->setHeight(pageSize);
        map->setDepthBits(24);

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
