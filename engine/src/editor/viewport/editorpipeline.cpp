#include "editor/viewport/editorpipeline.h"

#include "editor/viewport/handles.h"
#include "editor/viewport/cameractrl.h"

#include "editor/settingsmanager.h"

#include <commandbuffer.h>

#include <components/camera.h>
#include <components/actor.h>
#include <components/transform.h>
#include <components/renderable.h>

#include <resources/rendertarget.h>
#include <resources/material.h>
#include <resources/mesh.h>

#include <postprocess/renderpass.h>

#include <QVariant>
#include <QColor>

#define SELECT_MAP   "selectMap"
#define DEPTH_MAP    "depthMap"
#define OUTLINE_MAP  "outlineMap"
#define OUTDEPTH_MAP "outdepthMap"

#define SEL_TARGET   "objectSelect"
#define OUT_TARGET   "outLine"

#define OUTLINE      "Outline"

namespace {
    const char *gridColor("General/Colors/Grid_Color");
    const char *outlineWidth("General/Colors/Outline_Width");
    const char *outlineColor("General/Colors/Outline_Color");
};

class Outline : public RenderPass {
public:
    Outline() :
        m_width(1.0f) {

        m_resultTexture = Engine::objectCreate<Texture>();
        m_resultTexture->setFormat(Texture::RGBA8);

        m_resultTarget->setColorAttachment(0, m_resultTexture);

        m_outlineDepth = Engine::objectCreate<Texture>();
        m_outlineDepth->setFormat(Texture::Depth);
        m_outlineDepth->setDepthBits(24);

        m_outlineMap = Engine::objectCreate<Texture>();
        m_outlineMap->setFormat(Texture::RGBA8);

        m_outlineTarget = Engine::objectCreate<RenderTarget>();
        m_outlineTarget->setColorAttachment(0, m_outlineMap);
        m_outlineTarget->setDepthAttachment(m_outlineDepth);

        Material *material = Engine::loadResource<Material>(".embedded/outline.shader");
        if(material) {
            m_material = material->createInstance();
            m_material->setTexture("outlineMap", m_outlineMap);
        }

        SettingsManager::instance()->registerProperty(outlineWidth, 1.0f);
        SettingsManager::instance()->registerProperty(outlineColor, QColor(255, 128, 0, 255));
    }

    Texture *draw(Texture *source, PipelineContext *context) override {
        if(m_enabled && m_material) {
            m_material->setTexture("rgbMap", source);

            CommandBuffer *buffer = context->buffer();

            buffer->resetViewProjection();
            buffer->setRenderTarget(m_outlineTarget);
            buffer->clearRenderTarget();
            RenderList filter;
            for(auto actor : m_controller->selected()) {
                for(auto it : context->culledComponents()) {
                    Renderable *component = dynamic_cast<Renderable *>(it);
                    if(component && component->actor()->isInHierarchy(static_cast<Actor *>(actor))) {
                        filter.push_back(component);
                    }
                }
            }
            context->drawRenderers(CommandBuffer::RAYCAST, filter);

            buffer->setScreenProjection();

            buffer->setRenderTarget(m_resultTarget);
            buffer->drawMesh(Matrix4(), m_mesh, 0, CommandBuffer::UI, m_material);

            return m_resultTexture;
        }

        return source;
    }

    void resize(int32_t width, int32_t height) override {
        m_outlineMap->setWidth(width);
        m_outlineMap->setHeight(height);

        m_outlineDepth->setWidth(width);
        m_outlineDepth->setHeight(height);

        RenderPass::resize(width, height);
    }

    const char *name() const override {
        return "Outline";
    }

    void loadSettings() {
        QColor color = SettingsManager::instance()->property(qPrintable(outlineColor)).value<QColor>();
        m_color = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        m_width = SettingsManager::instance()->property(qPrintable(outlineWidth)).toFloat();

        if(m_material) {
            m_material->setFloat("uni.width", &m_width);
            m_material->setVector4("uni.color", &m_color);
        }
    }

    void setController(CameraCtrl *controller) {
        m_controller = controller;
    }

protected:
    float m_width;

    Vector4 m_color;

    Texture *m_outlineMap;
    Texture *m_outlineDepth;

    RenderTarget *m_outlineTarget;

    CameraCtrl *m_controller;

};

EditorPipeline::EditorPipeline() :
        m_controller(nullptr),
        m_outline(new Outline()),
        m_grid(nullptr) {

    {
        Texture *select = Engine::objectCreate<Texture>(SELECT_MAP);
        select->setFormat(Texture::RGBA8);
        select->resize(2, 2);
        m_textureBuffers[SELECT_MAP] = select;
        m_buffer->setGlobalTexture(SELECT_MAP, select);
    }


    RenderTarget *object = Engine::objectCreate<RenderTarget>(SEL_TARGET);
    object->setColorAttachment(0, m_textureBuffers[SELECT_MAP]);
    object->setDepthAttachment(m_textureBuffers[DEPTH_MAP]);
    m_renderTargets[SEL_TARGET] = object;

    m_postEffects.push_back(m_outline);

    Handles::init();

    Material *m = Engine::loadResource<Material>(".embedded/grid.shader");
    if(m) {
        m_grid = m->createInstance();
    }

    SettingsManager::instance()->registerProperty(gridColor, QColor(102, 102, 102, 102));

    QObject::connect(SettingsManager::instance(), &SettingsManager::updated, this, &EditorPipeline::onApplySettings);
}

