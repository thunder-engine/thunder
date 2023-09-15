#include "systems/rendersystem.h"

#include "components/world.h"
#include "components/meshrender.h"
#include "components/textrender.h"
#include "components/spriterender.h"
#include "components/particlerender.h"
#include "components/directlight.h"
#include "components/pointlight.h"
#include "components/spotlight.h"
#include "components/arealight.h"
#include "components/skinnedmeshrender.h"
#include "components/tilemaprender.h"

#include "components/postprocessvolume.h"

#include "components/camera.h"
#include "components/actor.h"

#include "components/gui/recttransform.h"
#include "components/gui/widget.h"
#include "components/gui/image.h"
#include "components/gui/label.h"
#include "components/gui/button.h"
#include "components/gui/switch.h"
#include "components/gui/progressbar.h"
#include "components/gui/frame.h"
#include "components/gui/menu.h"
#include "components/gui/textinput.h"
#include "components/gui/floatinput.h"
#include "components/gui/toolbutton.h"

#include "resources/material.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"
#include "pipelinecontext.h"

int32_t RenderSystem::m_registered = 0;

list<Widget *> RenderSystem::m_uiComponents;
list<BaseLight *> RenderSystem::m_lightComponents;
list<Renderable *> RenderSystem::m_renderableComponents;
list<PostProcessVolume *> RenderSystem::m_postProcessVolumes;

RenderSystem::RenderSystem() :
        m_offscreen(false),
        m_pipelineContext(nullptr) {

    if(m_registered == 0) {
        Renderable::registerClassFactory(this);
        MeshRender::registerClassFactory(this);
        TextRender::registerClassFactory(this);
        SpriteRender::registerClassFactory(this);
        SkinnedMeshRender::registerClassFactory(this);

        BaseLight::registerClassFactory(this);
        DirectLight::registerClassFactory(this);
        PointLight::registerClassFactory(this);
        SpotLight::registerClassFactory(this);
        AreaLight::registerClassFactory(this);

        ParticleRender::registerClassFactory(this);

        TileMapRender::registerClassFactory(this);

        CommandBuffer::registerClassFactory(this);

        PostProcessVolume::registerClassFactory(this);

        PipelineContext::registerClassFactory(this);

        RectTransform::registerClassFactory(this);

        Widget::registerClassFactory(this);
        Image::registerClassFactory(this);
        Frame::registerClassFactory(this);
        Label::registerClassFactory(this);

        AbstractButton::registerClassFactory(this);
        Button::registerClassFactory(this);
        Switch::registerClassFactory(this);

        ProgressBar::registerClassFactory(this);

        Menu::registerClassFactory(this);

        TextInput::registerClassFactory(this);
        FloatInput::registerClassFactory(this);

        ToolButton::registerClassFactory(this);
    }
    ++m_registered;

    setName("Render");
}

RenderSystem::~RenderSystem() {
    --m_registered;

    if(m_registered) {
        Renderable::unregisterClassFactory(this);
        MeshRender::unregisterClassFactory(this);
        TextRender::unregisterClassFactory(this);
        SpriteRender::unregisterClassFactory(this);
        SkinnedMeshRender::unregisterClassFactory(this);

        BaseLight::unregisterClassFactory(this);
        DirectLight::unregisterClassFactory(this);
        PointLight::unregisterClassFactory(this);
        SpotLight::unregisterClassFactory(this);
        AreaLight::unregisterClassFactory(this);

        ParticleRender::unregisterClassFactory(this);

        TileMapRender::unregisterClassFactory(this);

        CommandBuffer::unregisterClassFactory(this);

        PostProcessVolume::unregisterClassFactory(this);

        RectTransform::unregisterClassFactory(this);

        Widget::unregisterClassFactory(this);
        Image::unregisterClassFactory(this);
        Frame::unregisterClassFactory(this);
        Label::unregisterClassFactory(this);

        AbstractButton::unregisterClassFactory(this);
        Button::unregisterClassFactory(this);
        Switch::unregisterClassFactory(this);

        ProgressBar::unregisterClassFactory(this);

        Menu::unregisterClassFactory(this);

        TextInput::unregisterClassFactory(this);
        FloatInput::unregisterClassFactory(this);

        ToolButton::unregisterClassFactory(this);
    }
}

