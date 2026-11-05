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
#include "components/widget.h"

#include "components/canvas.h"
#include "components/layout.h"

#include "stylesheet.h"

#include <input.h>
#include <objectsystem.h>

namespace {
    const char *gCssSize("-uikit-size");
    const char *gCssWidth("width");
    const char *gCssHeight("height");
    const char *gCssPosition("-uikit-position");
    const char *gCssPivot("-uikit-pivot");
    const char *gCssMinAnchors("-uikit-min-anchors");
    const char *gCssMaxAnchors("-uikit-max-anchors");
    const char *gCssBorder("border-width");
    const char *gCssMargin("margin");
    const char *gCssPadding("padding");
    const char *gCssDisplay("display");

    enum {
        FLAG_ENABLED = 0x1,
        FLAG_VISIBLE = 0x2,
        FLAG_SUB_WIDGET = 0x4,
        FLAG_DIRTY = 0x8,
        FLAG_VPOLICY_SHIFT = 4,
        FLAG_HPOLICY_SHIFT = 6,
        FLAG_VPOLICY_MASK = 0x3 << FLAG_VPOLICY_SHIFT,
        FLAG_HPOLICY_MASK = 0x3 << FLAG_HPOLICY_SHIFT
    };
}

Widget *Widget::m_focusWidget = nullptr;
Widget *Widget::m_dragSource = nullptr;
TString Widget::m_dragMimeType;
TString Widget::m_dragData;
Vector2 Widget::m_mousePosition;
Vector2 Widget::m_dragStartPosition;
bool Widget::m_dragCandidate = false;
bool Widget::m_dragActive = false;

KeyEvent::KeyEvent(int keyCode, bool isPressed, bool isRepeat) :
    m_keyCode(keyCode),
    m_isPressed(isPressed),
    m_isRepeat(isRepeat) {

}

int KeyEvent::keyCode() const {
    return m_keyCode;
}
bool KeyEvent::isPressed() const {
    return m_isPressed;
}
bool KeyEvent::isRepeat() const {
    return m_isRepeat;
}

bool KeyEvent::isShiftPressed() const {
    return Input::isKey(Input::KEY_LEFT_SHIFT) || Input::isKey(Input::KEY_RIGHT_SHIFT);
}
bool KeyEvent::isControlPressed() const {
    return Input::isKey(Input::KEY_LEFT_CONTROL) || Input::isKey(Input::KEY_RIGHT_CONTROL);
}
bool KeyEvent::isAltPressed() const {
    return Input::isKey(Input::KEY_LEFT_ALT) || Input::isKey(Input::KEY_RIGHT_ALT);
}

/*!
    \class Widget
    \brief The Widget class is the base class of all user interface objects.
    \inmodule Gui

    The Widget class serves as the base class for all user interface objects, providing basic functionality for handling updates, drawing, and interaction.
    Internal methods are marked as internal and are intended for use within the framework rather than by external code.
*/

Widget::Widget() :
        m_position(0.0f),
        m_rotation(0.0f),
        m_scale(1.0f),
        m_worldTransform(),
        m_worldRotation(0.0f),
        m_worldScale(1.0f),
        m_hash(0),
        m_size(0.0f),
        m_pivot(0.5f),
        m_minimumSize(0.0f),
        m_anchors(0.5f),
        m_margin(0.0f),
        m_border(0.0f),
        m_padding(0.0f),
        m_icon(nullptr),
        m_canvas(nullptr),
        m_layout(nullptr),
        m_attachedLayout(nullptr),
        m_flags(FLAG_ENABLED | FLAG_VISIBLE | FLAG_DIRTY) {

}

Widget::~Widget() {
    repaint();

    if(Widget *parent = parentWidget()) {
        parent->m_childWidgets.remove(this);
    }

    if(m_dragSource == this) {
        m_dragSource = nullptr;
        m_dragMimeType.clear();
        m_dragData.clear();
        m_dragCandidate = false;
        m_dragActive = false;
    }

    if(m_attachedLayout) {
        m_attachedLayout->removeWidget(this);
    }
    delete m_layout;
}
/*!
    Returns a textual description of widget style.
*/
TString Widget::style() const {
    TString result;

    for(auto &it : m_styleRules) {
        if(it.second.first == 1000) {
            result += TString(it.first) + ": " + it.second.second + ";";
        }
    }

    return result;
}
/*!
    Returns a list of stylesheet class names attached to this widget.
*/
const StringList &Widget::classes() const {
    return m_classes;
}
/*!
    Adds a stylesheet class \a name attached to this widget.
*/
void Widget::addClass(const TString &name) {
    m_classes.push_back(name);
}
/*!
    \internal
    Internal method called to draw the widget.
*/
void Widget::draw() {
    drawSub();

    for(auto it : m_childWidgets) {
        if(!it->isSubWidget() && it->isVisible()) {
            it->draw();
        }
    }
}
/*!
    \internal
*/
bool Widget::isHovered(const Vector2 &pos) {
    Vector4 area(clipRegion());

    return (pos.x > area.x && pos.x < area.x + area.z && pos.y > area.y && pos.y < area.y + area.w);
}
/*!
    \internal
*/
void Widget::update(const Vector2 &pos) {
    for(auto it : m_childWidgets) {
        if(it->isEnabled() && it->isVisible()) {
            it->update(pos);
        }
    }
}
/*!
    \internal
    Marks parent canvas as dirty.
*/
void Widget::repaint() {
    Canvas *canvas = Widget::canvas();
    if(canvas) {
        canvas->markDirty();
    }
}

