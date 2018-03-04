#include "apipeline.h"

#include <aobject.h>

#include "controller.h"

#include "filters/ablurgl.h"

#include "postprocess/aambientocclusiongl.h"
#include "postprocess/aantialiasinggl.h"
#include "postprocess/abloomgl.h"

#include <components/actor.h>
#include <components/scene.h>
#include <components/component.h>
#include <components/camera.h>

#include "components/adirectlightgl.h"
#include "components/aspritegl.h"

#include "analytics/profiler.h"

#include "resources/ameshgl.h"

#include <log.h>

APipeline::APipeline(Engine *engine) :
        m_pEngine(engine),
        m_pScene(nullptr),
        m_pController(nullptr),
        m_Buffer(new CommandBufferGL()),
        m_Screen(Vector2(64, 64)),
        m_World(Vector3()) {

    //m_pBlur     = new ABlurGL();
    //m_pAO       = new AAmbientOcclusionGL();

    //m_PostEffects.push_back(new AAntiAliasingGL());
    //m_PostEffects.push_back(new ABloomGL());

    m_Select.create  (GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT);
    m_Depth.create  (GL_TEXTURE_2D, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

    m_SelectBuffer  = 0;
    glGenFramebuffers(1, &m_SelectBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_SelectBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Select.id(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_Depth.id(), 0);
}

APipeline::~APipeline() {
    glDeleteFramebuffers(1, &m_SelectBuffer);
}

void APipeline::draw(Scene &scene, uint32_t) {
    Profiler::statReset(POLYGONS);
    Profiler::statReset(DRAWCALLS);

    m_pScene    = &scene;
    m_Buffer->setGlobalValue("light.ambient", m_pScene->ambient());
    // Light prepass
    updateShadows(scene);

    analizeScene(scene);
}

ATextureGL *APipeline::postProcess(ATextureGL &source) {
    ATextureGL *result  = &source;
    for(auto it : m_PostEffects) {
        result  = it->draw(*result, *m_Buffer);
    }
    return result;
}

void APipeline::cameraReset() {
    Camera *camera  = activeCamera();
    if(camera) {
        camera->setRatio(m_Screen.x / m_Screen.y);
        m_Buffer->setGlobalValue("camera.position", Vector4(camera->actor().position(), camera->nearPlane()));
        m_Buffer->setGlobalValue("camera.target", Vector4(Vector3(), camera->farPlane()));
        m_Buffer->setGlobalValue("camera.screen", Vector2(1.0 / m_Screen.x, 1.0 / m_Screen.y));

        Matrix4 v, p;
        camera->matrices(v, p);
        m_Buffer->setViewProjection(v, p);
    }
}

Camera *APipeline::activeCamera() {
    if(m_pController) {
        return m_pController->activeCamera();
    }
    return m_pEngine->controller()->activeCamera();
}

void APipeline::resize(uint32_t width, uint32_t height) {
    glViewport(0, 0, width, height);

    m_Screen    = Vector2(width, height);
    //m_pAO->resize(width, height);

    m_Select.resize(width, height);
    m_Depth.resize(width, height);

    for(auto it : m_PostEffects) {
        it->resize(width, height);
    }
}

void APipeline::drawComponents(AObject &object, uint8_t layer) {
    for(auto &it : object.getChildren()) {
        AObject *child  = it;
        IDrawObject *draw = dynamic_cast<IDrawObject *>(child);
        if(draw) {
            draw->draw(*m_Buffer, layer);
        } else {
            Actor *actor    = dynamic_cast<Actor *>(child);
            if(actor) {
                if(!actor->isEnable()) {
                    continue;
                }
            }
            drawComponents(*child, layer);
            if(actor) {
                m_Buffer->setColor(Vector4(1.0));
            }
        }
    }
}

void APipeline::updateShadows(AObject &object) {
    for(auto &it : object.getChildren()) {
        ADirectLightGL *light = dynamic_cast<ADirectLightGL *>(it);
        if(light) {
            light->shadowsUpdate(*m_Buffer);
        } else {
            updateShadows(*it);
        }
    }
}

void APipeline::updateLights(AObject &object, uint8_t layer) {
    for(auto &it : object.getChildren()) {
        ADirectLightGL *light = dynamic_cast<ADirectLightGL *>(it);
        if(light) {
            light->draw(*m_Buffer, layer);
        } else {
            updateLights(*it, layer);
        }
    }
}

void APipeline::analizeScene(AObject &object) {
    glBindFramebuffer( GL_FRAMEBUFFER, m_SelectBuffer );
    // Retrive object id
    m_Buffer->clearRenderTarget(true, Vector4(0.0));
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    cameraReset();
    drawComponents(object, ICommandBuffer::RAYCAST);

    IController *controller = m_pEngine->controller();
    if(m_pController) {
        controller = m_pController;
    }

    Vector2 position;
    if(controller) {
        Vector2 v;
        controller->selectGeometry(position, v);
    }
    Vector3 screen  = Vector3(position.x / m_Screen.x, position.y / m_Screen.y, 0.0f);
    screen.y        = (1.0 - screen.y);
    glReadPixels((int)screen.x, (int)screen.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screen.z);
    Camera::unproject(screen, m_Buffer->modelView(), m_Buffer->projection(), m_World);
    // Get id
    uint32_t result = 0;
    if(position.x >= 0.0f && position.y >= 0.0f &&
        position.x < m_Screen.x && position.y < m_Screen.y) {

        uint8_t value[4];
        glReadPixels(position.x, (m_Screen.y - position.y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, value);

        result  = value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
    }
    list<uint32_t> l;
    if(result) {
        l.push_back(result);
    }
    controller->setSelectedObjects(l);
}
