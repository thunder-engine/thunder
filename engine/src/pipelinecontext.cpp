#include "pipelinecontext.h"

#include "systems/rendersystem.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/scenegraph.h"
#include "components/camera.h"
#include "components/renderable.h"
#include "components/directlight.h"
#include "components/postprocessvolume.h"

#include "components/private/postprocessorsettings.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertarget.h"
#include "resources/atlas.h"

#include "analytics/profiler.h"

#include "postprocess/ambientocclusion.h"
#include "postprocess/antialiasing.h"
#include "postprocess/reflections.h"
#include "postprocess/bloom.h"

#include "log.h"
#include "timer.h"

#include "commandbuffer.h"

#include <algorithm>

#include <float.h>

#define G_NORMALS   "normalsMap"
#define G_DIFFUSE   "diffuseMap"
#define G_PARAMS    "paramsMap"
#define G_EMISSIVE  "emissiveMap"

#define DEPTH_MAP   "depthMap"

#define GBUFFER     "gBuffer"
#define LIGHPASS    "lightPass"

#define OVERRIDE "texture0"

bool typeLessThan(PostProcessVolume *left, PostProcessVolume *right) {
    return left->priority() < right->priority();
}

PipelineContext::PipelineContext() :
        m_buffer(Engine::objectCreate<CommandBuffer>()),
        m_finalMaterial(nullptr),
        m_effectMaterial(nullptr),
        m_defaultTarget(Engine::objectCreate<RenderTarget>()),
        m_width(64),
        m_height(64),
        m_final(nullptr),
        m_debugTexture(nullptr),
        m_system(nullptr) {

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
    if(mtl) {
        m_finalMaterial = mtl->createInstance();
        m_effectMaterial = mtl->createInstance();
    }

    m_plane = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    int32_t pageWidth, pageHeight;
    RenderSystem::atlasPageSize(pageWidth, pageHeight);
    m_buffer->setGlobalValue("light.pageSize", Vector4(1.0f / pageWidth, 1.0f / pageHeight, pageWidth, pageHeight));

    {
        Texture *depth = Engine::objectCreate<Texture>(DEPTH_MAP);
        depth->setFormat(Texture::Depth);
        depth->setDepthBits(24);
        depth->resize(2, 2);
        m_textureBuffers[DEPTH_MAP] = depth;
        m_buffer->setGlobalTexture(DEPTH_MAP, depth);
    }
    {
        Texture *normals = Engine::objectCreate<Texture>(G_NORMALS);
        normals->setFormat(Texture::RGB10A2);
        m_textureBuffers[G_NORMALS] = normals;
        m_buffer->setGlobalTexture(G_NORMALS, normals);
    }
    {
        Texture *diffuse = Engine::objectCreate<Texture>(G_DIFFUSE);
        diffuse->setFormat(Texture::RGBA8);
        m_textureBuffers[G_DIFFUSE] = diffuse;
        m_buffer->setGlobalTexture(G_DIFFUSE, diffuse);
    }
    {
        Texture *params = Engine::objectCreate<Texture>(G_PARAMS);
        params->setFormat(Texture::RGBA8);
        m_textureBuffers[G_PARAMS] = params;
        m_buffer->setGlobalTexture(G_PARAMS, params);
    }
    {
        Texture *emissive = Engine::objectCreate<Texture>(G_EMISSIVE);
        emissive->setFormat(Texture::R11G11B10Float);
        m_textureBuffers[G_EMISSIVE] = emissive;
        m_buffer->setGlobalTexture(G_EMISSIVE, emissive);
    }

    RenderTarget *gbuffer = Engine::objectCreate<RenderTarget>(GBUFFER);
    gbuffer->setColorAttachment(0, m_textureBuffers[G_NORMALS]);
    gbuffer->setColorAttachment(1, m_textureBuffers[G_DIFFUSE]);
    gbuffer->setColorAttachment(2, m_textureBuffers[G_PARAMS]);
    gbuffer->setColorAttachment(3, m_textureBuffers[G_EMISSIVE]);
    gbuffer->setDepthAttachment(m_textureBuffers[DEPTH_MAP]);
    m_renderTargets[GBUFFER] = gbuffer;

    RenderTarget *light = Engine::objectCreate<RenderTarget>(LIGHPASS);
    light->setColorAttachment(0, m_textureBuffers[G_EMISSIVE]);
    light->setDepthAttachment(m_textureBuffers[DEPTH_MAP]);
    m_renderTargets[LIGHPASS] = light;

    m_postEffects = { new AmbientOcclusion(), new Reflections(), new AntiAliasing(), new Bloom() };
}

PipelineContext::~PipelineContext() {
    m_textureBuffers.clear();
}

