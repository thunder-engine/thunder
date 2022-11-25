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
    const char *uniPosition  = "uni.position";
    const char *uniDirection = "uni.direction";
    const char *uniRight     = "uni.right";
    const char *uniUp        = "uni.up";
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

    CommandBuffer *buffer = context->buffer();

    list<Renderable *> &components = context->sceneComponents();
    for(auto &it : context->sceneLights()) {
        BaseLight *base = static_cast<BaseLight *>(it);
        if(base->castShadows()) {
            switch(base->lightType()) {
            case BaseLight::DirectLight: directLightUpdate(buffer, static_cast<DirectLight *>(base), components, *context->currentCamera()); break;
            case BaseLight::AreaLight: areaLightUpdate(buffer, static_cast<AreaLight *>(base), components); break;
            case BaseLight::PointLight: pointLightUpdate(buffer, static_cast<PointLight *>(base), components); break;
            case BaseLight::SpotLight: spotLightUpdate(buffer, static_cast<SpotLight *>(base), components); break;
            default: break;
            }
        }
    }
    buffer->resetViewProjection();

    return source;
}

uint32_t ShadowMap::layer() const {
    return CommandBuffer::SHADOWCAST;
}

void ShadowMap::areaLightUpdate(CommandBuffer *buffer, AreaLight *light, list<Renderable *> &components) {
    Transform *t = light->actor()->transform();

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

        RenderList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(false, 90.0f, 1.0f, position, m_directions[i], zNear, zFar));
        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }

    auto instance = light->material();
    if(instance) {
        Vector3 direction(wt.rotation() * Vector3(0.0f, 0.0f, 1.0f));
        Vector3 right(wt.rotation() * Vector3(1.0f, 0.0f, 0.0f));
        Vector3 up(wt.rotation() * Vector3(0.0f, 1.0f, 0.0f));

        instance->setVector3(uniPosition, &position);
        instance->setVector3(uniDirection, &direction);
        instance->setVector3(uniRight, &right);
        instance->setVector3(uniUp, &up);

        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles, tiles, SIDES);
        instance->setTexture(SHADOW_MAP, shadowMap->depthAttachment());
    }
}

void ShadowMap::directLightUpdate(CommandBuffer *buffer, DirectLight *light, list<Renderable *> &components, Camera &camera) {
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

    Transform *transform = light->actor()->transform();
    Quaternion q = transform->worldQuaternion();
    Matrix4 rot = Matrix4(q.toMatrix()).inverse();

    Transform *t = camera.actor()->transform();
    bool orthographic = camera.orthographic();
    float sigma = (camera.orthographic()) ? camera.orthoSize() : camera.fov();
    ratio = camera.ratio();
    Vector3 wPosition = t->worldPosition();
    Quaternion wRotation = t->worldQuaternion();

    int32_t x[MAX_LODS], y[MAX_LODS], w[MAX_LODS], h[MAX_LODS];
    RenderTarget *shadowMap = requestShadowTiles(light->uuid(), 0, x, y, w, h, MAX_LODS);

    Vector4 tiles[MAX_LODS];
    Matrix4 matrix[MAX_LODS];

    buffer->setRenderTarget(shadowMap);
    for(int32_t lod = 0; lod < MAX_LODS; lod++) {
        float dist = distance[lod];
        const array<Vector3, 8> &points = Camera::frustumCorners(orthographic, sigma, ratio,
                                                                 wPosition, wRotation, nearPlane, dist);
        nearPlane = dist;

        AABBox box;
        box.setBox(&(points.at(0)), 8);

        Vector3 min, max;
        box.box(min, max);

        Vector3 rotPoints[8]  = {
            rot * Vector3(min.x, min.y, min.z),
            rot * Vector3(min.x, min.y, max.z),
            rot * Vector3(max.x, min.y, max.z),
            rot * Vector3(max.x, min.y, min.z),

            rot * Vector3(min.x, max.y, min.z),
            rot * Vector3(min.x, max.y, max.z),
            rot * Vector3(max.x, max.y, max.z),
            rot * Vector3(max.x, max.y, min.z)
        };

        min.x = FLT_MAX;
        max.x =-FLT_MAX;

        min.y = FLT_MAX;
        max.y =-FLT_MAX;

        for(uint32_t i = 0; i < 8; i++) {
            min.x = MIN(min.x, rotPoints[i].x);
            max.x = MAX(max.x, rotPoints[i].x);

            min.y = MIN(min.y, rotPoints[i].y);
            max.y = MAX(max.y, rotPoints[i].y);
        }

        /// \todo Must be replaced by the calculations
        min.z = -100.0f; /// \todo Negative values are bad for Vulkan
        max.z = 100.0f;

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

        Vector3 size(max - min);
        Vector3 pos(min + size * 0.5f);

        RenderList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(true, max.y - min.y, 1.0f,
                                                                          pos, q, min.z, max.z));

        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
    }

    auto instance = light->material();
    if(instance) {
        Vector3 direction(q * Vector3(0.0f, 0.0f, 1.0f));

        instance->setMatrix4(uniMatrix, matrix, MAX_LODS);
        instance->setVector4(uniTiles, tiles,  MAX_LODS);
        instance->setVector3(uniDirection, &direction);
        instance->setVector4(uniLod, &normalizedDistance);
        instance->setTexture(SHADOW_MAP, shadowMap->depthAttachment());
    }
}

void ShadowMap::pointLightUpdate(CommandBuffer *buffer, PointLight *light, list<Renderable *> &components) {
    Transform *t = light->actor()->transform();

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

        RenderList filter = Camera::frustumCulling(components,
                                                   Camera::frustumCorners(false, 90.0f, 1.0f, position,
                                                                          m_directions[i], zNear, zFar));
        // Draw in the depth buffer from position of the light source
        for(auto it : filter) {
            static_cast<Renderable *>(it)->draw(*buffer, CommandBuffer::SHADOWCAST);
        }
        buffer->resetViewProjection();
    }

    auto instance = light->material();
    if(instance) {
        Vector3 direction(wt.rotation() * Vector3(0.0f, 1.0f, 0.0f));

        instance->setVector3(uniPosition, &position);
        instance->setVector3(uniDirection, &direction);
        instance->setMatrix4(uniMatrix, matrix, SIDES);
        instance->setVector4(uniTiles,  tiles, SIDES);
        instance->setTexture(SHADOW_MAP, shadowMap->depthAttachment());
    }
}

void ShadowMap::spotLightUpdate(CommandBuffer *buffer, SpotLight *light, list<Renderable *> &components) {
    Transform *t = light->actor()->transform();

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

    RenderList filter = Camera::frustumCulling(components,
                                               Camera::frustumCorners(false, light->outerAngle() * 2.0f,
                                                                      1.0f, position, q, zNear, zFar));
    // Draw in the depth buffer from position of the light source
    for(auto it : filter) {
        it->draw(*buffer, CommandBuffer::SHADOWCAST);
    }
    buffer->resetViewProjection();

    auto instance = light->material();
    if(instance) {
        Vector3 direction(q * Vector3(0.0f, 0.0f, 1.0f));

        instance->setVector3(uniPosition,  &position);
        instance->setVector3(uniDirection, &direction);

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
        if(tile->second.second.size() == count) {
            for(uint32_t i = 0; i < count; i++) {
                AtlasNode *node = tile->second.second[i];
                x[i] = node->x;
                y[i] = node->y;
                w[i] = node->w;
                h[i] = node->h;
                node->dirty = false;
            }
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