void EditorPipeline::onApplySettings() {
    m_outline->loadSettings();

    QColor color = SettingsManager::instance()->property(gridColor).value<QColor>();
    m_gridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void EditorPipeline::setController(CameraCtrl *ctrl) {
    m_controller = ctrl;
    m_outline->setController(m_controller);
}

void EditorPipeline::setDragObjects(const ObjectList &list) {
    m_dragList.clear();
    for(auto it : list) {
        auto result = it->findChildren<Renderable *>();

        m_dragList.insert(m_dragList.end(), result.begin(), result.end());
    }
}

void EditorPipeline::draw(Camera &camera) {
    // Retrive object id
    m_buffer->setRenderTarget(m_renderTargets[SEL_TARGET]);
    m_buffer->clearRenderTarget();

    m_buffer->setViewport(0, 0, m_width, m_height);

    cameraReset(camera);
    for(auto it : m_culledComponents) {
        if(it->actor()->hideFlags() & Actor::SELECTABLE) {
            it->draw(*m_buffer, CommandBuffer::RAYCAST);
        }
    }
    for(auto it : m_uiComponents) {
        if(it->actor()->hideFlags() & Actor::SELECTABLE) {
            it->draw(*m_buffer, CommandBuffer::RAYCAST);
        }
    }

    for(auto it : m_dragList) {
        it->update();
        m_culledComponents.push_back(it);
    }

    PipelineContext::draw(camera);

    if(debugTexture() == nullptr) {
        // Draw handles
        m_buffer->resetViewProjection();
        drawGrid(*Camera::current());

        Handles::beginDraw(m_buffer);
        if(m_controller) {
            m_controller->drawHandles();
        }
        Handles::endDraw();
    }
}

void EditorPipeline::drawUi(Camera &camera) {
    cameraReset(camera);
    drawRenderers(CommandBuffer::UI, m_uiComponents);

    postProcess(m_renderTargets["lightPass"], CommandBuffer::UI);
}

void EditorPipeline::drawGrid(Camera &camera) {
    Transform *t = camera.actor()->transform();
    Vector3 cam = t->position();
    Vector3 pos(cam.x, 0.0f, cam.z);

    Quaternion rot;

    float scale = 1.0f;
    float width = 0.5f;

    bool ortho = camera.orthographic();
    if(ortho) {
        float length = camera.orthoSize() * 10.0f;

        scale = 0.01f;
        while(scale < length) {
            scale *= 10.0f;
        }

        width = length / scale;

        float depth = camera.farPlane() - camera.nearPlane();
        CameraCtrl::ViewSide side = m_controller->viewSide();
        switch(side) {
            case CameraCtrl::ViewSide::VIEW_FRONT:
            case CameraCtrl::ViewSide::VIEW_BACK: {
                rot = Quaternion();
                pos = Vector3(cam.x, cam.y, cam.z + ((side == CameraCtrl::ViewSide::VIEW_FRONT) ? -depth : depth));
                pos = Vector3(scale * int32_t(pos.x / scale),
                              scale * int32_t(pos.y / scale),
                              pos.z);
            } break;
            case CameraCtrl::ViewSide::VIEW_LEFT:
            case CameraCtrl::ViewSide::VIEW_RIGHT: {
                rot = Quaternion(Vector3(0, 1, 0), 90.0f);
                pos = Vector3(cam.x + ((side == CameraCtrl::ViewSide::VIEW_LEFT) ? depth : -depth), cam.y, cam.z);
                pos = Vector3(pos.x,
                              scale * int32_t(pos.y / scale),
                              scale * int32_t(pos.z / scale));
            } break;
            case CameraCtrl::ViewSide::VIEW_TOP:
            case CameraCtrl::ViewSide::VIEW_BOTTOM: {
                rot = Quaternion(Vector3(1, 0, 0), 90.0f);
                pos = Vector3(cam.x, cam.y + ((side == CameraCtrl::ViewSide::VIEW_TOP) ? -depth : depth), cam.z);
                pos = Vector3(scale * int32_t(pos.x / scale),
                              pos.y,
                              scale * int32_t(pos.z / scale));
            } break;
            default: break;
        }
    } else {
        scale = 100.0f;

        pos = Vector3(scale * int32_t(pos.x / scale),
                      0.0f,
                      scale * int32_t(pos.z / scale));

        rot = Quaternion(Vector3(1, 0, 0), 90.0f);
    }

    m_grid->setBool("uni.ortho", &ortho);
    m_grid->setFloat("uni.scale", &scale);
    m_grid->setFloat("uni.width", &width);

    m_buffer->setColor(m_gridColor);
    m_buffer->drawMesh(Matrix4(pos, rot, scale), m_plane, 0, CommandBuffer::TRANSLUCENT, m_grid);
    m_buffer->setColor(Vector4(1.0f));
}
