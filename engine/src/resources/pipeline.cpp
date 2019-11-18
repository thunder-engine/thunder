#include "pipeline.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/scene.h"
#include "components/camera.h"
#include "components/renderable.h"
#include "components/directlight.h"
#include "components/postprocesssettings.h"

#include "resources/mesh.h"
#include "resources/material.h"
#include "resources/rendertexture.h"
#include "resources/atlas.h"

#include "analytics/profiler.h"

#include "postprocess/ambientocclusion.h"
#include "postprocess/antialiasing.h"
#include "postprocess/bloom.h"

#include "log.h"

#include "commandbuffer.h"

#include <algorithm>

#include <float.h>

#define G_NORMALS   "normalsMap"
#define G_DIFFUSE   "diffuseMap"
#define G_PARAMS    "paramsMap"
#define G_EMISSIVE  "emissiveMap"

#define DEPTH_MAP   "depthMap"

#define SSAO_MAP    "ssaoMap"

#define OVERRIDE "uni.texture0"

int32_t Pipeline::m_ShadowPageWidth = 1024;
int32_t Pipeline::m_ShadowPageHeight = 1024;

Pipeline::Pipeline() :
        m_Buffer(nullptr),
        m_Screen(Vector2(64, 64)),
        m_pSprite(nullptr),
        m_Target(0),
        m_pFinal(nullptr) {

    m_Buffer = Engine::objectCreate<ICommandBuffer>();

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultSprite.mtl");
    if(mtl) {
        m_pSprite = mtl->createInstance();
    }

    m_pPlane = Engine::loadResource<Mesh>(".embedded/plane.fbx/Plane001");

    m_Buffer->setGlobalValue("light.map", Vector4(1.0f / m_ShadowPageWidth, 1.0f / m_ShadowPageHeight, m_ShadowPageWidth, m_ShadowPageHeight));

    RenderTexture *depth    = Engine::objectCreate<RenderTexture>();
    depth->setDepth(24);
    m_Targets[DEPTH_MAP]    = depth;
    m_Buffer->setGlobalTexture(DEPTH_MAP,   depth);

    RenderTexture *normals  = Engine::objectCreate<RenderTexture>();
    normals->setTarget(Texture::RGB10A2);
    m_Targets[G_NORMALS]    = normals;
    m_Buffer->setGlobalTexture(G_NORMALS,   normals);

    RenderTexture *diffuse  = Engine::objectCreate<RenderTexture>();
    diffuse->setTarget(Texture::RGBA8);
    m_Targets[G_DIFFUSE]    = diffuse;
    m_Buffer->setGlobalTexture(G_DIFFUSE,   diffuse);

    RenderTexture *params   = Engine::objectCreate<RenderTexture>();
    params->setTarget(Texture::RGBA8);
    m_Targets[G_PARAMS]     = params;
    m_Buffer->setGlobalTexture(G_PARAMS,    params);

    RenderTexture *emissive = Engine::objectCreate<RenderTexture>();
    emissive->setTarget(Texture::R11G11B10Float);
    m_Targets[G_EMISSIVE]   = emissive;
    m_Buffer->setGlobalTexture(G_EMISSIVE,  emissive);

    m_OpaqEffects.push_back(new AmbientOcclusion());

    m_PostEffects.push_back(new AntiAliasing());
    //m_PostEffects.push_back(new Bloom());
}

Pipeline::~Pipeline() {
    m_Targets.clear();
}

void Pipeline::draw(Camera &camera) {
    updateShadows(camera, m_Filter);

    m_Buffer->setViewport(0, 0, static_cast<int32_t>(m_Screen.x), static_cast<int32_t>(m_Screen.y));

    // Step1 - Fill G buffer pass draw opaque geometry
    m_Buffer->setRenderTarget({m_Targets[G_NORMALS], m_Targets[G_DIFFUSE], m_Targets[G_PARAMS], m_Targets[G_EMISSIVE]}, m_Targets[DEPTH_MAP]);
    m_Buffer->clearRenderTarget(true, camera.color());

    cameraReset(camera);
    drawComponents(ICommandBuffer::DEFAULT, m_Filter);

    // Screen Space Ambient Occlusion
    m_Buffer->setScreenProjection();
    m_Targets[SSAO_MAP] = m_OpaqEffects.front()->draw(m_Targets[G_EMISSIVE], *m_Buffer);
    m_Buffer->resetViewProjection();

    // Step2 - Light pass
    m_Buffer->setRenderTarget({m_Targets[G_EMISSIVE]}, m_Targets[DEPTH_MAP]);
    drawComponents(ICommandBuffer::LIGHT, m_Filter);

    // Step3 - Draw Transparent pass
    drawComponents(ICommandBuffer::TRANSLUCENT, m_Filter);

    // Step4 - Post Processing passes
    m_Buffer->setScreenProjection();
    m_pFinal = postProcess(m_Targets[G_EMISSIVE]);
}

