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

#include <renderpass.h>

#include <QVariant>
#include <QColor>

namespace {
    const char *gridColor("General/Colors/Grid_Color");
};

EditorPipeline::EditorPipeline() :
        m_controller(nullptr),
        m_grid(nullptr) {

    Material *m = Engine::loadResource<Material>(".embedded/grid.shader");
    if(m) {
        m_grid = m->createInstance();
    }

    SettingsManager::instance()->registerProperty(gridColor, QColor(102, 102, 102, 102));

    QObject::connect(SettingsManager::instance(), &SettingsManager::updated, this, &EditorPipeline::onApplySettings);
}

void EditorPipeline::onApplySettings() {
    QColor color = SettingsManager::instance()->property(gridColor).value<QColor>();
    m_gridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void EditorPipeline::setController(CameraCtrl *ctrl) {
    m_controller = ctrl;
}

void EditorPipeline::draw(Camera &camera) {
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

    renderPass(m_renderTargets["lightPass"], CommandBuffer::UI);
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
