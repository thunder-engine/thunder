#include "pipeline.h"

#include "systems/rendersystem.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/scene.h"
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

Pipeline::Pipeline() :
        m_Buffer(Engine::objectCreate<CommandBuffer>()),
        m_pSprite(nullptr),
        m_pDefaultTarget(Engine::objectCreate<RenderTarget>()),
        m_Width(64),
        m_Height(64),
        m_pFinal(nullptr),
        m_pSystem(nullptr) {

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
    if(mtl) {
        m_pSprite = mtl->createInstance();
    }

    m_pPlane = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    int32_t pageWidth, pageHeight;
    RenderSystem::atlasPageSize(pageWidth, pageHeight);
    m_Buffer->setGlobalValue("light.pageSize", Vector4(1.0f / pageWidth, 1.0f / pageHeight, pageWidth, pageHeight));

    {
        Texture *depth = Engine::objectCreate<Texture>();
        depth->setFormat(Texture::Depth);
        depth->setDepthBits(24);
        m_textureBuffers[DEPTH_MAP] = depth;
        m_Buffer->setGlobalTexture(DEPTH_MAP, depth);
    }
    {
        Texture *normals = Engine::objectCreate<Texture>();
        normals->setFormat(Texture::RGB10A2);
        m_textureBuffers[G_NORMALS] = normals;
        m_Buffer->setGlobalTexture(G_NORMALS, normals);
    }
    {
        Texture *diffuse = Engine::objectCreate<Texture>();
        diffuse->setFormat(Texture::RGBA8);
        m_textureBuffers[G_DIFFUSE] = diffuse;
        m_Buffer->setGlobalTexture(G_DIFFUSE, diffuse);
    }
    {
        Texture *params = Engine::objectCreate<Texture>();
        params->setFormat(Texture::RGBA8);
        m_textureBuffers[G_PARAMS] = params;
        m_Buffer->setGlobalTexture(G_PARAMS, params);
    }
    {
        Texture *emissive = Engine::objectCreate<Texture>();
        emissive->setFormat(Texture::R11G11B10Float);
        m_textureBuffers[G_EMISSIVE] = emissive;
        m_Buffer->setGlobalTexture(G_EMISSIVE, emissive);
    }

    RenderTarget *gbuffer = Engine::objectCreate<RenderTarget>();
    gbuffer->setColorAttachment(0, m_textureBuffers[G_NORMALS]);
    gbuffer->setColorAttachment(1, m_textureBuffers[G_DIFFUSE]);
    gbuffer->setColorAttachment(2, m_textureBuffers[G_PARAMS]);
    gbuffer->setColorAttachment(3, m_textureBuffers[G_EMISSIVE]);
    gbuffer->setDepthAttachment(m_textureBuffers[DEPTH_MAP]);
    m_renderTargets[GBUFFER] = gbuffer;

    RenderTarget *light = Engine::objectCreate<RenderTarget>();
    light->setColorAttachment(0, m_textureBuffers[G_EMISSIVE]);
    light->setDepthAttachment(m_textureBuffers[DEPTH_MAP]);
    m_renderTargets[LIGHPASS] = light;

    m_PostEffects = { new AmbientOcclusion(), new Reflections(), new AntiAliasing(), new Bloom() };
}

Pipeline::~Pipeline() {
    m_textureBuffers.clear();
}