int RenderSystem::threadPolicy() const {
    return Main;
}

bool RenderSystem::init() {
    m_pipelineContext = new PipelineContext;
    return true;
}

void RenderSystem::update(World *world) {
    PROFILE_FUNCTION();

    PROFILER_RESET(POLYGONS);
    PROFILER_RESET(DRAWCALLS);

    Camera *camera = Camera::current();
    if(camera && m_pipelineContext) {
        m_pipelineContext->analizeGraph(world);
        m_pipelineContext->draw(camera);
    }
}

void RenderSystem::composeComponent(Component *component) const {
    component->composeComponent();
}

Object *RenderSystem::instantiateObject(const MetaObject *meta, const string &name, Object *parent) {
    Object *result = ObjectSystem::instantiateObject(meta, name, parent);
    Widget *widget = dynamic_cast<Widget *>(result);
    if(widget) {
        widget->actor()->setLayers(CommandBuffer::UI | CommandBuffer::RAYCAST);
    }
    return result;
}

PipelineContext *RenderSystem::pipelineContext() const {
    return m_pipelineContext;
}

void RenderSystem::setOffscreenMode(bool mode) {
    m_offscreen = mode;
}

bool RenderSystem::isOffscreenMode() const {
    return m_offscreen;
}

void RenderSystem::addRenderable(Renderable *renderable) {
    m_renderableComponents.push_back(renderable);
}

void RenderSystem::removeRenderable(Renderable *renderable) {
    m_renderableComponents.remove(renderable);
}

list<Renderable *> &RenderSystem::renderables() {
    return m_renderableComponents;
}

void RenderSystem::addWidget(Widget *widget) {
    m_uiComponents.push_back(widget);
}

void RenderSystem::removeWidget(Widget *widget) {
    m_uiComponents.remove(widget);
}

list<Widget *> &RenderSystem::widgets() {
    return m_uiComponents;
}

void RenderSystem::addLight(BaseLight *light) {
    m_lightComponents.push_back(light);
}
void RenderSystem::removeLight(BaseLight *light) {
    m_lightComponents.remove(light);
}

list<BaseLight *> &RenderSystem::lights() {
    return m_lightComponents;
}

void RenderSystem::addPostProcessVolume(PostProcessVolume *volume) {
    m_postProcessVolumes.push_back(volume);
}

void RenderSystem::removePostProcessVolume(PostProcessVolume *volume) {
    m_postProcessVolumes.remove(volume);
}

list<PostProcessVolume *> &RenderSystem::postProcessVolumes() {
    return m_postProcessVolumes;
}

#if defined(SHARED_DEFINE)
QWindow *RenderSystem::createRhiWindow() {
    return nullptr;
}

ByteArray RenderSystem::renderOffscreen(World *sceneGraph, int width, int height) {
    static Texture *color = nullptr;
    if(color == nullptr) {
        color = Engine::objectCreate<Texture>();
        color->setFormat(Texture::RGBA8);
    }
    color->resize(width, height);

    static Texture *depth = nullptr;
    if(depth == nullptr) {
        depth = Engine::objectCreate<Texture>();
        depth->setFormat(Texture::Depth);
        depth->setDepthBits(24);
    }
    depth->resize(width, height);

    static RenderTarget *target = nullptr;
    if(target == nullptr) {
        target = Engine::objectCreate<RenderTarget>();
        target->setColorAttachment(0, color);
        target->setDepthAttachment(depth);
    }
    RenderTarget *back = nullptr;

    Camera *camera = Camera::current();
    if(camera && m_pipelineContext) {
        m_pipelineContext->resize(width, height);

        back = m_pipelineContext->defaultTarget();
        m_pipelineContext->setDefaultTarget(target);
    }

    setOffscreenMode(true);
    update(sceneGraph);
    setOffscreenMode(false);

    color->readPixels(0, 0, width, height);

    if(m_pipelineContext && back) {
        m_pipelineContext->setDefaultTarget(back);
    }

    return color->getPixels(0);
}
#endif
