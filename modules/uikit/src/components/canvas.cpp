/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "components/canvas.h"

#include "components/widget.h"
#include "components/menu.h"

#include <stylesheet.h>

#include <resources/texture.h>
#include <resources/material.h>
#include <resources/rendertarget.h>

#include <pipelinecontext.h>
#include <commandbuffer.h>
#include <input.h>
#include <pugixml.hpp>

namespace {
    const char *gUi("ui");
    const char *gName("name");
    const char *gStyle("style");
    const char *gClass("class");

    void loadElementHelper(pugi::xml_node &node, Widget *parent, bool root = false) {
        std::string type = node.name();
        std::string name = node.attribute(gName).as_string();

        Widget *element = dynamic_cast<Widget *>(parent->find(name));
        if(element == nullptr) {
            element = Widget::createWidget(type, name, parent);
        }

        Widget *widget = element;
        if(widget) {
            const MetaObject *meta = widget->metaObject();
            for(auto it : node.attributes()) {
                int32_t index = meta->indexOfProperty(it.name());
                if(index > -1) {
                    MetaProperty property = meta->property(index);
                    Variant current = widget->property(property.name());

                    TString annotation;
                    const char *text = property.table()->annotation;
                    if(text) {
                        annotation = text;
                    }

                    switch(current.type()) {
                        case MetaType::BOOLEAN: widget->setProperty(property.name(), it.as_bool()); break;
                        case MetaType::INTEGER: widget->setProperty(property.name(), it.as_int()); break;
                        case MetaType::FLOAT: widget->setProperty(property.name(), it.as_float()); break;
                        case MetaType::STRING: widget->setProperty(property.name(), it.as_string()); break;
                        default: {
                            if(annotation == "editor=Asset") {
                                Resource *resource = Engine::loadResource(it.as_string());
                                if(resource) {
                                    uint32_t type = MetaType::type(resource->typeName().data()) + 1;
                                    widget->setProperty(property.name(), Variant(type, &resource));
                                }
                            }
                        } break;
                    }
                }
            }

            TString classes = node.attribute(gClass).as_string();
            if(!classes.isEmpty()) {
                for(auto &it : classes.split(' ')) {
                    widget->addClass(it);
                }
            }

            TString style = node.attribute(gStyle).as_string();
            if(!style.isEmpty()) {
                StyleSheet::resolveInline(widget, style);
            }
        }

        if(element) {
            for(pugi::xml_node it : node.children()) {
                loadElementHelper(it, element);
            }

            if(root) {
                element->blockSerialization(true);
            }
        }
    }
}

/*!
    \class Canvas
    \brief A rendering surface for UI components.
    \inmodule Gui

    Canvas provides an off-screen rendering surface for UI widgets.
    It renders all child widgets to a texture, which can then be
    displayed in the scene.
*/

Canvas::Canvas() :
        m_target(Engine::objectCreate<RenderTarget>("canvasTarget")),
        m_texture(Engine::objectCreate<Texture>("canvasTexture")),
        m_buffer(nullptr),
        m_finalMaterial(nullptr),
        m_document(nullptr),
        m_styleSheet(nullptr),
        m_dirty(true),
        m_skipRenderResult(false),
        m_lastPositionValid(false) {

    m_texture->setFormat(Texture::RGBA8);
    m_texture->setFlags(Texture::Render);

    m_target->setColorAttachment(0, m_texture);
    m_target->setClearColor(0.0f);
    m_target->setFlags(RenderTarget::ClearColor);

    Material *mtl = Engine::loadResource<Material>(".embedded/DefaultPostEffect.shader");
    if(mtl) {
        m_finalMaterial = mtl->createInstance();
        m_finalMaterial->setTexture("mainTexture", m_texture);

        mtl->subscribe(&Canvas::materialUpdated, this);
    }
}

