#include "objectctrlpipeline.h"

#include "rendertexture.h"
#include "commandbuffer.h"

#include "components/camera.h"
#include "components/actor.h"
#include "components/transform.h"
#include "components/scene.h"

#include "components/renderable.h"

#include "resources/material.h"
#include "resources/mesh.h"

#include "settingsmanager.h"

#include "objectctrl.h"

#include <handles/handletools.h>

#include <QVariant>
#include <QColor>

#define SELECT_MAP  "selectMap"
#define DEPTH_MAP   "depthMap"

ObjectCtrlPipeline::ObjectCtrlPipeline() :
        Pipeline() {
    RenderTexture *select   = Engine::objectCreate<RenderTexture>();
    select->setTarget(Texture::RGBA8);
    m_Targets[SELECT_MAP]   = select;
    m_Buffer->setGlobalTexture(SELECT_MAP,  select);

    m_pGrid  = Engine::objectCreate<Mesh>("Grid");

    Mesh::Lod lod;
    lod.vertices.resize(404);
    lod.indices.resize(404);
    for(uint8_t x = 0; x <= 100; x++) {
        uint32_t index = x * 2;
        lod.vertices[index] = Vector3(x - 50, -50, 0);
        lod.vertices[index + 1] = Vector3(x - 50, 50, 0);

        lod.indices[index] = index;
        lod.indices[index + 1] = index + 1;
    }
    for(uint8_t y = 0; y <= 100; y++) {
        uint32_t index = y * 2 + 202;
        lod.vertices[index] = Vector3(-50, y - 50, 0);
        lod.vertices[index + 1] = Vector3(50, y - 50, 0);

        lod.indices[index] = index;
        lod.indices[index + 1] = index + 1;
    }

    m_pGrid->setMode(Mesh::MODE_LINES);
    m_pGrid->addLod(lod);

    Material *m = Engine::loadResource<Material>(".embedded/gizmo.mtl");
    if(m) {
        m_pGizmo = m->createInstance();
    }

    loadSettings();
}

void ObjectCtrlPipeline::loadSettings() {
    QColor color = SettingsManager::instance()->property("General/Colors/Grid_Color").value<QColor>();
    m_SecondaryGridColor = m_PrimaryGridColor = Vector4(color.redF(), color.greenF(), color.blueF(), color.alphaF());
}

void ObjectCtrlPipeline::setController(ObjectCtrl *ctrl) {
    m_pController = ctrl;
}

void ObjectCtrlPipeline::draw(Camera &camera) {
    // Retrive object id
    m_Buffer->setRenderTarget({m_Targets[SELECT_MAP]}, m_Targets[DEPTH_MAP]);
    m_Buffer->clearRenderTarget(true, Vector4(0.0));

    m_Buffer->setViewport(0, 0, static_cast<int32_t>(m_Screen.x), static_cast<int32_t>(m_Screen.y));

    cameraReset(camera);
    drawComponents(ICommandBuffer::RAYCAST, m_Filter);

    Pipeline::draw(camera);

    cameraReset(camera);
    m_Buffer->setRenderTarget({m_pFinal}, m_Targets[DEPTH_MAP]);

    drawGrid(camera);

    Handles::beginDraw(m_Buffer);
    m_pController->drawHandles();
    Handles::endDraw();
}

void ObjectCtrlPipeline::resize(int32_t width, int32_t height) {
    Pipeline::resize(width, height);
    m_pController->resize(width, height);
}

void ObjectCtrlPipeline::drawGrid(Camera &camera) {
    Transform *t = camera.actor()->transform();
    Vector3 cam = t->position();
    Vector3 pos;
    float length;

    if(camera.orthographic()) {
        pos = Vector3(cam.x, cam.y, 0.0);
        length = camera.orthoSize();
    } else {
        pos = Vector3(cam.x, 0.0f, cam.z);
        length = (pos - cam).length();
    }

    float scale = 0.001f;
    while(scale < length) {
        scale *= 10.0f;
    }
    m_SecondaryGridColor.w = m_PrimaryGridColor.w * (1.0f - (length / scale));

    Matrix4 transform(Vector3(scale * int32_t(pos.x / scale), 0.0f, scale * int32_t(pos.z / scale)),
                     (camera.orthographic()) ? Quaternion() : Quaternion(Vector3(1, 0, 0), 90.0f), scale);

    m_Buffer->setColor(m_PrimaryGridColor);
    m_Buffer->drawMesh(transform, m_pGrid, ICommandBuffer::TRANSLUCENT, m_pGizmo);

    Matrix4 m;
    m.scale(0.1f);

    m_Buffer->setColor(m_SecondaryGridColor);
    m_Buffer->drawMesh(transform * m, m_pGrid, ICommandBuffer::TRANSLUCENT, m_pGizmo);

    m_Buffer->setColor(Vector4(1.0f));
}