void Pipeline::draw(Camera &camera) {
    updateShadows(camera);

    m_Buffer->setViewport(0, 0, m_Width, m_Height);

    // Step 1.1 - Fill G buffer pass draw opaque geometry
    m_Buffer->setRenderTarget(m_renderTargets[GBUFFER]);
    m_Buffer->clearRenderTarget(true, camera.color());

    cameraReset(camera);
    drawComponents(CommandBuffer::DEFAULT, m_Filter);

    // Step 1.2 - Opaque pass post processing
    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::DEFAULT);

    // Step 2.1 - Light pass
    m_Buffer->setRenderTarget(m_renderTargets[LIGHPASS]);
    drawComponents(CommandBuffer::LIGHT, m_SceneLights);

    // Step 2.2 - Light pass post processing
    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::LIGHT);

    // Step 3.1 - Transparent pass
    drawComponents(CommandBuffer::TRANSLUCENT, m_Filter);

    // Step 3.2 - Transparent pass post processing
    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::TRANSLUCENT);
    m_pFinal = m_textureBuffers[G_EMISSIVE];

    drawUi(camera);
}

void Pipeline::drawUi(Camera &camera) {
    A_UNUSED(camera);

    m_Buffer->setViewProjection(Matrix4(), Matrix4::ortho(0, m_Width, 0, m_Height, 0.0f, 500.0f));
    drawComponents(CommandBuffer::UI, m_UiComponents);

    postProcess(m_renderTargets[LIGHPASS], CommandBuffer::UI);
}

void Pipeline::finish() {
    m_Buffer->setScreenProjection();
    m_Buffer->setRenderTarget(m_pDefaultTarget);
    m_Buffer->clearRenderTarget();

    m_pSprite->setTexture(OVERRIDE, m_pFinal);
    m_Buffer->drawMesh(Matrix4(), m_pPlane, 0, CommandBuffer::UI, m_pSprite);
}

void Pipeline::cameraReset(Camera &camera) {
    Matrix4 v = camera.viewMatrix();
    Matrix4 p = camera.projectionMatrix();
    camera.setRatio((float)m_Width / (float)m_Height);

    Transform *c = camera.actor()->transform();

    m_Buffer->setGlobalValue("camera.position", Vector4(c->worldPosition(), camera.nearPlane()));
    m_Buffer->setGlobalValue("camera.target", Vector4(c->worldTransform().rotation() * Vector3(0.0f, 0.0f, 1.0f), camera.farPlane()));

    Matrix4 vp = p * v;

    m_Buffer->setGlobalValue("camera.view", v);
    m_Buffer->setGlobalValue("camera.projectionInv", p.inverse());
    m_Buffer->setGlobalValue("camera.projection", p);
    m_Buffer->setGlobalValue("camera.screenToWorld", vp.inverse());
    m_Buffer->setGlobalValue("camera.worldToScreen", vp);

    m_Buffer->setViewProjection(v, p);
}

Texture *Pipeline::renderTexture(const string &name) const {
    auto it = m_textureBuffers.find(name);
    if(it != m_textureBuffers.end()) {
        return it->second;
    }
    return nullptr;
}

void Pipeline::setRenderTexture(const string &name, Texture *texture) {
    m_textureBuffers[name] = texture;
}

void Pipeline::resize(int32_t width, int32_t height) {
    if(m_Width != width || m_Height != height) {
        m_Width = width;
        m_Height = height;

        for(auto &it : m_textureBuffers) {
            it.second->setWidth(width);
            it.second->setHeight(height);
        }
        for(auto &it : m_PostEffects) {
            it->resize(width, height);
        }

        m_Buffer->setGlobalValue("camera.screen", Vector4(1.0f / (float)m_Width, 1.0f / (float)m_Height, m_Width, m_Height));
    }
}

int Pipeline::screenWidth() const {
    return m_Width;
}

int Pipeline::screenHeight() const {
    return m_Height;
}