Canvas::~Canvas() {
    if(m_finalMaterial) {
        m_finalMaterial->material()->unsubscribe(this);
        delete m_finalMaterial;
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
    if(parentWidget()) {
        Widget::update(position);
        return;
    }

    Vector2 globalPos(position);
    if(!m_lastPositionValid || globalPos != m_lastPosition) {
        dispatchMouseEvent(globalPos, Event::MouseMove, 0);
        m_lastPosition = globalPos;
        m_lastPositionValid = true;
    }

    float wheelDelta = Input::mouseScrollDelta();
    if(wheelDelta != 0.0f) {
        int delta = static_cast<int>(wheelDelta);
        if(delta == 0) {
            delta = wheelDelta > 0.0f ? 1 : -1;
        }
        dispatchMouseWheelEvent(globalPos, delta, false);
    }

    for(int key = Input::KEY_SPACE; key <= Input::KEY_MENU; ++key) {
        if(Input::isKeyDown((Input::KeyCode)key) || Input::isKeyUp((Input::KeyCode)key)) {
            KeyEvent event(key, Input::isKeyDown((Input::KeyCode)key));
            dispatchKeyEvent(&event);
        }
    }

    if(Input::isMouseButtonDown(Input::MOUSE_LEFT)) {
        dispatchMouseEvent(globalPos, Event::MouseDown, Input::MOUSE_LEFT);
    }
    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        dispatchMouseEvent(globalPos, Event::MouseUp, Input::MOUSE_LEFT);
    }
    if(Input::isMouseButtonDoubleClick(Input::MOUSE_LEFT)) {
        dispatchMouseEvent(globalPos, Event::MouseDoubleClick, Input::MOUSE_LEFT);
    }

    Widget::update(position);
    if(Input::isMouseButtonUp(Input::MOUSE_LEFT)) {
        Widget::finishDrag(false);
    }
}
/*!
    Draws the canvas and its contents uses command \a buffer to record draw commands into.
*/
void Canvas::draw(CommandBuffer *buffer) {
    m_buffer = buffer;

    Vector2 canvasSize(size());
    if(canvasSize.x > 0.0f && canvasSize.y > 0.0f) {
        resizeTexture(static_cast<int>(canvasSize.x), static_cast<int>(canvasSize.y));
    }

    RenderTarget *target = m_buffer->renderTarget();
    Vector4 viewport = m_buffer->viewport();
    Matrix4 view;
    view[14] = -50.0f;
    Matrix4 projection = Matrix4::ortho(0, m_texture->width(), 0, m_texture->height(), 0.0f, 100.0f);

    /*if(m_dirty)*/ {
        m_buffer->setViewProjection(view * worldTransform().inverse(), projection);
        RenderTarget *canvasTarget = renderTarget();
        if(m_buffer->renderTarget() != canvasTarget) {
            m_buffer->setRenderTarget(canvasTarget);
        }
        m_buffer->setViewport(0, 0, m_texture->width(), m_texture->height());

        Widget::draw();

        m_drawingPopups = true;
        for(size_t i = 0; i < m_popupMenus.size(); ++i) {
            Menu *menu = m_popupMenus[i];
            if(menu && menu->isVisible() && menu->isEnabled()) {
                menu->draw();
            }
        }
        m_drawingPopups = false;
        m_popupMenus.clear();
        m_dirty = false;
    }

    if(!m_skipRenderResult) {
        m_buffer->setViewProjection(view, projection);
    }
    if(m_buffer->renderTarget() != target) {
        m_buffer->setRenderTarget(target);
    }
    if(!m_skipRenderResult && m_finalMaterial) {
        Matrix4 transform;
        transform[0] = canvasSize.x;
        transform[5] = canvasSize.y;
        transform[12] = canvasSize.x * 0.5f;
        transform[13] = canvasSize.y * 0.5f;
        uint32_t hash = this->hash();
        Mathf::hashCombine(hash, transform[0]);
        Mathf::hashCombine(hash, transform[5]);
        Mathf::hashCombine(hash, transform[12]);
        Mathf::hashCombine(hash, transform[13]);
        m_finalMaterial->setTransform(worldTransform() * transform, 0, hash);
        m_buffer->drawMesh(PipelineContext::defaultPlane(), 0, Material::Opaque, *m_finalMaterial);
    }
    if(viewport.z > 0.0f && viewport.w > 0.0f) {
        m_buffer->setViewport(static_cast<int32_t>(viewport.x), static_cast<int32_t>(viewport.y),
                              static_cast<int32_t>(viewport.z), static_cast<int32_t>(viewport.w));
    }
}
/*!
    \brief Draws a rectangle with the given \a material and \a transform.

    Creates a model matrix from the rect's size and position, combines
    it with the world transform, and draws the default plane mesh.
    The hash is computed from the transform and size for batching purposes.
*/
void Canvas::drawRect(MaterialInstance *material, Widget *transform) {
    if(transform) {
        Vector2 size(transform->size());
        Matrix4 s;
        s[0] = size.x;
        s[5] = size.y;
        s[12] = size.x * 0.5f;
        s[13] = size.y * 0.5f;

        uint32_t hash = transform->hash();
        Mathf::hashCombine(hash, s[0]);
        Mathf::hashCombine(hash, s[5]);
        Mathf::hashCombine(hash, s[12]);
        Mathf::hashCombine(hash, s[13]);

        material->setTransform(transform->worldTransform() * s, 0, hash);
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
    Widget::setSize(Vector2(width, height));
    resizeTexture(width, height);
}

void Canvas::resizeTexture(int width, int height) {
    if(m_texture) {
        if(m_texture->width() == width && m_texture->height() == height) {
            return;
        }

        m_texture->resize(width, height);
        m_dirty = true;
    }
}
/*!
    Sets the clip \a region (scissor rectangle).

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
    Returns Canvas texture.
*/
Texture *Canvas::texture() const {
    return m_texture;
}

RenderTarget *Canvas::renderTarget() const {
    return m_target;
}

Canvas *Canvas::canvas() {
    return this;
}
/*!
    Allows to \a skip the rendering of the result.
    This is useful if the UI is the only rendering task.
*/
void Canvas::skipRenderResult(bool skip) {
    m_skipRenderResult = skip;
}
/*!
    Returns the UI document associated with this canvas.
*/
UiDocument *Canvas::document() const {
    return m_document;
}
/*!
    Sets the UI document and reloads the canvas hierarchy.
*/
void Canvas::setDocument(UiDocument *document) {
    if(m_document != document) {
        m_document = document;

        if(m_document) {
            fromBuffer(m_document->data());
        } else {
            cleanHierarchy(this);
        }
    }
}
/*!
    Returns the stylesheet assigned to this canvas.
*/
StyleSheet *Canvas::styleSheet() const {
    return m_styleSheet;
}
/*!
    Sets a stylesheet for the canvas hierarchy.
*/
void Canvas::setStyleSheet(StyleSheet *style) {
    if(m_styleSheet != style) {
        m_styleSheet = style;

        if(m_styleSheet) {
            m_styleSheet->addRawData(m_documentStyle);
            resolveStyleSheet(this);
        }

        applyStyle();
    }
}
/*!
    Returns the raw style definition embedded in the UI document.
*/
TString Canvas::documentStyle() const {
    return m_documentStyle;
}
/*!
    Loads UI elements and style definitions from an XML buffer.
*/
void Canvas::fromBuffer(const TString &buffer) {
    cleanHierarchy(this);

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());

    if(result) {
        for(pugi::xml_node node : doc.child(gUi).children()) {
            std::string type = node.name();
            if(type == gStyle) {
                m_documentStyle = node.text().as_string();

                if(m_styleSheet) {
                    m_styleSheet->addRawData(m_documentStyle);
                }
            } else {
                loadElementHelper(node, this, true);
            }
        }

        applyStyle();
        documentLoaded();
    }
}
/*!
    Emits a signal when the UI document is loaded.
*/
void Canvas::documentLoaded() {
    emitSignal(_SIGNAL(documentLoaded()));
}