bool Widget::onMouseDown(int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    return false;
}

bool Widget::onMouseUp(int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    return false;
}

bool Widget::onMouseMove(int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    return false;
}

bool Widget::onMouseDoubleClick(int x, int y) {
    A_UNUSED(x);
    A_UNUSED(y);
    return false;
}

bool Widget::onMouseWheel(int delta, bool horizontal) {
    A_UNUSED(delta);
    A_UNUSED(horizontal);
    return false;
}

bool Widget::onDrop(const TString &mimeType, const TString &data, int x, int y) {
    A_UNUSED(mimeType);
    A_UNUSED(data);
    A_UNUSED(x);
    A_UNUSED(y);
    return false;
}

bool Widget::onKeyPress(KeyEvent *event) {
    A_UNUSED(event);
    return false;
}

bool Widget::onKeyRelease(KeyEvent *event) {
    A_UNUSED(event);
    return false;
}

/*!
    Sets current state of widget to \a enabled or disabled.
*/
void Widget::setEnabled(bool enabled) {
    if(isEnabled() != enabled) {
        m_flags = (m_flags & ~FLAG_ENABLED) | (enabled ? FLAG_ENABLED : 0);
    }
    repaint();
}

bool Widget::isEnabled() const {
    return (m_flags & FLAG_ENABLED) != 0;
}

bool Widget::isVisible() const {
    return (m_flags & FLAG_VISIBLE) != 0;
}

void Widget::setVisible(bool visible) {
    if(isVisible() != visible) {
        m_flags = (m_flags & ~FLAG_VISIBLE) | (visible ? FLAG_VISIBLE : 0);
        if(m_attachedLayout) {
            Layout *layout = m_attachedLayout;
            layout->invalidate();
            if(Widget *widget = layout->widget()) {
                widget->cleanDirty();
            }
        }
        repaint();
    }
}

Sprite *Widget::icon() const {
    return m_icon;
}

void Widget::setIcon(Sprite *icon) {
    m_icon = icon;
}

Widget *Widget::createWidget(const TString &type, const TString &name, Object *parent) {
    Widget *widget = dynamic_cast<Widget *>(ObjectSystem::objectCreate(type, name, parent));
    if(widget) {
        widget->composeComponent();
    }
    return widget;
}
/*!
    \internal
    Internal method called to draw the sub widget using the provided command buffer.
*/
void Widget::drawSub() {
    for(auto it : m_childWidgets) {
        if(it->isSubWidget() && it->isVisible()) {
            it->draw();
        }
    }
}
/*!
    Lowers the widget to the bottom of the widget's stack.

    \sa raise()
*/
void Widget::lower() {
    if(Widget *parent = parentWidget()) {
        parent->m_childWidgets.remove(this);
        parent->m_childWidgets.push_front(this);
        repaint();
    }
}
/*!
    Raises this widget to the top of the widget's stack.

    \sa lower()
*/
void Widget::raise() {
    if(Widget *parent = parentWidget()) {
        parent->m_childWidgets.remove(this);
        parent->m_childWidgets.push_back(this);
        repaint();
    }
}
/*!
    Callback to respond to changes in the widget's \a size.
*/
void Widget::boundChanged(const Vector2 &size) {
    A_UNUSED(size);
}
/*!
    Applies style settings assigned to widget.
*/
void Widget::applyStyle() {
    blockSignals(true);
    applyGeometryStyle();
    blockSignals(false);

    for(auto it : m_childWidgets) {
        it->applyStyle();
    }

    repaint();
}
/*!
    Returns the parent Widget.
*/
Widget *Widget::parentWidget() const {
    return dynamic_cast<Widget *>(Object::parent());
}
/*!
    Returns a list of child widgets;
*/
std::list<Widget *> &Widget::childWidgets() {
    return m_childWidgets;
}
/*!
    Returns the Canvas that is the root node in the given Widget hierarchy.
*/
Canvas *Widget::canvas() {
    if(m_canvas == nullptr) {
        if(Widget *parent = parentWidget()) {
            m_canvas = parent->canvas();
        }
    }
    return m_canvas;
}

/*!
    Returns true if widget is a part of complex widget; otherwise returns false.
*/
bool Widget::isSubWidget() const {
    return (m_flags & FLAG_SUB_WIDGET) != 0;
}
/*!
    Returns the application widget that has the keyboard input focus, or nullptr if no widget in this application has the focus.
*/
Widget *Widget::focusWidget() {
    return m_focusWidget;
}

bool Widget::setDragCandidate(Widget *source, const TString &mimeType, const TString &data) {
    if(!source || mimeType.isEmpty() || data.isEmpty() || m_dragActive ||
       (m_dragCandidate && m_dragSource != source)) {
        return false;
    }

    m_dragSource = source;
    m_dragMimeType = mimeType;
    m_dragData = data;
    m_dragStartPosition = m_mousePosition;
    m_dragCandidate = true;
    return true;
}

