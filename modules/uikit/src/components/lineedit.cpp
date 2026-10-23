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
#include "lineedit.h"

#include "label.h"
#include "frame.h"
#include "canvas.h"

#include <resources/mesh.h>
#include <resources/font.h>
#include <resources/material.h>

#ifndef THUNDER_MOBILE
#include <clipboard.h>
#endif
#include <input.h>
#include <timer.h>
#include <pipelinecontext.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iterator>

#define HOLD_TIME 0.1f

namespace {
    const char *gTexture("mainTexture");
    const char *gColor("mainColor");
    const char *gUseSDF("useSdf");

    const char *gDefaultFont(".embedded/DefaultFont.shader");
    const char *gDefaultSprite(".embedded/DefaultSprite.shader");
}

/*!
    \class LineEdit
    \brief The LineEdit class is a UI component that allows users to input text.
    \inmodule Gui

    The LineEdit class provides a user interface for text input, supporting text editing, cursor positioning, and input handling.
    It inherits functionality from the Widget class and extends it to handle text-related features and animations.
*/

LineEdit::LineEdit() :
        m_textColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_selectionColor(0.01f, 0.47f, 0.74f, 1.0f),
        m_cursorMaterial(nullptr),
        m_selectionMaterial(nullptr),
        m_fontMaterial(nullptr),
        m_font(nullptr),
        m_textMesh(Engine::objectCreate<Mesh>()),
        m_cursorPosition(0),
        m_selectionAnchor(0),
        m_fontSize(14),
        m_textPosition(0.0f),
        m_cursorBlinkRate(0.85f),
        m_cursorBlinkCurrent(0.0f),
        m_cursorRepeatHold(0.0f),
        m_hovered(false),
        m_focused(false),
        m_cursorVisible(false),
        m_mouseSelecting(false),
        m_textDirty(true) {

    setBackgroundColor(Vector4(0.38f, 0.38f, 0.38f, 1.0f));
    m_textMesh->makeDynamic();

    Material *fontMaterial = Engine::loadResource<Material>(gDefaultFont);
    if(fontMaterial) {
        m_fontMaterial = fontMaterial->createInstance();
        fontMaterial->subscribe(&LineEdit::materialUpdated, this);
    }

    Material *cursorMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(cursorMaterial) {
        m_cursorMaterial = cursorMaterial->createInstance();
        m_selectionMaterial = cursorMaterial->createInstance();
        m_cursorMaterial->setTexture(gTexture, PipelineContext::whiteTexture());
        m_selectionMaterial->setTexture(gTexture, PipelineContext::whiteTexture());
        const Vector4 cursorColor(1.0f);
        m_cursorMaterial->setVector4(gColor, &cursorColor);
        m_selectionMaterial->setVector4(gColor, &m_selectionColor);
        cursorMaterial->subscribe(&LineEdit::materialUpdated, this);
    }

    m_cursorTransform.scale(Vector3(2.0f, 16.0f, 1.0f));
}