/*!
    \internal
    Draws content of this canvas as widget.
*/
void Canvas::draw() {
    Vector4 viewport(m_buffer->viewport());
    Vector4 clip(clipRegion());
    if(clip.z > 0.0f && clip.w > 0.0f) {
        m_buffer->setViewport(clip.x, clip.y, clip.z, clip.w);
        if(m_finalMaterial) {
            drawRect(m_finalMaterial, this);
        }
        if(viewport.z > 0.0f && viewport.w > 0.0f) {
            m_buffer->setViewport(viewport.x, viewport.y, viewport.z, viewport.w);
        }
    }
}
/*!
    \internal
*/
void Canvas::resolveStyleSheet(Widget *widget) {
    for(auto it : widget->childWidgets()) {
        if(!it->isSubWidget()) {
            m_styleSheet->resolve(it);
            resolveStyleSheet(it);
        }
    }
}
/*!
    \internal
*/
void Canvas::cleanHierarchy(Widget *widget) {
    std::list<Widget *> children = widget->childWidgets();

    for(auto it : children) {
        if(!it->isSubWidget()) {
            delete it;
        }
    }
}
/*!
    \internal
*/
void Canvas::materialUpdated(int state, void *ptr) {
    if(state == Material::ToBeDeleted) {
        Canvas *canvas = static_cast<Canvas *>(ptr);
        if(canvas && canvas->m_finalMaterial) {
            delete canvas->m_finalMaterial;
            canvas->m_finalMaterial = nullptr;
        }
    }
}
