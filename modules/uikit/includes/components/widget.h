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
#ifndef WIDGET_H
#define WIDGET_H

#include <object.h>
#include <uikit.h>
#include <astring.h>

class CommandBuffer;
class StyleSheet;

class Canvas;

class KeyEvent {
public:
    KeyEvent(int keyCode, bool isPressed, bool isRepeat = false);

    int keyCode() const;
    bool isPressed() const;
    bool isRepeat() const;

    bool isShiftPressed() const;
    bool isControlPressed() const;
    bool isAltPressed() const;

private:
    int m_keyCode;
    bool m_isPressed;
    bool m_isRepeat;
};

class Layout;

class UIKIT_EXPORT Widget : public Object {
    A_OBJECT(Widget, Object, UI)

    A_PROPERTIES(
        A_PROPERTY(bool, enabled, Widget::isEnabled, Widget::setEnabled),
        A_PROPERTY(bool, visible, Widget::isVisible, Widget::setVisible),
        A_PROPERTY(Vector3, position, Widget::position, Widget::setPosition),
        A_PROPERTY(float, rotation, Widget::rotation, Widget::setRotation),
        A_PROPERTY(Vector3, scale, Widget::scale, Widget::setScale),
        A_PROPERTY(Vector2, size, Widget::size, Widget::setSize),
        A_PROPERTY(Vector2, minAnchors, Widget::minAnchors, Widget::setMinAnchors),
        A_PROPERTY(Vector2, maxAnchors, Widget::maxAnchors, Widget::setMaxAnchors),
        A_PROPERTY(Vector2, pivot, Widget::pivot, Widget::setPivot),
        A_PROPERTY(Vector4, margin, Widget::margin, Widget::setMargin),
        A_PROPERTY(Vector4, border, Widget::border, Widget::setBorder),
        A_PROPERTY(Vector4, padding, Widget::padding, Widget::setPadding)
    )
    A_METHODS(
        A_SLOT(Widget::lower),
        A_SLOT(Widget::raise)
    )
    A_ENUMS(
        A_ENUM(Orientation,
            A_VALUE(Horizontal),
            A_VALUE(Vertical)
        )
    )

public:
    enum Orientation {
        Horizontal,
        Vertical
    };
    enum SizePolicy {
        Fixed = 0,
        Preferred,
        Expanding
    };

public:
    Widget();
    ~Widget() override;

    static Widget *createWidget(const TString &type, const TString &name, Object *parent = nullptr);
    template<typename T>
    static T *createWidget(const TString &name, Object *parent = nullptr) {
        return dynamic_cast<T *>(createWidget(T::metaClass()->name(), name, parent));
    }

    TString style() const;

    const StringList &classes() const;
    void addClass(const TString &name);

    Widget *parentWidget() const;
    std::list<Widget *> &childWidgets();

    bool isEnabled() const;
    virtual void setEnabled(bool enabled);
    bool isVisible() const;
    void setVisible(bool visible);

    Vector3 position() const;
    void setPosition(const Vector3 &position);
    float rotation() const;
    void setRotation(float rotation);
    Vector3 scale() const;
    void setScale(const Vector3 &scale);
    Vector3 worldPosition() const;
    float worldRotation() const;
    Vector3 worldScale() const;
    const Matrix4 &localTransform() const;
    const Matrix4 &worldTransform() const;
    uint32_t hash() const;
    const std::list<Widget *> &children() const;
    Widget *parentTransform() const;
    void setParentTransform(Widget *parent, bool force = false);

    Vector2 size() const;
    void setSize(const Vector2 &size);
    Vector2 pivot() const;
    void setPivot(const Vector2 &pivot);
    Vector2 minAnchors() const;
    void setMinAnchors(const Vector2 &anchors);
    Vector2 maxAnchors() const;
    void setMaxAnchors(const Vector2 &anchors);
    void setAnchors(const Vector2 &minimum, const Vector2 &maximum);
    Vector4 margin() const;
    void setMargin(const Vector4 &margin);
    Vector4 border() const;
    void setBorder(const Vector4 &border);
    Vector4 padding() const;
    void setPadding(const Vector4 &padding);
    Vector2 mapFromGlobal(float x, float y);
    Layout *layout() const;
    void setLayout(Layout *layout);
    AABBox bound() const;
    int verticalPolicy() const;
    void setVerticalPolicy(SizePolicy policy);
    int horizontalPolicy() const;
    void setHorizontalPolicy(SizePolicy policy);
    Vector2 sizeHint() const;
    Vector4 clipRegion() const;