void Pipeline::finish() {
    m_Buffer->setScreenProjection();
    m_Buffer->setRenderTarget(m_Target);
    m_Buffer->clearRenderTarget();

    m_pSprite->setTexture(OVERRIDE, m_pFinal);
    m_Buffer->drawMesh(Matrix4(), m_pPlane, ICommandBuffer::UI, m_pSprite);
}

void Pipeline::cameraReset(Camera &camera) {
    Matrix4 v, p;
    camera.matrices(v, p);
    camera.setRatio(m_Screen.x / m_Screen.y);
    m_Buffer->setGlobalValue("camera.position", Vector4(camera.actor()->transform()->worldPosition(), camera.nearPlane()));
    m_Buffer->setGlobalValue("camera.target", Vector4(Vector3(), camera.farPlane()));

    m_Buffer->setGlobalValue("camera.mvpi", (p * v).inverse());
    m_Buffer->setGlobalValue("camera.proj", p);
    m_Buffer->setViewProjection(v, p);
}

RenderTexture *Pipeline::target(const string &target) const {
    auto it = m_Targets.find(target);
    if(it != m_Targets.end()) {
        return it->second;
    }
    return nullptr;
}

void Pipeline::resize(int32_t width, int32_t height) {
    m_Screen = Vector2(width, height);

    for(auto &it : m_Targets) {
        it.second->resize(width, height);
    }
    for(auto &it : m_PostEffects) {
        it->resize(width, height);
    }
    for(auto &it : m_OpaqEffects) {
        it->resize(width, height);
    }
    m_Buffer->setGlobalValue("camera.screen", Vector4(1.0f / m_Screen.x, 1.0f / m_Screen.y, m_Screen.x, m_Screen.y));
}

void Pipeline::combineComponents(Object *object) {
    for(auto &it : object->getChildren()) {
        Object *child = it;
        Renderable *comp = dynamic_cast<Renderable *>(child);
        if(comp) {
            if(comp->isEnabled()) {
                m_Components.push_back(comp);
            }
        } else {
            PostProcessSettings *settings = dynamic_cast<PostProcessSettings *>(child);
            if(settings) {
                m_PostProcessSettings.push_back(settings);
            } else {
                Actor *actor = dynamic_cast<Actor *>(child);
                if(actor && !actor->isEnabled()) {
                    continue;
                }
                combineComponents(child);
            }
        }
    }
}

void Pipeline::analizeScene(Scene *scene) {
    m_Components.clear();
    m_Filter.clear();
    m_PostProcessSettings.clear();

    combineComponents(scene);

    Camera *camera = Camera::current();
    m_Filter = Camera::frustumCulling(m_Components, Camera::frustumCorners(*camera));
    sortByDistance(m_Filter, camera->actor()->transform()->position());

    if(!m_PostProcessSettings.empty()) {
        PostProcessSettings *settings = m_PostProcessSettings.front();

        m_Buffer->setGlobalValue("light.ambient", settings->ambientLightIntensity());

        for(auto &it : m_PostEffects) {
            it->setSettings(*settings);
        }
        for(auto &it : m_OpaqEffects) {
            it->setSettings(*settings);
        }
    }
}

void Pipeline::setTarget(uint32_t resource) {
    m_Target = resource;
}