LineEdit::~LineEdit() {
    if(m_font) {
        m_font->unsubscribe(this);
    }
    if(m_fontMaterial) {
        m_fontMaterial->material()->unsubscribe(this);
    }
    if(m_cursorMaterial) {
        m_cursorMaterial->material()->unsubscribe(this);
    }
    if(m_selectionMaterial) {
        m_selectionMaterial->material()->unsubscribe(this);
    }

    delete m_textMesh;
    delete m_fontMaterial;
    delete m_cursorMaterial;
    delete m_selectionMaterial;
}
/*!
    \internal
*/
void LineEdit::draw() {
    Canvas *canvas = LineEdit::canvas();
    const bool showingPlaceholder = m_text.isEmpty() && !m_placeholderText.isEmpty();
    const TString &displayText = showingPlaceholder ? m_placeholderText : m_text;

    if(m_textDirty && m_font && m_fontMaterial) {
        const Vector4 color = showingPlaceholder ? Vector4(0.6f, 0.6f, 0.6f, 1.0f) : m_textColor;
        const Font::Settings settings = {m_fontSize, Alignment::Left | Alignment::Middle, 0, color, size()};
        m_font->composeMesh(m_textMesh, displayText, settings);

        m_fontMaterial->setTexture(gTexture, m_font->page());

        m_textDirty = false;
    }

    // Draw background
    Frame::draw();
    canvas->setClipRegion(clipRegion());

    const Vector3Vector &vertices = m_textMesh->vertices();
    float cursorY = size().y * 0.5f;
    if(m_font && !m_text.isEmpty() && !vertices.empty()) {
        float minY = vertices.front().y;
        float maxY = minY;
        for(const Vector3 &vertex : vertices) {
            minY = MIN(minY, vertex.y);
            maxY = MAX(maxY, vertex.y);
        }
        cursorY = (minY + maxY) * 0.5f;
    }

    const int32_t selectionStart = MIN(m_cursorPosition, m_selectionAnchor);
    const int32_t selectionEnd = MAX(m_cursorPosition, m_selectionAnchor);
    if(m_focused && selectionStart != selectionEnd && m_selectionMaterial && m_font) {
        const float start = cursorAt(selectionStart) + m_textPosition;
        const float end = cursorAt(selectionEnd) + m_textPosition;
        const float width = end - start;
        if(width > 0.0f) {
            Matrix4 selectionTransform;
            selectionTransform.scale(Vector3(width, m_cursorTransform[5], 1.0f));
            selectionTransform[12] = (start + end) * 0.5f;
            selectionTransform[13] = cursorY;

            uint32_t hash = LineEdit::hash();
            Mathf::hashCombine(hash, selectionTransform[12]);
            Mathf::hashCombine(hash, selectionTransform[13]);
            Mathf::hashCombine(hash, width);
            m_selectionMaterial->setTransform(worldTransform() * selectionTransform, 0, hash);
            canvas->drawMesh(PipelineContext::defaultPlane(), m_selectionMaterial);
        }
    }
    // Draw text
    if(m_fontMaterial && !displayText.isEmpty()) {
        Matrix4 m(worldTransform());
        m[12] += m_textPosition;

        uint32_t hash = LineEdit::hash();
        Mathf::hashCombine(hash, m[12]);

        m_fontMaterial->setTransform(m, 0, hash);
        canvas->drawMesh(m_textMesh, m_fontMaterial);
    }

    // Draw cursor
    if(m_cursorVisible && m_cursorMaterial) {
        m_cursorTransform[13] = cursorY;

        uint32_t hash;
        std::memcpy(&hash, &m_cursorTransform[12], sizeof(float));
        m_cursorMaterial->setTransform(worldTransform() * m_cursorTransform, 0, hash);
        canvas->drawRect(m_cursorMaterial, nullptr);
    }
    canvas->disableClip();
}
/*!
    Returns the current text entered into the TextInput.
*/
TString LineEdit::text() const {
    return m_text;
}
/*!
    Sets the \a text in the TextInput.
*/
void LineEdit::setText(const TString &text) {
    m_text = text;
    m_textDirty = true;
    const int32_t textLength = static_cast<int32_t>(m_text.toUtf32().size());
    if(m_cursorPosition == 0) {
        m_cursorPosition = textLength;
    } else {
        m_cursorPosition = std::min(m_cursorPosition, textLength);
    }
    m_selectionAnchor = m_cursorPosition;
    recalcCursor();
    repaint();
}
/*!
    Returns the font which will be used to draw a text.
*/
TString LineEdit::placeholderText() const {
    return m_placeholderText;
}

void LineEdit::setPlaceholderText(const TString &text) {
    if(m_placeholderText != text) {
        m_placeholderText = text;
        m_textDirty = true;
        repaint();
    }
}