void PipelineContext::draw(Camera &camera) {
    updateShadows(camera);

    m_buffer->setViewport(0, 0, m_width, m_height);

    // Step 1.1 - Fill G buffer pass draw opaque geometry
    m_buffer->setRenderTarget(m_renderTargets[GBUFFER]);
    m_buffer->clearRenderTarget(true, camera.color());

    cameraReset(camera);
    drawRenderers(CommandBuffer::DEFAULT, m_culledComponents);

    // Step 1.2 - Opaque pass post processing
    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::DEFAULT);

    // Step 2.1 - Light pass
    m_buffer->setRenderTarget(m_renderTargets[LIGHPASS]);
    drawRenderers(CommandBuffer::LIGHT, m_sceneLights);

    // Step 2.2 - Light pass post processing
    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::LIGHT);

    // Step 3.1 - Transparent pass
    drawRenderers(CommandBuffer::TRANSLUCENT, m_culledComponents);

    // Step 3.2 - Transparent pass post processing
    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::TRANSLUCENT);
    m_final = m_textureBuffers[G_EMISSIVE];
}

void PipelineContext::drawUi(Camera &camera) {
    A_UNUSED(camera);

    m_buffer->setScreenProjection(0, 0, m_width, m_height);
    drawRenderers(CommandBuffer::UI, m_uiComponents);

    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::UI);
}

void PipelineContext::finish() {
    if(m_debugTexture != nullptr) {
        m_final = m_debugTexture;
    }

    m_buffer->setScreenProjection();
    m_buffer->setRenderTarget(m_defaultTarget);
    m_buffer->clearRenderTarget();

    m_finalMaterial->setTexture(OVERRIDE, m_final);
    m_buffer->drawMesh(Matrix4(), m_plane, 0, CommandBuffer::UI, m_finalMaterial);
}

void PipelineContext::cameraReset(Camera &camera) {
    Matrix4 v = camera.viewMatrix();
    Matrix4 p = camera.projectionMatrix();
    Matrix4 vp = p * v;

    camera.setRatio((float)m_width / (float)m_height);

    Transform *c = camera.actor()->transform();

    m_buffer->setGlobalValue("camera.position", Vector4(c->worldPosition(), camera.nearPlane()));
    m_buffer->setGlobalValue("camera.target", Vector4(c->worldTransform().rotation() * Vector3(0.0f, 0.0f, 1.0f), camera.farPlane()));
    m_buffer->setGlobalValue("camera.view", v);
    m_buffer->setGlobalValue("camera.projectionInv", p.inverse());
    m_buffer->setGlobalValue("camera.projection", p);
    m_buffer->setGlobalValue("camera.screenToWorld", vp.inverse());
    m_buffer->setGlobalValue("camera.worldToScreen", vp);

    m_buffer->setViewProjection(v, p);
}

void PipelineContext::setRenderTarget(const string &name) {
    m_buffer->setRenderTarget(m_renderTargets[name]);
}

Texture *PipelineContext::renderTexture(const string &name) const {
    auto it = m_textureBuffers.find(name);
    if(it != m_textureBuffers.end()) {
        return it->second;
    }
    return nullptr;
}

void PipelineContext::setRenderTexture(const string &name, Texture *texture) {
    m_textureBuffers[name] = texture;
}

RenderTarget *PipelineContext::renderTarget(const string &name) const {
    auto it = m_renderTargets.find(name);
    if(it != m_renderTargets.end()) {
        return it->second;
    }
    return nullptr;
}

void PipelineContext::resize(int32_t width, int32_t height) {
    if(m_width != width || m_height != height) {
        m_width = width;
        m_height = height;

        for(auto &it : m_textureBuffers) {
            if(it.second->isFramebuffer()) {
                it.second->setWidth(width);
                it.second->setHeight(height);
            } else {
                it.second->resize(width, height);
            }
        }
        for(auto &it : m_postEffects) {
            it->resize(width, height);
        }

        m_buffer->setGlobalValue("camera.screen", Vector4(1.0f / (float)m_width, 1.0f / (float)m_height, m_width, m_height));
    }
}

void PipelineContext::analizeScene(SceneGraph *graph, RenderSystem *system) {
    m_system = system;

    m_sceneComponents.clear();
    m_sceneLights.clear();
    m_uiComponents.clear();

    m_postProcessVolume.clear();

    combineComponents(graph, graph->isToBeUpdated());

    Camera *camera = Camera::current();
    Transform *cameraTransform = camera->actor()->transform();
    m_culledComponents = Camera::frustumCulling(m_sceneComponents, Camera::frustumCorners(*camera));
    sortRenderables(m_culledComponents, cameraTransform->position());

    // Post process settings mixer
    PostProcessSettings &settings = graph->finalPostProcessSettings();
    settings.resetDefault();

    //std::sort(m_postProcessVolume.begin(), m_postProcessVolume.end(), typeLessThan);
    for(auto &it : m_postProcessVolume) {
        if(!it->unbound()) {
            if(!it->bound().intersect(cameraTransform->worldPosition(), camera->nearPlane())) {
                continue;
            }
        }
        settings.lerp(it->settings(), it->blendWeight());
    }

    m_buffer->setGlobalValue("light.ambient", 0.1f/*settings.ambientLightIntensity()*/);
    for(auto &it : m_postEffects) {
        it->setSettings(settings);
    }
}

