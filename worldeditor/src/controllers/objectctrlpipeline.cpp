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

#include <QVariant>
#include <QColor>

#define SELECT_MAP  "selectMap"

#define G_NORMALS   "normalsMap"
#define G_DIFFUSE   "diffuseMap"
#define G_PARAMS    "paramsMap"
#define G_EMISSIVE  "emissiveMap"

#define DEPTH_MAP   "depthMap"
#define SHADOW_MAP  "shadowMap"

#define OVERRIDE "uni.texture0"

ObjectCtrlPipeline::ObjectCtrlPipeline() :
        Pipeline() {
    RenderTexture *select   = Engine::objectCreate<RenderTexture>();
    select->setTarget(Texture::RGBA8);
    select->apply();
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
    m_pGrid->apply();

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

void ObjectCtrlPipeline::draw(Scene *scene, Camera &camera) {
    // Retrive object id
    m_Buffer->setRenderTarget({m_Targets[SELECT_MAP]}, m_Targets[DEPTH_MAP]);
    m_Buffer->clearRenderTarget(true, Vector4(0.0));

    m_Buffer->setViewport(0, 0, static_cast<int32_t>(m_Screen.x), static_cast<int32_t>(m_Screen.y));

    cameraReset(camera);
    drawComponents(ICommandBuffer::RAYCAST, m_Components);

    ObjectList filter = frustumCulling(m_Components, camera.frustumCorners(camera.nearPlane(), camera.farPlane()));

    sortByDistance(filter, camera.actor()->transform()->position());

    // Light prepass
    m_Buffer->setGlobalValue("light.ambient", scene->ambient());

    m_Buffer->setRenderTarget(TargetBuffer(), m_Targets[SHADOW_MAP]);
    m_Buffer->clearRenderTarget();

    updateShadows(camera, scene);

    m_Buffer->setViewport(0, 0, static_cast<int32_t>(m_Screen.x), static_cast<int32_t>(m_Screen.y));

    cameraReset(camera);

    // Step1 - Fill G buffer pass
    m_Buffer->setRenderTarget({m_Targets[G_NORMALS], m_Targets[G_DIFFUSE], m_Targets[G_PARAMS], m_Targets[G_EMISSIVE]}, m_Targets[DEPTH_MAP]);
    m_Buffer->clearRenderTarget(true, camera.color());

    // Draw Opaque pass
    drawComponents(ICommandBuffer::DEFAULT, filter);

    /// \todo Screen Space Ambient Occlusion effect should be defined here
    m_Buffer->setRenderTarget({m_Targets[G_EMISSIVE]}, m_Targets[DEPTH_MAP]);

    // Step2 - Light pass
    drawComponents(ICommandBuffer::LIGHT, filter);

    cameraReset(camera);
    // Step3 - Draw Transparent pass
    drawComponents(ICommandBuffer::TRANSLUCENT, filter);

    drawGrid(camera);

    m_Buffer->setRenderTarget(m_Target);
    m_Buffer->setScreenProjection();
    m_Buffer->clearRenderTarget(true, camera.color(), false);

    m_pSprite->setTexture(OVERRIDE, postProcess(*m_Targets[G_EMISSIVE]));
    m_Buffer->drawMesh(Matrix4(), m_pPlane, ICommandBuffer::UI, m_pSprite);
}

void ObjectCtrlPipeline::drawGrid(Camera &camera) {
    Transform *t = camera.actor()->transform();
    Vector3 cam = t->position();
    Vector3 pos;
    float length;

    if(camera.orthographic()) {
        pos = Vector3(cam.x, cam.y, 0.0);
        length = camera.orthoHeight();
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
