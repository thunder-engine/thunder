#include "components/canvas.h"

#include "components/recttransform.h"
#include "components/widget.h"

#include <resources/texture.h>
#include <resources/material.h>
#include <resources/rendertarget.h>

#include <pipelinecontext.h>
#include <commandbuffer.h>

/*!
    \class Canvas
    \brief A rendering surface for UI components.

    Canvas provides an off-screen rendering surface for UI widgets.
    It renders all child widgets to a texture, which can then be
    displayed in the scene.
*/

Canvas::Canvas() :
        m_target(Engine::objectCreate<RenderTarget>("canvasTarget")),
        m_texture(Engine::objectCreate<Texture>("canvasTexture")),
        m_transform(nullptr),
        m_buffer(nullptr),
        m_finalMaterial(nullptr),
        m_dirty(true) {

    m_texture->setFormat(Texture::RGBA8);
    m_texture->setFlags(Texture::Render);

    m_target->setColorAttachment(0, m_texture);
    m_target->setClearColor(0.0f);
    m_target->setClearFlags(RenderTarget::ClearColor);

    static uint32_t hash = Mathf::hashString("canvas");
    addTagByHash(hash);

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultPostEffect.shader");
    if(mtl) {
        m_finalMaterial = mtl->createInstance();
        m_finalMaterial->setTexture("mainTexture", m_texture);

        Material::BlendState state;
        state.enabled = true;
        state.sourceColorBlendMode = Material::BlendFactor::One;
        state.sourceAlphaBlendMode = Material::BlendFactor::One;
        state.destinationColorBlendMode = Material::BlendFactor::OneMinusSourceAlpha;
        state.destinationAlphaBlendMode = Material::BlendFactor::OneMinusSourceAlpha;
        m_finalMaterial->setBlendState(state);
    }
}
/*!
    Marks the canvas as dirty, forcing a re-render.

    When marked dirty, the canvas will redraw all child widgets on the next draw call.
*/
void Canvas::markDirty() {
    m_dirty = true;
}
/*!
    Updates all child widgets with the given cursor/touch \a position.

    Propagates the update call to all child widgets, setting their canvas reference before updating.
*/
void Canvas::update(const Vector2 &position) {
    for(auto it : m_transform->children()) {
        RectTransform *rect = dynamic_cast<RectTransform *>(it);
        if(rect) {
            Widget *widget = rect->widget();
            if(widget) {
                widget->m_canvas = this;
                widget->update(position);
            }
        }
    }
}
/*!
    Draws the canvas and its contents uses command \a buffer to record draw commands into.
*/
void Canvas::draw(CommandBuffer *buffer) {
    m_buffer = buffer;

    RenderTarget *target = m_buffer->renderTarget();
    if(m_dirty) {
        Matrix4 v;
        v[14] = -50.0f;

        m_buffer->setRenderTarget(m_target);
        m_buffer->setViewProjection(v, Matrix4::ortho(0, m_texture->width(), 0, m_texture->height(), 0.0f, 100.0f));

        for(auto it : m_transform->children()) {
            RectTransform *rect = dynamic_cast<RectTransform *>(it);
            if(rect) {
                Widget *widget = rect->widget();
                if(widget) {
                    widget->draw();
                }
            }
        }
        m_dirty = false;
    }

    m_buffer->setRenderTarget(target);
    m_buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_finalMaterial);
}
/*!
    \brief Draws a rectangle with the given \a material and \a transform.

    Creates a model matrix from the rect's size and position, combines
    it with the world transform, and draws the default plane mesh.
    The hash is computed from the transform and size for batching purposes.
*/
void Canvas::drawRect(MaterialInstance *material, RectTransform *rect) {
    if(rect) {
        Vector2 size(rect->size());
        Matrix4 s;
        s[0] = size.x;
        s[5] = size.y;
        s[12] = size.x * 0.5f;
        s[13] = size.y * 0.5f;

        uint32_t hash = rect->hash();
        Mathf::hashCombine(hash, s[0]);
        Mathf::hashCombine(hash, s[5]);
        Mathf::hashCombine(hash, s[12]);
        Mathf::hashCombine(hash, s[13]);

        material->setTransform(rect->worldTransform() * s, 0, hash);
    }

    drawMesh(PipelineContext::defaultPlane(), material);
}
/*!
    Draws a \a mesh with the given \a material.
*/
void Canvas::drawMesh(Mesh *mesh, MaterialInstance *material) {
    m_buffer->drawMesh(mesh, 0, Material::Translucent, *material);
}

void Canvas::setSize(int width, int height) {
    if(m_texture) {
        if(m_texture->width() == width && m_texture->height() == height) {
            return;
        }

        m_texture->resize(width, height);
    }

    RectTransform *rect = dynamic_cast<RectTransform *>(transform());
    if(rect) {
        rect->setSize(Vector2(width, height));
    }
}
/*!
    Returns the RectTransform component of the canvas.

    Lazy-initializes and caches the RectTransform reference.
*/
RectTransform *Canvas::rectTransform() {
    if(m_transform == nullptr) {
        setRectTransform(dynamic_cast<RectTransform *>(transform()));
    }
    return m_transform;
}
/*!
    Sets the rect \a transform reference.

    Internal method for caching the RectTransform.
*/
void Canvas::setRectTransform(RectTransform *transform) {
    if(m_transform != transform) {
        m_transform = transform;
    }
}
/*!
    Sets the clip region (scissor rectangle).

    Enables scissor testing to restrict rendering to the specified region.
    Coordinates are in screen space.
*/
void Canvas::setClipRegion(const Vector4 &region) {
    m_buffer->enableScissor(region.x, region.y, region.z, region.w);
}
/*!
    Disables the clip region.

    Turns off scissor testing, allowing rendering to the full screen.
*/
void Canvas::disableClip() {
    m_buffer->disableScissor();
}
/*!
    \internal
*/
void Canvas::composeComponent() {
    Actor *object = Canvas::actor();
    if(object) {
        Transform *transform = object->transform();

        RectTransform *rect = dynamic_cast<RectTransform *>(transform);
        if(rect == nullptr) {
            if(transform) {
                delete transform;
            }

            rect = Engine::objectCreate<RectTransform>("RectTransform", object);
            object->setTransform(rect);

            setRectTransform(rect);
        }
    }
}
