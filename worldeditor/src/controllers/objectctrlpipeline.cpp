#include "objectctrlpipeline.h"

#include "rendertexture.h"
#include "commandbuffer.h"

#define SELECT_MAP  "selectMap"
#define DEPTH_MAP   "depthMap"

ObjectCtrlPipeline::ObjectCtrlPipeline() :
        Pipeline() {
    RenderTexture *select   = Engine::objectCreate<RenderTexture>();
    select->setTarget(Texture::RGBA8);
    select->apply();
    m_Targets[SELECT_MAP]   = select;
    m_Buffer->setGlobalTexture(SELECT_MAP,  select);
}

void ObjectCtrlPipeline::draw(Scene *scene, Camera &camera) {
    // Retrive object id
    m_Buffer->setRenderTarget({m_Targets[SELECT_MAP]}, m_Targets[DEPTH_MAP]);
    m_Buffer->clearRenderTarget(true, Vector4(0.0));

    m_Buffer->setViewport(0, 0, m_Screen.x, m_Screen.y);

    cameraReset(camera);
    drawComponents(ICommandBuffer::RAYCAST, m_Components);

    Pipeline::draw(scene, camera);
}