Vector2 Widget::mousePosition() {
    return m_mousePosition;
}

void Widget::finishDrag(bool dropped) {
    Widget *source = m_dragSource;
    const bool wasActive = m_dragActive;
    m_dragSource = nullptr;
    m_dragMimeType.clear();
    m_dragData.clear();
    m_dragCandidate = false;
    m_dragActive = false;
    if(wasActive && source) {
        source->onDragFinished(dropped);
    }
}

void Widget::cancelDragCandidate(Widget *source) {
    if(source && source != m_dragSource) {
        return;
    }
    if(!m_dragActive) {
        m_dragSource = nullptr;
        m_dragMimeType.clear();
        m_dragData.clear();
        m_dragCandidate = false;
    }
}
/*!
    Return a sub widget (a part of more complex widget) with specified \a name.
*/
Widget *Widget::subWidget(const TString &name) const {
    for(auto it : m_childWidgets) {
        if(it->isSubWidget()) {
            if(it->name() == name) {
                return it;
            } else {
                Widget *result = it->subWidget(name);
                if(result) {
                    return result;
                }
            }
        }
    }
    return nullptr;
}
/*!
    \internal
    Marks \a widget as a part of more complex widget.
*/
void Widget::setSubWidget(Widget *widget) {
    if(widget && widget->parentWidget() != this) {
        widget->setParent(this);
    }
    if(m_childWidgets.empty()) {
        onHierarchyUpdated();
    }

    if(!widget) {
        return;
    }
    Widget *current = subWidget(widget->name());
    if(current != widget) {
        widget->makeSubWidget(true);

        if(current) {
            current->makeSubWidget(false);
        }
    }
}
/*!
    \internal
*/
void Widget::makeSubWidget(bool enable) {
    m_flags = (m_flags & ~FLAG_SUB_WIDGET) | (enable ? FLAG_SUB_WIDGET : 0);
}
/*!
    \internal
    Internal method to set the widget that has the keyboard input focus.
*/
void Widget::setFocusWidget(Widget *widget) {
    m_focusWidget = widget;
}
/*!
    \internal
    Internal method to set the Widget component for the widget.
*/
/*!
    \internal
    Internal method called when the parent actor of the widget changes.
*/
void Widget::onHierarchyUpdated() {
    m_childWidgets.clear();
    for(Object *child : Object::getChildren()) {
        if(Widget *widget = dynamic_cast<Widget *>(child)) {
            m_childWidgets.push_back(widget);
        }
    }
}

void Widget::setParent(Object *parent, int32_t position, bool force) {
    if(parent == this) {
        return;
    }
    Widget *oldParent = parentWidget();
    Widget *newParent = dynamic_cast<Widget *>(parent);
    Vector3 worldPos;
    float worldRot = 0.0f;
    float worldScale = 1.0f;
    const bool preserveWorldTransform = oldParent != newParent && newParent && !force;
    if(preserveWorldTransform) {
        worldPos = worldPosition();
        worldRot = worldRotation();
        worldScale = Widget::worldScale();
    }
    Object::setParent(parent, position, force);
    if(oldParent) {
        oldParent->m_childWidgets.remove(this);
    }
    if(newParent) {
        auto insertAt = newParent->m_childWidgets.end();
        if(position >= 0 && static_cast<size_t>(position) < newParent->m_childWidgets.size()) {
            insertAt = std::next(newParent->m_childWidgets.begin(), position);
        }
        newParent->m_childWidgets.insert(insertAt, this);
        m_canvas = newParent->canvas();
    } else {
        m_canvas = nullptr;
    }

    if(preserveWorldTransform) {
        float parentScale = 1.0f / newParent->worldScale();
        Matrix4 parentRotation;
        parentRotation.rotate(Vector3(0.0f, 0.0f, -newParent->worldRotation()));
        m_position = (parentRotation * (worldPos - newParent->worldPosition())) * parentScale;
        m_rotation = worldRot - newParent->worldRotation();
        m_scale = worldScale * parentScale;
    }
    markSizeDirty();
    if(newParent && newParent->layout()) {
        if(m_attachedLayout != newParent->layout()) {
            newParent->layout()->addWidget(this);
        } else if(oldParent == newParent) {
            newParent->layout()->insertWidget(position, this);
        }
    }
    if(oldParent && oldParent != newParent && oldParent->layout() &&
       m_attachedLayout == oldParent->layout()) {
        oldParent->layout()->removeWidget(this);
    }
    if(newParent && oldParent != newParent) {
        newParent->childAdded(this);
    }
    if(oldParent && oldParent != newParent) {
        oldParent->repaint();
    }
    repaint();
}