Font *LineEdit::font() const {
    return m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void LineEdit::setFont(Font *font) {
    if(m_font != font) {
        if(m_font) {
            m_font->unsubscribe(this);
        }

        m_font = font;
        if(m_font) {
            m_font->subscribe(&LineEdit::fontUpdated, this);
        }

        m_textDirty = true;
        repaint();
    }
}
/*!
    Returns color of the text.
*/
Vector4 LineEdit::textColor() const {
    return m_textColor;
}
/*!
    Sets the \a color of the text.
*/
void LineEdit::setTextColor(const Vector4 &color) {
    m_textColor = color;
    m_textDirty = true;
    repaint();
}
/*!
    \brief Called when the component receives focus.
    Emits a signal indicating focus has been gained.
*/
void LineEdit::focusIn() {
    emitSignal(_SIGNAL(focusIn()));
}
/*!
    \brief Called when the component loses focus.
    Emits a signal indicating focus has been lost.
*/
void LineEdit::focusOut() {
    emitSignal(_SIGNAL(focusOut()));
}
/*!
    \brief Called when editing is finished (Enter key pressed).
    Emits a signal indicating editing has completed.
*/
void LineEdit::editingFinished() {
    emitSignal(_SIGNAL(editingFinished()));
}

void LineEdit::editingCanceled() {
    emitSignal(_SIGNAL(editingCanceled()));
}
/*!
    \internal
    Overrides the update method to handle text input and cursor animation.
*/
void LineEdit::update(const Vector2 &pos) {
    if(Widget::focusWidget() == this) {
        if(!m_focused) {
            focusIn();
            m_focused = true;
            m_cursorVisible = true;
            repaint();
        }

        bool breakCursorFlashing = false;
        if(Input::isKeyDown(Input::KEY_ESCAPE)) {
            editingCanceled();
        } else if(Input::isKeyDown(Input::KEY_ENTER) || Input::isKeyDown(Input::KEY_KP_ENTER)) {
            editingFinished();
        } else {
            const bool shift = Input::isKey(Input::KEY_LEFT_SHIFT) || Input::isKey(Input::KEY_RIGHT_SHIFT);
            const bool control = Input::isKey(Input::KEY_LEFT_CONTROL) || Input::isKey(Input::KEY_RIGHT_CONTROL);
            const bool alt = Input::isKey(Input::KEY_LEFT_ALT) || Input::isKey(Input::KEY_RIGHT_ALT);
            const bool super = Input::isKey(Input::KEY_LEFT_SUPER) || Input::isKey(Input::KEY_RIGHT_SUPER);
            const bool command = control || super;
            const int32_t textLength = static_cast<int32_t>(m_text.toUtf32().size());

            bool clipboardKey = false;
            if(command && Input::isKeyDown(Input::KEY_A)) {
                m_selectionAnchor = 0;
                m_cursorPosition = textLength;
                recalcCursor();
                breakCursorFlashing = true;
                clipboardKey = true;
            }
#ifndef THUNDER_MOBILE
            if(command && (Input::isKeyDown(Input::KEY_C) || Input::isKeyDown(Input::KEY_X))) {
                clipboardKey = true;
                if(hasSelection()) {
                    const std::u32string text = m_text.toUtf32();
                    const int32_t start = MIN(m_cursorPosition, m_selectionAnchor);
                    const int32_t end = MAX(m_cursorPosition, m_selectionAnchor);
                    const TString selected = TString::fromUtf32(text.substr(start, end - start));
                    if(Clipboard::setText(selected) && Input::isKeyDown(Input::KEY_X)) {
                        std::u32string editedText = text;
                        editedText.erase(start, end - start);
                        replaceText(editedText, start);
                        breakCursorFlashing = true;
                    }
                }
            } else if(command && Input::isKeyDown(Input::KEY_V)) {
                clipboardKey = true;
                const std::u32string pasted = Clipboard::text().toUtf32();
                std::u32string inserted;
                std::copy_if(pasted.begin(), pasted.end(), std::back_inserter(inserted),
                             [](char32_t character) { return character >= 32; });
                if(!inserted.empty()) {
                    std::u32string editedText = m_text.toUtf32();
                    const int32_t start = MIN(m_cursorPosition, m_selectionAnchor);
                    const int32_t end = MAX(m_cursorPosition, m_selectionAnchor);
                    editedText.erase(start, end - start);
                    editedText.insert(start, inserted);
                    replaceText(editedText, start + static_cast<int32_t>(inserted.size()));
                    breakCursorFlashing = true;
                } else if(hasSelection()) {
                    std::u32string editedText = m_text.toUtf32();
                    const int32_t start = MIN(m_cursorPosition, m_selectionAnchor);
                    const int32_t end = MAX(m_cursorPosition, m_selectionAnchor);
                    editedText.erase(start, end - start);
                    replaceText(editedText, start);
                    breakCursorFlashing = true;
                }
            }
#endif

            const bool backspace = Input::isKey(Input::KEY_BACKSPACE);
            const bool deleteForward = Input::isKey(Input::KEY_DELETE);
            const bool left = Input::isKey(Input::KEY_LEFT);
            const bool right = Input::isKey(Input::KEY_RIGHT);
            const bool editingKeyHeld = backspace || deleteForward || left || right;
            if(editingKeyHeld) {
                if(m_cursorRepeatHold <= 0.0f) {
                    if(backspace || deleteForward) {
                        std::u32string text = m_text.toUtf32();
                        int32_t start = MIN(m_cursorPosition, m_selectionAnchor);
                        int32_t end = MAX(m_cursorPosition, m_selectionAnchor);
                        if(start == end) {
                            if(backspace && start > 0) {
                                --start;
                            } else if(deleteForward && end < static_cast<int32_t>(text.size())) {
                                ++end;
                            }
                        }
                        if(start != end) {
                            text.erase(start, end - start);
                            replaceText(text, start);
                        }
                    } else {
                        int32_t target = m_cursorPosition;
                        if(!shift && hasSelection()) {
                            target = left ? MIN(m_cursorPosition, m_selectionAnchor) :
                                             MAX(m_cursorPosition, m_selectionAnchor);
                        } else {
                            target += left ? -1 : 1;
                        }
                        setCursorPosition(target, shift);
                    }
                    m_cursorRepeatHold = HOLD_TIME;
                    breakCursorFlashing = true;
                } else {
                    m_cursorRepeatHold -= Timer::deltaTime();
                }
            } else if(Input::isKeyUp(Input::KEY_BACKSPACE) || Input::isKeyUp(Input::KEY_DELETE) ||
                      Input::isKeyUp(Input::KEY_LEFT) || Input::isKeyUp(Input::KEY_RIGHT)) {
                m_cursorRepeatHold = 0.0f;
            }

            if(Input::isKeyDown(Input::KEY_HOME)) {
                setCursorPosition(0, shift);
                breakCursorFlashing = true;
            } else if(Input::isKeyDown(Input::KEY_END)) {
                setCursorPosition(textLength, shift);
                breakCursorFlashing = true;
            }

            if(!command && !alt && !editingKeyHeld && !clipboardKey &&
               !Input::isKeyDown(Input::KEY_HOME) && !Input::isKeyDown(Input::KEY_END)) {
                TString input = Input::inputString();
                std::string text = input.toStdString();
                text.erase(remove_if(text.begin(), text.end(), [](unsigned char c) { return c < 32; }), text.end());
                input = text;
                if(!input.isEmpty()) {
                    std::u32string editedText = m_text.toUtf32();
                    const int32_t start = MIN(m_cursorPosition, m_selectionAnchor);
                    const int32_t end = MAX(m_cursorPosition, m_selectionAnchor);
                    editedText.erase(start, end - start);
                    std::u32string inserted = input.toUtf32();
                    editedText.insert(start, inserted);
                    replaceText(editedText, start + static_cast<int32_t>(inserted.size()));
                    breakCursorFlashing = true;
                }
            }

            if(m_mouseSelecting) {
                if(!Input::isMouseButton(Input::MOUSE_LEFT)) {
                    m_mouseSelecting = false;
                } else {
                    Vector2 mousePosition = Widget::mousePosition();
                    Vector2 localPosition = mapFromGlobal(mousePosition.x, mousePosition.y);
                    setCursorPosition(cursorPositionAt(localPosition.x), true);
                    breakCursorFlashing = true;
                }
            }
        }

        if(breakCursorFlashing) {
            m_cursorVisible = true;
        } else {
            m_cursorBlinkCurrent += Timer::deltaTime();
            if(m_cursorBlinkCurrent >= m_cursorBlinkRate) {
                m_cursorBlinkCurrent = 0.0f;
                m_cursorVisible = !m_cursorVisible;
                repaint();
            }
        }
    } else {
        if(m_focused) {
            focusOut();
            m_focused = false;
            m_cursorVisible = false;
            m_mouseSelecting = false;
            m_selectionAnchor = m_cursorPosition;
            repaint();
        }
    }

    Widget::update(pos);

    bool hover = isHovered(pos);
    if(m_hovered != hover) {
        m_hovered = hover;
        repaint();
    }

    if(m_hovered) {
        if(!m_focused && Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN) {
            Widget::setFocusWidget(this);

            focusIn();
            m_focused = true;
            repaint();
        }
    }
}

bool LineEdit::onMouseDown(int x, int y) {
    A_UNUSED(y);
    if(!m_focused) {
        Widget::setFocusWidget(this);
        focusIn();
        m_focused = true;
        m_cursorVisible = true;
        repaint();
    }
    const bool shift = Input::isKey(Input::KEY_LEFT_SHIFT) || Input::isKey(Input::KEY_RIGHT_SHIFT);
    m_mouseSelecting = true;
    setCursorPosition(cursorPositionAt(static_cast<float>(x)), shift);
    return true;
}

bool LineEdit::onMouseUp(int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    m_mouseSelecting = false;
    return true;
}

bool LineEdit::hasSelection() const {
    return m_cursorPosition != m_selectionAnchor;
}

int32_t LineEdit::cursorPositionAt(float x) const {
    const std::u32string text = m_text.toUtf32();
    if(!m_font || text.empty()) {
        return 0;
    }

    const float textX = x - m_textPosition;
    int32_t nearestPosition = 0;
    float nearestDistance = std::abs(textX);
    for(int32_t i = 1; i <= static_cast<int32_t>(text.size()); ++i) {
        const float distance = std::abs(cursorAt(i) - textX);
        if(distance < nearestDistance) {
            nearestDistance = distance;
            nearestPosition = i;
        }
    }
    return nearestPosition;
}

void LineEdit::setCursorPosition(int32_t position, bool selecting) {
    const int32_t textLength = static_cast<int32_t>(m_text.toUtf32().size());
    position = std::max(0, std::min(position, textLength));
    const int32_t previousCursor = m_cursorPosition;
    const int32_t previousAnchor = m_selectionAnchor;

    if(selecting) {
        if(!hasSelection()) {
            m_selectionAnchor = m_cursorPosition;
        }
    } else {
        m_selectionAnchor = position;
    }

    m_cursorPosition = position;
    if(previousCursor != m_cursorPosition || previousAnchor != m_selectionAnchor) {
        recalcCursor();
    }
    if(!m_cursorVisible) {
        repaint();
    }
    m_cursorVisible = true;
    m_cursorBlinkCurrent = 0.0f;
}

void LineEdit::replaceText(const std::u32string &text, int32_t cursorPosition) {
    m_text = TString::fromUtf32(text);
    m_textDirty = true;
    const int32_t textLength = static_cast<int32_t>(text.size());
    m_cursorPosition = std::max(0, std::min(cursorPosition, textLength));
    m_selectionAnchor = m_cursorPosition;
    recalcCursor();
    repaint();
}

/**
    \internal
    Overrides the composeComponent method to create the text input component.
*/
void LineEdit::composeComponent() {
    Widget::composeComponent();

    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    setText("");

    Widget *rect = this;
    if(rect) {
        rect->setSize(Vector2(100.0f, 30.0f));
        rect->setPadding(Vector4(2.0f));
    }

    recalcCursor();
}
/*!
    \internal
    Recalculates the cursor position based on the current text and adjusts the text position accordingly.
*/
void LineEdit::recalcCursor() {
    float pos = (m_cursorPosition > 0) ? cursorAt(m_cursorPosition) : 0.0f;

    Widget *rect = this;
    float padding = rect->padding().w;
    m_cursorTransform[12] = pos;
    m_cursorTransform[13] = rect->size().y * 0.5f;
    m_textPosition = padding;

    float size = rect->clipRegion().z;
    float gap = pos + m_textPosition;
    if(gap > size) {
        m_textPosition += size - pos;
    } else if(gap < 0.0f) {
        m_textPosition -= gap;
    }
    m_cursorTransform[12] += m_textPosition;
    repaint();
}
/*!
    Returns a \a position for virtual cursor.
*/
float LineEdit::cursorAt(int position) const {
    if(!m_font) {
        return 0.0f;
    }
    std::u32string u32 = m_text.toUtf32();
    position = std::max(0, std::min(position, static_cast<int>(u32.size())));
    return m_font->textWidth(TString::fromUtf32(u32.substr(0, position)), m_fontSize, 0);
}
/*!
    \internal
*/
void LineEdit::fontUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        LineEdit *p = static_cast<LineEdit *>(ptr);
        p->m_textDirty = true;
        p->repaint();
    }
}
/*!
    \internal
*/
void LineEdit::materialUpdated(int state, void *ptr) {
    LineEdit *lineEdit = static_cast<LineEdit *>(ptr);
    if(!lineEdit) {
        return;
    }

    if(state == Material::ToBeDeleted) {
        if(lineEdit->m_fontMaterial && lineEdit->m_fontMaterial->material()->state() == state) {
            delete lineEdit->m_fontMaterial;
            lineEdit->m_fontMaterial = nullptr;
        }
        if(lineEdit->m_cursorMaterial && lineEdit->m_cursorMaterial->material()->state() == state) {
            delete lineEdit->m_cursorMaterial;
            lineEdit->m_cursorMaterial = nullptr;
        }
        if(lineEdit->m_selectionMaterial && lineEdit->m_selectionMaterial->material()->state() == state) {
            delete lineEdit->m_selectionMaterial;
            lineEdit->m_selectionMaterial = nullptr;
        }
    }

    lineEdit->m_textDirty = true;
    lineEdit->repaint();
}
