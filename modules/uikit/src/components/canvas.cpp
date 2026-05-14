#include "components/canvas.h"

#include "components/recttransform.h"
#include "components/widget.h"

#include "resources/texture.h"
#include "resources/rendertarget.h"

#include "pipelinecontext.h"
#include "commandbuffer.h"

Canvas::Canvas() :
        m_target(Engine::objectCreate<RenderTarget>("canvasTarget")),
        m_texture(Engine::objectCreate<Texture>("canvasTexture")),
        m_transform(nullptr) {

    m_texture->setFormat(Texture::RGBA8);
    m_texture->setFlags(Texture::Render);

    m_target->setColorAttachment(0, m_texture);
    m_target->setClearFlags(RenderTarget::ClearColor);

    static uint32_t hash = Mathf::hashString("canvas");
    addTagByHash(hash);
}

void Canvas::update(const Vector2 &pos) {
    for(auto it : m_transform->children()) {
        RectTransform *rect = dynamic_cast<RectTransform *>(it);
        if(rect) {
            Widget *widget = rect->widget();
            if(widget) {
                widget->setCanvas(this);
                widget->update(pos);
            }
        }
    }
}

void Canvas::draw(CommandBuffer *buffer) {
    m_buffer = buffer;

    Matrix4 v;
    v[14] = -50.0f;
    buffer->setViewProjection(v, Matrix4::ortho(0, m_texture->width(), 0, m_texture->height(), 0.0f, 100.0f));

    for(auto it : m_transform->children()) {
        RectTransform *rect = dynamic_cast<RectTransform *>(it);
        if(rect) {
            Widget *widget = rect->widget();
            if(widget) {
                widget->draw();
            }
        }
    }
}

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

void Canvas::drawMesh(Mesh *mesh, MaterialInstance *material) {
    m_buffer->drawMesh(mesh, 0, Material::Translucent, *material);
}

void Canvas::setSize(int width, int height) {
    if(m_texture) {
        m_texture->resize(width, height);
    }

    RectTransform *rect = dynamic_cast<RectTransform *>(transform());
    if(rect) {
        rect->setSize(Vector2(width, height));
    }
}

RectTransform *Canvas::rectTransform() {
    if(m_transform == nullptr) {
        setRectTransform(dynamic_cast<RectTransform *>(transform()));
    }
    return m_transform;
}

void Canvas::setRectTransform(RectTransform *transform) {
    if(m_transform != transform) {
        m_transform = transform;
    }
}

void Canvas::setClipRegion(const Vector4 &rect) {
    m_buffer->enableScissor(rect.x, rect.y, rect.z, rect.w);
}

void Canvas::disableClip() {
    m_buffer->disableScissor();
}

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