const Object::ObjectList &Widget::getChildren() const {
    return Object::getChildren();
}
/*!
    \internal
    Internal method to compose the widget component, creating and setting the Widget.
*/
void Widget::composeComponent() {
}
/*!
    \internal
    This call back can be used to modify some properties for a new \a child.
*/
void Widget::childAdded(Widget *child) {
    A_UNUSED(child);
}
/*!
    \internal
    Applies a new stylesheet \a rules to the widget.
    A \a wieght parameter required to select rules between new one and existant.
*/
void Widget::addStyleRules(const std::map<TString, TString> &rules, uint32_t weight) {
    for(auto rule : rules) {
        auto it = m_styleRules.find(rule.first);
        if(it == m_styleRules.end() || it->second.first <= weight) {
            m_styleRules[rule.first] = std::make_pair(weight, rule.second);
        }
    }

    for(auto it : m_childWidgets) {
        if(it->isSubWidget()) {
            it->addStyleRules(rules, weight);
        }
    }
}
/*!
    \internal
    Returns length for the stylesheet \a property.
    Default \a value will be used in case of property will not be found.
    Parameter \a pixels contains a definition of unit of measurement.
*/
float Widget::styleLength(const TString &property, float value, bool &pixels) {
    auto it = m_styleRules.find(property);
    if(it != m_styleRules.end()) {
        return StyleSheet::toLength(it->second.second, pixels);
    } else {
        pixels = true;
    }
    return value;
}
/*!
    \internal
    Returns length block for the stylesheet \a property.
    Default \a value will be used in case of property will not be found.
    Parameter \a pixels contains a definition of unit of measurement.
*/
Vector2 Widget::styleBlock2Length(const TString &property, const Vector2 &value, bool &pixels) {
    Vector2 result(value);

    auto it = m_styleRules.find(property);
    if(it != m_styleRules.end()) {
        auto list = it->second.second.split(' ');

        Vector2 value;
        if(list.size() == 1) {
            result.x = value.y = list.front().toFloat();
        } else {
            result.x = list.front().toFloat();
            result.y = list.back().toFloat();
        }
    }

    return result;
}
/*!
    \internal
    Returns length block for the stylesheet \a property.
    Default \a value will be used in case of property will not be found.
    Parameter \a pixels contains a definition of unit of measurement.
*/
Vector4 Widget::styleBlock4Length(const TString &property, const Vector4 &value, bool &pixels) {
    Vector4 result(value);

    auto it = m_styleRules.find(property);
    if(it != m_styleRules.end()) {
        auto array = it->second.second.split(' ');
        switch(array.size()) {
            case 1: {
                result = Vector4(StyleSheet::toLength(array.front(), pixels));
            } break;
            case 2: {
                result.x = result.z = StyleSheet::toLength(array.front(), pixels);
                result.y = result.w = StyleSheet::toLength(*std::next(array.begin(), 1), pixels);
            } break;
            case 3: {
                result.y = StyleSheet::toLength(array.front(), pixels);
                result.z = result.x = StyleSheet::toLength(*std::next(array.begin(), 1), pixels);
                result.w = StyleSheet::toLength(*std::next(array.begin(), 2), pixels);
            } break;
            case 4: {
                result.x = StyleSheet::toLength(array.front(), pixels);
                result.y = StyleSheet::toLength(*std::next(array.begin(), 1), pixels);
                result.z = StyleSheet::toLength(*std::next(array.begin(), 2), pixels);
                result.w = StyleSheet::toLength(*std::next(array.begin(), 3), pixels);
            } break;
            default: break;
        }
    }

    return result;
}
/*!
    \internal
*/
void Widget::updateStyleProperty(const TString &name, const float *v, int32_t size) {
    if(!isSignalsBlocked() && !isSubWidget()) {
        TString data;
        for(uint32_t i = 0; i < size; i++) {
            data += TString::number((int)v[i]) + "px ";
        }
        data.removeLast();
        StyleSheet::setStyleProperty(this, name, data);
    }
}
/*!
    \internal
*/
void Widget::dispatchKeyEvent(KeyEvent *event) {
    if(focusWidget() == this) {
        if(event->isPressed()) {
            onKeyPress(event);
        } else {
            onKeyRelease(event);
        }
        return;
    }

    for(auto child : m_childWidgets) {
        child->dispatchKeyEvent(event);
    }
}

bool Widget::dispatchMouseWheelEvent(const Vector2 &pos, int delta, bool horizontal) {
    if(!isEnabled() || !isVisible()) {
        return false;
    }

    for(int pass = 0; pass < 2; ++pass) {
        const bool subWidget = pass != 0;
        for(auto it = m_childWidgets.rbegin(); it != m_childWidgets.rend(); ++it) {
            Widget *child = *it;
            if(child && child->isSubWidget() == subWidget && child->isEnabled() &&
               child->isVisible() && child->isHovered(pos)) {
                if(child->dispatchMouseWheelEvent(pos, delta, horizontal)) {
                    return true;
                }
            }
        }
    }

    if(isHovered(pos)) {
        return onMouseWheel(delta, horizontal);
    }
    return false;
}