    virtual Canvas *canvas();

    bool isSubWidget() const;

    virtual void draw();

    virtual bool isHovered(const Vector2 &pos);

    virtual void update(const Vector2 &pos);

    static Widget *focusWidget();

    static bool setDragCandidate(Widget *source, const TString &mimeType, const TString &data);
    static bool isDragActive();
    static Widget *dragSource();
    static const TString &dragMimeType();
    static const TString &dragData();
    static Vector2 mousePosition();
    static void finishDrag(bool dropped);
    static void cancelDragCandidate(Widget *source = nullptr);

    void repaint();

    virtual bool onMouseDown(int x, int y) { A_UNUSED(x); A_UNUSED(y); return false; }
    virtual bool onMouseUp(int x, int y) { A_UNUSED(x); A_UNUSED(y); return false; }
    virtual bool onMouseMove(int x, int y) { A_UNUSED(x); A_UNUSED(y); return false; }
    virtual bool onMouseDoubleClick(int x, int y) { A_UNUSED(x); A_UNUSED(y); return false; }
    virtual bool onMouseWheel(int delta, bool horizontal) { A_UNUSED(delta); A_UNUSED(horizontal); return false; }
    virtual bool onDrop(const TString &mimeType, const TString &data, int x, int y) {
        A_UNUSED(mimeType); A_UNUSED(data); A_UNUSED(x); A_UNUSED(y); return false;
    }
    virtual bool onKeyPress(KeyEvent *event) { A_UNUSED(event); return false; }
    virtual bool onKeyRelease(KeyEvent *event) { A_UNUSED(event); return false; }

    Widget *subWidget(const TString &name) const;
    void setSubWidget(Widget *widget);
    void setParent(Object *parent, int32_t position = -1, bool force = false) override;
    const ObjectList &getChildren() const override;

public: // slots
    void lower();

    void raise();

protected:
    virtual void drawSub();

    virtual void boundChanged(const Vector2 &size);

    virtual void applyStyle();

    void onHierarchyUpdated();
    virtual void composeComponent();

    void dispatchKeyEvent(KeyEvent *event);
    void dispatchMouseEvent(const Vector2 &pos, Event::Type type, int button);
    void dispatchMouseWheelEvent(const Vector2 &pos, int delta, bool horizontal);
    virtual void onDragFinished(bool dropped) { A_UNUSED(dropped); }

    float styleLength(const TString &key, float value, bool &pixels);
    Vector2 styleBlock2Length(const TString &property, const Vector2 &value, bool &pixels);
    Vector4 styleBlock4Length(const TString &property, const Vector4 &value, bool &pixels);

    static void setFocusWidget(Widget *widget);

    void updateStyleProperty(const TString &name, const float *v, int32_t size);

protected:
    virtual void childAdded(Widget *child);

protected:
    std::map<TString, std::pair<uint32_t, TString>> m_styleRules;

    std::list<Widget *> m_childWidgets;

private:
    void addStyleRules(const std::map<TString, TString> &rules, uint32_t weight);
    void applyGeometryStyle();
    void setDirty();
    void cleanDirty() const;
    void cleanDirtySize() const;

private:
    friend class Canvas;
    friend class StyleSheet;
    friend class Layout;

    StringList m_classes;

    Canvas *m_canvas;

    Vector3 m_position;
    float m_rotation;
    Vector3 m_scale;
    mutable Matrix4 m_transform;
    mutable Matrix4 m_worldTransform;
    mutable float m_worldRotation;
    mutable Vector3 m_worldScale;
    mutable uint32_t m_hash;
    mutable bool m_dirty;

    mutable Vector2 m_size;
    Vector2 m_pivot;
    Vector2 m_minAnchors;
    Vector2 m_maxAnchors;
    Vector4 m_margin;
    Vector4 m_border;
    Vector4 m_padding;
    Layout *m_layout;
    Layout *m_attachedLayout;
    int m_verticalPolicy;
    int m_horizontalPolicy;
    bool m_enabled;
    bool m_visible;

    bool m_subWidget;

    static Widget *m_focusWidget;
    static Widget *m_dragSource;
    static TString m_dragMimeType;
    static TString m_dragData;
    static Vector2 m_mousePosition;
    static Vector2 m_dragStartPosition;
    static bool m_dragCandidate;
    static bool m_dragActive;

};

#endif // WIDGET_H