RenderTexture *Pipeline::requestShadowTiles(uint32_t id, uint32_t lod, int32_t *x, int32_t *y, int32_t *w, int32_t *h, uint32_t count) {
    auto tile = m_Tiles.find(id);
    if(tile != m_Tiles.end()) {
        if(tile->second.second.size() == count) {
            for(uint32_t i = 0; i < count; i++) {
                PackNode *node = tile->second.second[i];
                x[i] = node->x;
                y[i] = node->y;
                w[i] = node->w;
                h[i] = node->h;
                node->fill = true;
            }
        }
        return tile->second.first;
    }

    int32_t width = SM_RESOLUTION_DEFAULT / (lod + 1);
    int32_t height = SM_RESOLUTION_DEFAULT / (lod + 1);

    uint32_t columns = MAX(count / 2, 1);
    uint32_t rows = count / columns;

    RenderTexture *target = nullptr;
    PackNode *sub = nullptr;

    for(auto page : m_ShadowPages) {
        target = page.first;
        PackNode *root = page.second;

        sub = root->insert(width * columns, height * rows);
        if(sub) {
            break;
        }
    }

    if(sub == nullptr) {
        target = Engine::objectCreate<RenderTexture>();
        target->setDepth(24);
        target->resize(m_ShadowPageWidth, m_ShadowPageHeight);
        target->setFixed(true);

        PackNode *root = new PackNode;

        root->w = m_ShadowPageWidth;
        root->h = m_ShadowPageHeight;

        m_ShadowPages[target] = root;

        sub = root->insert(width * columns, height * rows);
    }

    vector<PackNode *> tiles;
    for(uint32_t i = 0; i < count; i++) {
        PackNode *node = sub->insert(width, height);
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
        m_Tiles[id] = pair<RenderTexture *, vector<PackNode *>>(target, tiles);
    }
    return target;
}

ICommandBuffer *Pipeline::buffer() const {
    return m_Buffer;
}

void Pipeline::shadowPageSize(int32_t &width, int32_t &height) {
    width = m_ShadowPageWidth;
    height = m_ShadowPageHeight;
}

void Pipeline::setShadowPageSize(int32_t width, int32_t height) {
    m_ShadowPageWidth = width;
    m_ShadowPageHeight = height;
}

void Pipeline::drawComponents(uint32_t layer, ObjectList &list) {
    for(auto it : list) {
        static_cast<Renderable *>(it)->draw(*m_Buffer, layer);
    }
}

void Pipeline::updateShadows(Camera &camera, ObjectList &list) {
    {
        for(auto &tile : m_Tiles) {
            for(auto &it : tile.second.second) {
                it->fill = false;
            }
        }
    }
    {
        for(auto &it : list) {
            BaseLight *light = dynamic_cast<BaseLight *>(it);
            if(light) {
                light->shadowsUpdate(camera, this, m_Components);
            }
        }
    }
    {
        for(auto tiles = m_Tiles.begin(); tiles != m_Tiles.end(); ) {
            bool outdate = false;
            for(auto &it : tiles->second.second) {
                if(it->fill == false) {
                    outdate = true;
                    break;
                }
            }
            if(outdate) {
                tiles = m_Tiles.erase(tiles);
            } else {
                ++tiles;
            }
        }
        /// \todo This activity leads to crash
        //for(auto &it : m_ShadowPages) {
        //    it.second->clean();
        //}
    }
}

RenderTexture *Pipeline::postProcess(RenderTexture *source) {
    m_Buffer->setScreenProjection();
    RenderTexture *result  = source;
    for(auto it : m_PostEffects) {
        result = it->draw(result, *m_Buffer);
    }
    m_Buffer->resetViewProjection();
    return result;
}

RenderTexture *Pipeline::opacProcess(RenderTexture *source) {
    m_Buffer->setScreenProjection();
    RenderTexture *result  = source;
    for(auto it : m_OpaqEffects) {
        result = it->draw(result, *m_Buffer);
    }
    m_Buffer->resetViewProjection();
    return result;
}

struct ObjectComp {
    bool operator() (const Object *left, const Object *right) {
        if(dynamic_cast<const DirectLight *>(left)) {
            return false;
        } else {
            Matrix4 m1 = static_cast<const Component *>(left)->actor()->transform()->worldTransform();
            Matrix4 m2 = static_cast<const Component *>(right)->actor()->transform()->worldTransform();
            return origin.dot(Vector3(m1[12], m1[13], m1[14])) < origin.dot(Vector3(m2[12], m2[13], m2[14]));
        }
    }
    Vector3 origin;
};

void Pipeline::sortByDistance(ObjectList &in, const Vector3 &origin) {
    ObjectComp comp;
    comp.origin = origin;

    in.sort(comp);
}