bool Widget::dispatchMouseEvent(const Vector2 &pos, Event::Type type, int button) {
    m_mousePosition = pos;
    if(type == Event::MouseDown && button == Input::MOUSE_RIGHT) {
        return dispatchContextMenuEvent(pos);
    }
    if(!isEnabled() || !isVisible()) {
        return false;
    }

    if(type == Event::MouseMove && m_dragCandidate && !m_dragActive &&
       Input::isMouseButton(Input::MOUSE_LEFT)) {
        float dx = pos.x - m_dragStartPosition.x;
        float dy = pos.y - m_dragStartPosition.y;
        if(dx * dx + dy * dy >= 16.0f) {
            m_dragActive = true;
        }
    }

    for(int pass = 0; pass < 2; ++pass) {
        const bool subWidget = pass != 0;
        for(auto it = m_childWidgets.rbegin(); it != m_childWidgets.rend(); ++it) {
            Widget *child = *it;
            if(child && child->isSubWidget() == subWidget && child->isEnabled() &&
               child->isVisible() && child->isHovered(pos)) {
                if(child->dispatchMouseEvent(pos, type, button)) {
                    return true;
                }
            }
        }
    }

    if(isHovered(pos)) {
        Vector2 localPos = mapFromGlobal(pos.x, pos.y);
        localPos.y = size().y - localPos.y;

        switch(type) {
            case Event::MouseDown:
                return onMouseDown(static_cast<int>(localPos.x), static_cast<int>(localPos.y));
            case Event::MouseUp:
                if(button == Input::MOUSE_LEFT && m_dragActive &&
                   onDrop(m_dragMimeType, m_dragData, static_cast<int>(localPos.x), static_cast<int>(localPos.y))) {
                    finishDrag(true);
                    return true;
                }
                return onMouseUp(static_cast<int>(localPos.x), static_cast<int>(localPos.y));
            case Event::MouseMove:
                return onMouseMove(static_cast<int>(localPos.x), static_cast<int>(localPos.y));
            case Event::MouseDoubleClick:
                return onMouseDoubleClick(static_cast<int>(localPos.x), static_cast<int>(localPos.y));
            default:
                return false;
        }
    }
    return false;
}

bool Widget::dispatchContextMenuEvent(const Vector2 &pos) {
    if(!isEnabled() || !isVisible() || !isHovered(pos)) {
        return false;
    }

    for(auto it = m_childWidgets.rbegin(); it != m_childWidgets.rend(); ++it) {
        Widget *child = *it;
        if(child && child->dispatchContextMenuEvent(pos)) {
            return true;
        }
    }

    if(isSubWidget()) {
        return false;
    }

    Vector2 localPos = mapFromGlobal(pos.x, pos.y);
    Vector2 eventPos(localPos.x, size().y - localPos.y);
    onContextMenuRequested(eventPos);
    contextMenuRequested(localPos);
    return true;
}

void Widget::onContextMenuRequested(const Vector2 &position) {
    A_UNUSED(position);
}

void Widget::contextMenuRequested(const Vector2 &position) {
    emitSignal(_SIGNAL(contextMenuRequested(Vector2)), position);
}

/*!
    Changes the \a position of the Widget in local space.
*/
void Widget::setPosition(const Vector3 &position) {
    if(m_position != position) {
        m_position = position;
        markDirty();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) {
            updateStyleProperty(gCssPosition, position.v, 2);
        }
#endif
        repaint();
    }
}

/*!
    Returns the position of the Widget in local space.
*/
Vector3 Widget::position() const {
    return m_position;
}

/*!
    Returns the rotation of the Widget in local space.
*/
float Widget::rotation() const {
    return m_rotation;
}

/*!
    Returns the scale of the Widget in local space.
*/
float Widget::scale() const {
    return m_scale;
}

/*!
    Changes the \a rotation of the Widget in local space.
*/
void Widget::setRotation(float rotation) {
    if(m_rotation != rotation) {
        m_rotation = rotation;
        markDirty();
        repaint();
    }
}

/*!
    Changes the \a scale of the Widget in local space.
*/
void Widget::setScale(float scale) {
    if(m_scale != scale) {
        m_scale = scale;
        markDirty();
        repaint();
    }
}

/*!
    Returns the world position of the Widget.
*/
Vector3 Widget::worldPosition() const { cleanDirty(); return m_worldTransform.position(); }
/*!
    Returns the world rotation of the Widget.
*/
float Widget::worldRotation() const {
    cleanDirty(); return m_worldRotation;
}
/*!
    Returns the world scale of the Widget.
*/
float Widget::worldScale() const {
    cleanDirty(); return m_worldScale;
}
/*!
    Returns the world transform of the Widget.
*/
const Matrix4 &Widget::worldTransform() const { cleanDirty(); return m_worldTransform; }
uint32_t Widget::hash() const { cleanDirty(); return m_hash; }
const std::list<Widget *> &Widget::children() const { return m_childWidgets; }

/*!
    Returns the size of the associated UI element.
*/
Vector2 Widget::size() const { return m_size; }

/*!
    Sets the \a size of the Widget.
*/
void Widget::setSize(const Vector2 &size) {
    Vector2 boundedSize(MAX(size.x, m_minimumSize.x), MAX(size.y, m_minimumSize.y));
    if(m_size != boundedSize) {
        if(m_attachedLayout || abs(m_anchors.x - m_anchors.z) <= EPSILON) {
            m_size.x = boundedSize.x;
        }
        if(m_attachedLayout || abs(m_anchors.y - m_anchors.w) <= EPSILON) {
            m_size.y = boundedSize.y;
        }
        if(m_attachedLayout) {
            m_attachedLayout->invalidate();
        }
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) {
            if(horizontalPolicy() == Fixed && verticalPolicy() == Fixed) {
                updateStyleProperty(gCssSize, m_size.v, 2);
            } else if(horizontalPolicy() == Fixed) {
                updateStyleProperty(gCssWidth, &m_size.x, 1);
            } else if(verticalPolicy() == Fixed) {
                updateStyleProperty(gCssHeight, &m_size.y, 1);
            }
        }
#endif
        markSizeDirty();
        boundChanged(m_size);
        repaint();
    }
}