RenderTarget *PipelineContext::defaultTarget() {
    return m_defaultTarget;
}

void PipelineContext::setDefaultTarget(RenderTarget *target) {
    m_defaultTarget = target;
}

Texture *PipelineContext::debugTexture() const {
    return m_debugTexture;
}

void PipelineContext::setDebugTexture(const string &string) {
    m_debugTexture = nullptr;
    auto it = m_textureBuffers.find(string);
    if(it != m_textureBuffers.end()) {
        m_debugTexture = it->second;
    }
}

RenderTarget *PipelineContext::requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count) {
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
        int32_t pageWidth, pageHeight;
        RenderSystem::atlasPageSize(pageWidth, pageHeight);

        Texture *map = Engine::objectCreate<Texture>();
        map->setFormat(Texture::Depth);
        map->setWidth(pageWidth);
        map->setHeight(pageHeight);
        map->setDepthBits(24);

        target = Engine::objectCreate<RenderTarget>();
        target->setDepthAttachment(map);

        AtlasNode *root = new AtlasNode;

        root->w = pageWidth;
        root->h = pageHeight;

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

CommandBuffer *PipelineContext::buffer() const {
    return m_buffer;
}

const list<RenderPass *> &PipelineContext::postEffects() const {
    return m_postEffects;
}

list<string> PipelineContext::renderTextures() const {
    list<string> result;
    for(auto &it : m_textureBuffers) {
        result.push_back(it.first);
    }

    return result;
}

void PipelineContext::drawRenderers(uint32_t layer, const list<Renderable *> &list) {
    for(auto it : list) {
        it->draw(*m_buffer, layer);
    }
}

const list<Renderable *> &PipelineContext::culledComponents() const {
    return m_culledComponents;
}

void PipelineContext::cleanShadowCache() {
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

void PipelineContext::updateShadows(Camera &camera) {
    cleanShadowCache();

    for(auto &it : m_sceneLights) {
        static_cast<BaseLight *>(it)->shadowsUpdate(camera, this, m_sceneComponents);
    }
}

void PipelineContext::postProcess(RenderTarget *source, uint32_t layer) {
    m_buffer->setScreenProjection();
    Texture *result = source->colorAttachment(0);
    for(auto it : m_postEffects) {
        if(it->layer() == layer) {
            result = it->draw(result, this);
        }
    }
    Texture *texture = source->colorAttachment(0);
    if(result != texture) {
        m_buffer->setViewport(0, 0, texture->width(), texture->height());
        m_buffer->setRenderTarget(source);
        m_effectMaterial->setTexture(OVERRIDE, result);
        m_buffer->drawMesh(Matrix4(), m_plane, 0, CommandBuffer::UI, m_effectMaterial);
    }

    m_buffer->resetViewProjection();
}

void PipelineContext::combineComponents(Object *object, bool update) {
    for(auto &it : object->getChildren()) {
        Object *child = it;
        if(child->isComponent()) {
            Component *component = static_cast<Component *>(child);
            if(component->isRenderable()) {
                Renderable *comp = static_cast<Renderable *>(child);
                if(comp->isEnabled() && comp->actor()->isEnabledInHierarchy()) {
                    if(update) {
                        comp->update();
                    }
                    if(comp->isLight()) {
                        m_sceneLights.push_back(comp);
                    } else {
                        if(comp->actor()->layers() & CommandBuffer::UI) {
                            m_uiComponents.push_back(comp);
                        } else {
                            m_sceneComponents.push_back(comp);
                        }
                    }
                }
            } else if(component->isPostProcessVolume()){
                m_postProcessVolume.push_back(static_cast<PostProcessVolume *>(component));
            }
        } else {
            combineComponents(child, update);
        }
    }
}

struct ObjectComp {
    bool operator() (const Renderable *left, const Renderable *right) {
        int p1 = left->priority();
        int p2 = right->priority();
        if(p1 == p2) {
            const Matrix4 &m1 = left->actor()->transform()->worldTransform();
            const Matrix4 &m2 = right->actor()->transform()->worldTransform();

            return origin.dot(Vector3(m1[12], m1[13], m1[14])) < origin.dot(Vector3(m2[12], m2[13], m2[14]));
        }
        return p1 < p2;
    }
    Vector3 origin;
};

void PipelineContext::sortRenderables(list<Renderable *> &in, const Vector3 &origin) {
    ObjectComp comp;
    comp.origin = origin;

    in.sort(comp);
}