void Pipeline::analizeScene(Scene *scene, RenderSystem *system) {
    m_pSystem = system;

    m_SceneComponents.clear();
    m_SceneLights.clear();
    m_UiComponents.clear();

    m_postProcessVolume.clear();

    combineComponents(scene, scene->isToBeUpdated());

    Camera *camera = Camera::current();
    m_Filter = Camera::frustumCulling(m_SceneComponents, Camera::frustumCorners(*camera));
    sortByDistance(m_Filter, camera->actor()->transform()->position());

    // Post process settings mixer
    PostProcessSettings &settings = scene->finalPostProcessSettings();
    settings.resetDefault();

    //std::sort(m_postProcessVolume.begin(), m_postProcessVolume.end(), typeLessThan);
    for(auto &it : m_postProcessVolume) {
        if(!it->unbound()) {
            Actor *a = camera->actor();
            if(a) {
                Transform *t = a->transform();
                if(!it->bound().intersect(t->worldPosition(), camera->nearPlane())) {
                    continue;
                }
            }
        }
        settings.lerp(it->settings(), it->blendWeight());
    }

    m_Buffer->setGlobalValue("light.ambient", 0.1f/*settings.ambientLightIntensity()*/);
    for(auto &it : m_PostEffects) {
        it->setSettings(settings);
    }
}

RenderTarget *Pipeline::defaultTarget() {
    return m_pDefaultTarget;
}

void Pipeline::setDefaultTarget(RenderTarget *target) {
    m_pDefaultTarget = target;
}

RenderTarget *Pipeline::requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count) {
    auto tile = m_Tiles.find(id);
    if(tile != m_Tiles.end()) {
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

    for(auto page : m_ShadowPages) {
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

        m_ShadowPages[target] = root;

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
        m_Tiles[id] = make_pair(target, tiles);
    }
    return target;
}

CommandBuffer *Pipeline::buffer() const {
    return m_Buffer;
}

void Pipeline::drawComponents(uint32_t layer, list<Renderable *> &list) {
    for(auto it : list) {
        it->draw(*m_Buffer, layer);
    }
}

void Pipeline::cleanShadowCache() {
    for(auto tiles = m_Tiles.begin(); tiles != m_Tiles.end(); ) {
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
            tiles = m_Tiles.erase(tiles);
        } else {
            ++tiles;
        }
    }
    /// \todo This activity leads to crash
    //for(auto &it : m_ShadowPages) {
    //    it.second->clean();
    //}

    for(auto &tile : m_Tiles) {
        for(auto &it : tile.second.second) {
            it->dirty = true;
        }
    }
}

void Pipeline::updateShadows(Camera &camera) {
    cleanShadowCache();

    for(auto &it : m_SceneLights) {
        static_cast<BaseLight *>(it)->shadowsUpdate(camera, this, m_SceneComponents);
    }
}

void Pipeline::postProcess(RenderTarget *source, uint32_t layer) {
    m_Buffer->setScreenProjection();
    Texture *result = source->colorAttachment(0);
    for(auto it : m_PostEffects) {
        if(it->layer() == layer) {
            result = it->draw(result, this);
        }
    }
    Texture *texture = source->colorAttachment(0);
    if(result != texture) {
        m_Buffer->setViewport(0, 0, texture->width(), texture->height());
        m_Buffer->setRenderTarget(source);
        m_pSprite->setTexture(OVERRIDE, result);
        m_Buffer->drawMesh(Matrix4(), m_pPlane, 0, CommandBuffer::UI, m_pSprite);
    }

    m_Buffer->resetViewProjection();
}

void Pipeline::combineComponents(Object *object, bool update) {
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
                        m_SceneLights.push_back(comp);
                    } else {
                        if(comp->actor()->layers() & CommandBuffer::UI) {
                            m_UiComponents.push_back(comp);
                        } else {
                            m_SceneComponents.push_back(comp);
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
        Matrix4 m1 = left->actor()->transform()->worldTransform();
        Matrix4 m2 = right->actor()->transform()->worldTransform();
        return origin.dot(Vector3(m1[12], m1[13], m1[14])) < origin.dot(Vector3(m2[12], m2[13], m2[14]));
    }
    Vector3 origin;
};

void Pipeline::sortByDistance(list<Renderable *> &in, const Vector3 &origin) {
    ObjectComp comp;
    comp.origin = origin;

    in.sort(comp);
}