/*!
    Returns the minimum size of the Widget.
*/
Vector2 Widget::minimumSize() const {
    return m_minimumSize;
}

/*!
    Sets the minimum \a size of the Widget.
*/
void Widget::setMinimumSize(const Vector2 &size) {
    Vector2 minimum(MAX(0.0f, size.x), MAX(0.0f, size.y));
    if(m_minimumSize != minimum) {
        m_minimumSize = minimum;
        if(m_attachedLayout) {
            m_attachedLayout->invalidate();
        }
        setSize(m_size);
        repaint();
    }
}

/*!
    Returns the pivot point of the Widget.
*/
Vector2 Widget::pivot() const {
    return m_pivot;
}

/*!
    Returns the minimum anchors of the Widget.
*/
Vector2 Widget::minAnchors() const {
    return Vector2(m_anchors.x, m_anchors.y);
}

/*!
    Returns the maximum anchors of the Widget.
*/
Vector2 Widget::maxAnchors() const {
    return Vector2(m_anchors.z, m_anchors.w);
}

/*!
    Returns the margin offsets of the Widget.
    The Vector4 contains offsets in top, right, bottom and left order.
*/
Vector4 Widget::margin() const {
    return m_margin;
}

/*!
    Returns the border width of the Widget.
    The Vector4 contains border widths in top, right, bottom and left order.
*/
Vector4 Widget::border() const { return m_border; }
/*!
    Returns the padding offset of the Widget.
    The Vector4 contains padding offsets in top, right, bottom and left order.
*/
Vector4 Widget::padding() const { return m_padding; }

/*!
    Sets the \a pivot point of the Widget.
*/
void Widget::setPivot(const Vector2 &pivot) {
    if(m_pivot != pivot) {
        m_pivot = pivot;
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markDirty();
        repaint();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) updateStyleProperty(gCssPivot, m_pivot.v, 2);
#endif
    }
}

/*!
    Sets the minimum \a anchors of the Widget.
*/
void Widget::setMinAnchors(const Vector2 &anchors) {
    if(minAnchors() != anchors) {
        m_anchors.x = anchors.x;
        m_anchors.y = anchors.y;
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markSizeDirty();
        repaint();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) updateStyleProperty(gCssMinAnchors, m_anchors.v, 2);
#endif
    }
}

/*!
    Sets the maximum \a anchors of the Widget.
*/
void Widget::setMaxAnchors(const Vector2 &anchors) {
    if(maxAnchors() != anchors) {
        m_anchors.z = anchors.x;
        m_anchors.w = anchors.y;
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markSizeDirty();
        repaint();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) updateStyleProperty(gCssMaxAnchors, &m_anchors.v[2], 2);
#endif
    }
}

/*!
    Sets both the \a minimum and \a maximum anchors of the Widget.
*/
void Widget::setAnchors(const Vector2 &minimum, const Vector2 &maximum) {
    const bool minChanged = minAnchors() != minimum;
    const bool maxChanged = maxAnchors() != maximum;
    if(minChanged || maxChanged) {
        m_anchors.x = minimum.x;
        m_anchors.y = minimum.y;
        m_anchors.z = maximum.x;
        m_anchors.w = maximum.y;
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markSizeDirty();
        repaint();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) {
            if(minChanged) updateStyleProperty(gCssMinAnchors, m_anchors.v, 2);
            if(maxChanged) updateStyleProperty(gCssMaxAnchors, &m_anchors.v[2], 2);
        }
#endif
    }
}
/*!
    Sets the top, right, bottom and left \a margin offsets of the Widget.
*/
void Widget::setMargin(const Vector4 &margin) {
    if(m_margin != margin) {
        m_margin = margin;
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markSizeDirty();
        repaint();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) updateStyleProperty(gCssMargin, m_margin.v, 4);
#endif
    }
}

/*!
    Sets the top, right, bottom and left \a border width of the Widget.
*/
void Widget::setBorder(const Vector4 &border) {
    if(m_border != border) {
        m_border = border;
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markSizeDirty();
        repaint();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) updateStyleProperty(gCssBorder, m_border.v, 4);
#endif
    }
}

/*!
    Sets the top, right, bottom and left \a padding offsets of the Widget.
*/
void Widget::setPadding(const Vector4 &padding) {
    if(m_padding != padding) {
        m_padding = padding;
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markSizeDirty();
        repaint();
#ifdef SHARED_DEFINE
        if(!isSignalsBlocked()) updateStyleProperty(gCssPadding, m_padding.v, 4);
#endif
    }
}

/*!
    Translates the global screen \a x and \a y coordinates to widget space.
*/
Vector2 Widget::mapFromGlobal(float x, float y) {
    cleanDirty();
    return Vector2(x - m_worldTransform[12], y - m_worldTransform[13]);
}

/*!
    Returns the layout assigned to the Widget.
*/
Layout *Widget::layout() const {
    return m_layout;
}
/*!
    Sets the \a layout for the Widget.
*/
void Widget::setLayout(Layout *layout) {
    if(m_layout == layout) return;
    if(m_layout) {
        m_layout->setWidget(nullptr);
        delete m_layout;
    }
    m_layout = layout;
    if(m_layout) {
        m_layout->setWidget(this);
        for(Widget *child : m_childWidgets) m_layout->addWidget(child);
        m_layout->invalidate();
        cleanDirty();
    } else {
        if(m_attachedLayout) m_attachedLayout->invalidate();
        markSizeDirty();
        repaint();
    }
}

/*!
    \internal
    Internal method that returns the axis-aligned bounding box (AABBox).
*/
AABBox Widget::bound() const {
    AABBox result;
    result.extent = Vector3(m_size * 0.5f, 0.0f);
    result.center = result.extent;
    return result * worldTransform();
}

/*!
    Returns vertical size policy.
*/
int Widget::verticalPolicy() const {
    return (m_flags & FLAG_VPOLICY_MASK) >> FLAG_VPOLICY_SHIFT;
}

/*!
    Returns horizontal size policy.
*/
int Widget::horizontalPolicy() const {
    return (m_flags & FLAG_HPOLICY_MASK) >> FLAG_HPOLICY_SHIFT;
}

/*!
    Sets vertical size \a policy.
*/
void Widget::setVerticalPolicy(SizePolicy policy) {
    m_flags = (m_flags & ~FLAG_VPOLICY_MASK) | ((policy & 0x3) << FLAG_VPOLICY_SHIFT);
    if(m_attachedLayout) m_attachedLayout->invalidate();
    markSizeDirty();
    repaint();
}
/*!
    Sets horizontal size \a policy.
*/
void Widget::setHorizontalPolicy(SizePolicy policy) {
    m_flags = (m_flags & ~FLAG_HPOLICY_MASK) | ((policy & 0x3) << FLAG_HPOLICY_SHIFT);
    if(m_attachedLayout) m_attachedLayout->invalidate();
    markSizeDirty();
    repaint();
}

/*!
    Returns the size recommended to contain all visible content.
*/
Vector2 Widget::sizeHint() const {
    Vector2 hint(m_layout ? m_layout->sizeHint() : m_size);
    return Vector2(MAX(hint.x, m_minimumSize.x), MAX(hint.y, m_minimumSize.y));
}

/*!
    Returns the internal scissor area. All content outside of this area will not be rendered.
*/
Vector4 Widget::clipRegion() const {
    cleanDirty();
    Vector4 offsets(m_border + m_padding);
    return Vector4(m_worldTransform[12] + offsets.w,
                   m_worldTransform[13] + offsets.z,
                   (m_size.x - offsets.y - offsets.w) * m_worldScale,
                   (m_size.y - offsets.x - offsets.z) * m_worldScale);
}

bool Widget::isDirty() const {
    return (m_flags & FLAG_DIRTY) != 0;
}

void Widget::setDirty(bool dirty) const {
    m_flags = (m_flags & ~FLAG_DIRTY) | (dirty ? FLAG_DIRTY : 0);
}

void Widget::markDirty() {
    setDirty(true);
    for(Widget *child : m_childWidgets) {
        child->markDirty();
    }
}

void Widget::markSizeDirty() {
    setDirty(true);
    const Vector2 oldSize(m_size);
    cleanDirtySize();
    if(m_layout) {
        const Vector4 inset(m_padding + m_border);
        const Vector2 availableSize(MAX(0.0f, m_size.x - inset.y - inset.w),
                                    MAX(0.0f, m_size.y - inset.x - inset.z));
        m_layout->solveItemsDimension(availableSize.x, true, false);
        m_layout->solveItemsDimension(availableSize.y, false, false);
        m_layout->solveItemsPosition(m_size.y, Vector2(inset.w, inset.z));
    }
    if(m_size != oldSize) {
        boundChanged(m_size);
        repaint();
    }
    for(Widget *child : m_childWidgets) {
        child->markSizeDirty();
    }
}

void Widget::setLayoutSize(const Vector2 &size) {
    Vector2 boundedSize(MAX(size.x, m_minimumSize.x), MAX(size.y, m_minimumSize.y));
    if(m_size != boundedSize) {
        m_size = boundedSize;
        markSizeDirty();
        boundChanged(m_size);
        repaint();
    }
}

void Widget::cleanDirtySize() {
    const bool fitWidth = abs(minAnchors().x - maxAnchors().x) > EPSILON;
    const bool fitHeight = abs(minAnchors().y - maxAnchors().y) > EPSILON;
    if(m_layout && ((!fitWidth && horizontalPolicy() == Preferred) ||
                    (!fitHeight && verticalPolicy() == Preferred))) {
        Vector2 hint(m_layout->sizeHint());
        if(!fitWidth && horizontalPolicy() == Preferred) {
            m_size.x = hint.x;
        }
        if(!fitHeight && verticalPolicy() == Preferred) {
            m_size.y = hint.y;
        }
    }
    if(m_attachedLayout == nullptr && (fitWidth || fitHeight)) {
        Vector2 parentSize;
        if(Widget *parent = parentWidget()) {
            parentSize = parent->size();
            Vector4 parentBorder(parent->border());
            parentSize -= Vector2(parentBorder.y + parentBorder.w,
                                  parentBorder.x + parentBorder.z);
        }
        if(fitWidth) {
            m_size.x = parentSize.x * (maxAnchors().x - minAnchors().x) -
                       (m_margin[1] + m_margin[3]);
        }
        if(fitHeight) {
            m_size.y = parentSize.y * (maxAnchors().y - minAnchors().y) -
                       (m_margin[0] + m_margin[2]);
        }
    }
    m_size.x = MAX(m_size.x, m_minimumSize.x);
    m_size.y = MAX(m_size.y, m_minimumSize.y);
}

void Widget::cleanDirty() const {
    if(!isDirty()) return;
    Matrix4 transform;
    transform.translate(m_position);
    Matrix4 rotation;
    rotation.rotate(Vector3(0.0f, 0.0f, m_rotation));
    transform *= rotation;
    Matrix4 scale;
    scale.scale(m_scale);
    transform *= scale;
    m_worldTransform = transform;
    m_worldRotation = m_rotation;
    m_worldScale = m_scale;
    Widget *parent = parentWidget();
    if(parent) {
        m_worldScale = parent->worldScale() * m_worldScale;
        m_worldRotation = parent->worldRotation() + m_worldRotation;
        m_worldTransform = parent->worldTransform() * transform;
    }

    if(m_attachedLayout == nullptr && parent) {
        Vector2 anchors(minAnchors() + maxAnchors());
        Vector2 parentCenter(anchors * parent->m_size * 0.5f);
        float x = abs(minAnchors().x - maxAnchors().x) > EPSILON ?
            parent->m_size.x * minAnchors().x + m_margin[3] :
            parentCenter.x / parent->m_scale - m_size.x * m_pivot.x + m_margin[3];
        float y = abs(minAnchors().y - maxAnchors().y) > EPSILON ?
            parent->m_size.y * minAnchors().y + m_margin[2] :
            parentCenter.y / parent->m_scale - m_size.y * m_pivot.y + m_margin[2];
        transform[12] += x;
        transform[13] += y;
        m_worldTransform = parent->worldTransform() * transform;
    }
    m_hash = 16;
    for(int i = 0; i < 16; ++i) Mathf::hashCombine(m_hash, m_worldTransform[i]);
    setDirty(false);
}

void Widget::applyGeometryStyle() {
    blockSignals(true);
    bool pixels;
    Vector2 pos = styleBlock2Length(gCssPosition, Vector2(m_position), pixels);
    pos.x = styleLength("top", pos.x, pixels);
    pos.y = styleLength("left", pos.y, pixels);
    setPosition(Vector3(pos, 0.0f));

    Vector2 sizeValue = styleBlock2Length(gCssSize, m_size, pixels);
    sizeValue.x = styleLength(gCssWidth, sizeValue.x, pixels);
    sizeValue.y = styleLength(gCssHeight, sizeValue.y, pixels);
    setSize(sizeValue);
    setPivot(styleBlock2Length(gCssPivot, m_pivot, pixels));
    setMinAnchors(styleBlock2Length(gCssMinAnchors, minAnchors(), pixels));
    setMaxAnchors(styleBlock2Length(gCssMaxAnchors, maxAnchors(), pixels));

    Vector4 borderValue = styleBlock4Length(gCssBorder, m_border, pixels);
    borderValue.x = styleLength("border-top-width", borderValue.x, pixels);
    borderValue.y = styleLength("border-right-width", borderValue.y, pixels);
    borderValue.z = styleLength("border-bottom-width", borderValue.z, pixels);
    borderValue.w = styleLength("border-left-width", borderValue.w, pixels);
    setBorder(borderValue);

    Vector4 marginValue = styleBlock4Length(gCssMargin, m_margin, pixels);
    marginValue.x = styleLength("margin-top", marginValue.x, pixels);
    marginValue.y = styleLength("margin-right", marginValue.y, pixels);
    marginValue.z = styleLength("margin-bottom", marginValue.z, pixels);
    marginValue.w = styleLength("margin-left", marginValue.w, pixels);
    setMargin(marginValue);

    Vector4 paddingValue = styleBlock4Length(gCssPadding, m_padding, pixels);
    paddingValue.x = styleLength("padding-top", paddingValue.x, pixels);
    paddingValue.y = styleLength("padding-right", paddingValue.y, pixels);
    paddingValue.z = styleLength("padding-bottom", paddingValue.z, pixels);
    paddingValue.w = styleLength("padding-left", paddingValue.w, pixels);
    setPadding(paddingValue);

    auto it = m_styleRules.find(gCssDisplay);
    if(it != m_styleRules.end()) {
        if(it->second.second == "none") {
            setVisible(false);
        } else {
            setVisible(true);
            if(!m_layout) {
                Layout *next = new Layout;
                if(it->second.second == "block") next->setOrientation(Vertical);
                else if(it->second.second == "inline") next->setOrientation(Horizontal);
                setLayout(next);
            }
        }
    }
    blockSignals(false);
}
