#include "components/abstractbutton.h"

#include "components/recttransform.h"
#include "components/canvas.h"

#include <components/actor.h>
#include <components/textrender.h>

#include <resources/stylesheet.h>

#include <input.h>
#include <timer.h>
#include <log.h>

namespace {
    const float gCorner = 4.0f;

    const char *gCssBackgroundColor("background-color");

    const char *gOverride("mainTexture");
    const char *gColor("mainColor");

    const char *gBackgroundColor("backgroundColor");
    const char *gBorderWidth("borderWidth");
    const char *gBorderRadius("borderRadius");

    const char *gTopColor("topColor");
    const char *gRightColor("rightColor");
    const char *gBottomColor("bottomColor");
    const char *gLeftColor("leftColor");

    const char *gDefaultSprite(".embedded/DefaultUI.shader");
    const char *gDefaultFrame(".embedded/Frame.shader");
}
/*!
    \class AbstractButton
    \brief The AbstractButton class represents a base class for interactive buttons in a graphical user interface.
    \inmodule Gui

    The AbstractButton class provides a foundation for creating interactive buttons within a graphical user interface.
    It allows customization of various visual properties and handles user interaction events.
    Internal methods are marked as internal and are intended for use within the framework rather than by external code.
*/

AbstractButton::AbstractButton() :
        m_borderRadius(0.0f),
        m_borderColor(0.8f),
        m_backgroundColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f)),
        m_highlightedColor(Vector4(0.6f, 0.6f, 0.6f, 1.0f)),
        m_pressedColor(Vector4(0.7f, 0.7f, 0.7f, 1.0f)),
        m_currentColor(m_backgroundColor),
        m_backgroundImage(nullptr),
        m_backgroundMesh(nullptr),
        m_imageMaterial(nullptr),
        m_frameMaterial(nullptr),
        m_fadeDuration(0.1f),
        m_currentFade(1.0f),
        m_hovered(false),
        m_checkable(false),
        m_checked(false),
        m_exclusive(false),
        m_dirtyBackground(true)  {

    Material *spriteMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(spriteMaterial) {
        m_imageMaterial = spriteMaterial->createInstance();
        m_imageMaterial->setVector4(gColor, &m_backgroundColor);
    }

    Material *frameMaterial = Engine::loadResource<Material>(gDefaultFrame);
    if(frameMaterial) {
        m_frameMaterial = frameMaterial->createInstance();

        Vector4 width(0.0f);
        m_frameMaterial->setVector4(gBorderWidth, &width);
        m_frameMaterial->setVector4(gBorderRadius, &m_borderRadius);
        m_frameMaterial->setVector4(gTopColor, &m_borderColor);
        m_frameMaterial->setVector4(gRightColor, &m_borderColor);
        m_frameMaterial->setVector4(gBottomColor, &m_borderColor);
        m_frameMaterial->setVector4(gLeftColor, &m_borderColor);
        m_frameMaterial->setVector4(gBackgroundColor, &m_backgroundColor);
    }
}

AbstractButton::~AbstractButton() {
    delete m_imageMaterial;
    delete m_frameMaterial;

    delete m_backgroundMesh;
}

/*!
    \internal
*/
void AbstractButton::draw() {
    RectTransform *rect = rectTransform();
    if(m_dirtyBackground) {
        if(m_backgroundImage) {
            m_backgroundMesh = Engine::objectCreate<Mesh>();
            m_backgroundMesh->makeDynamic();

            Vector2 size(rect->size());
            m_backgroundImage->composeMesh(m_backgroundMesh, Sprite::Sliced, size);

            m_imageMaterial->setTexture(gOverride, m_backgroundImage->texture());
        }
        m_dirtyBackground = false;
    }

    Canvas *canvas = AbstractButton::canvas();
    if(m_backgroundImage) {
        Matrix4 mat(rect->worldTransform());

        const Vector3Vector &verts(m_backgroundMesh->vertices());
        Vector2 scl(rect->worldScale());
        mat[12] -= verts[0].x * scl.x;
        mat[13] -= verts[0].y * scl.y;

        uint32_t hash = rect->hash();
        Mathf::hashCombine(hash, mat[12]);
        Mathf::hashCombine(hash, mat[13]);

        m_imageMaterial->setTransform(mat, 0, hash);

        canvas->drawMesh(m_backgroundMesh, m_imageMaterial);
    } else {
        canvas->drawRect(m_frameMaterial, rect);
    }

    Widget::draw();
}
/*!
    \internal
*/
void AbstractButton::setHovered(bool hover, bool instant) {
    if(m_hovered != hover) {
        if(!hover && instant) {
            updateBackgroundColor(m_checked ? m_pressedColor : m_backgroundColor);
        } else {
            m_currentFade = 0.0f;
        }
        m_hovered = hover;
    }
}

/*!
    Returns background image.
*/
Sprite *AbstractButton::backgroundImage() const {
    return m_backgroundImage;
}
/*!
    Sets background \a image.
*/
void AbstractButton::setBackgroundImage(Sprite *image) {
    if(m_backgroundImage != image) {
        m_backgroundImage = image;

        m_dirtyBackground = true;
    }
}
/*!
    Returns the corners radiuses of button.
*/
Vector4 AbstractButton::corners() const {
    return m_borderRadius;
}
/*!
    Sets the \a corners radiuses of button.
*/
void AbstractButton::setCorners(const Vector4 &corners) {
    m_borderRadius = corners;

    if(m_frameMaterial) {
        RectTransform *rect = rectTransform();
        if(rect) {
            Vector4 normCorners(m_borderRadius / rect->size().y);
            m_frameMaterial->setVector4(gBorderRadius, &normCorners);
        }
    }
}

Vector4 AbstractButton::borderColor() const {
    return m_borderColor;
}

void AbstractButton::setBorderColor(const Vector4 &color) {
    m_borderColor = color;

    if(m_frameMaterial) {
        m_frameMaterial->setVector4(gTopColor, &m_borderColor);
        m_frameMaterial->setVector4(gRightColor, &m_borderColor);
        m_frameMaterial->setVector4(gBottomColor, &m_borderColor);
        m_frameMaterial->setVector4(gLeftColor, &m_borderColor);
    }
}

Vector4 AbstractButton::backgroundColor() const {
    return m_backgroundColor;
}

void AbstractButton::setBackgroundColor(const Vector4 &color) {
    m_backgroundColor = color;

    if(m_imageMaterial) {
        m_imageMaterial->setVector4(gColor, &m_backgroundColor);
    }

    if(m_frameMaterial) {
        m_frameMaterial->setVector4(gBackgroundColor, &m_backgroundColor);
    }

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        StyleSheet::setStyleProperty(this, gCssBackgroundColor, StyleSheet::toColor(m_backgroundColor));
    }
#endif
}
/*!
    Returns the color used when the button is highlighted.
*/
Vector4 AbstractButton::highlightedColor() const {
    return m_highlightedColor;
}
/*!
    Sets the \a color used when the button is highlighted.
*/
void AbstractButton::setHighlightedColor(const Vector4 &color) {
    m_highlightedColor = color;
}
/*!
    Returns the color used when the button is pressed.
*/
Vector4 AbstractButton::pressedColor() const {
    return m_pressedColor;
}
/*!
    Sets the \a color used when the button is pressed.
*/
void AbstractButton::setPressedColor(const Vector4 &color) {
    m_pressedColor = color;
}
/*!
    Returns true if the button is checkable; otherwise, false.
*/
bool AbstractButton::isCheckable() const {
    return m_checkable;
}
/*!
    Sets whether the button is \a checkable.
*/
void AbstractButton::setCheckable(bool checkable) {
    m_checkable = checkable;
}
/*!
    Returns true if the button is checked; otherwise, false.
*/
bool AbstractButton::isChecked() const {
    return m_checked;
}
/*!
    Sets the \a checked state of the button.
*/
void AbstractButton::setChecked(bool checked) {
    m_checked = checked;

    updateBackgroundColor(checked ? m_pressedColor : m_backgroundColor);

    checkStateSet();

    toggled(m_checked);
}
/*!
    Returns true if the button is in exclusive mode; otherwise, false.
*/
bool AbstractButton::isExclusive() const {
    return m_exclusive;
}
/*!
    Sets whether the button is in \a exclusive mode.
*/
void AbstractButton::setExclusive(bool exclusive) {
    m_exclusive = exclusive;
}

void AbstractButton::pressed() {
    emitSignal(_SIGNAL(pressed()));
}

void AbstractButton::clicked() {
    emitSignal(_SIGNAL(clicked()));
}

void AbstractButton::toggled(bool checked) {
    emitSignal(_SIGNAL(toggled(bool)), checked);
}
/*!
    \internal
    Internal method called to update the button's visual appearance and handle user interaction.
*/
void AbstractButton::update(const Vector2 &pos) {
    Widget::update(pos);

    setHovered(isHovered(pos));

    Vector4 color(m_checked ? m_pressedColor : m_backgroundColor);
    if(m_hovered) {
        color = m_highlightedColor;
        if(Input::isMouseButtonDown(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_BEGAN)) {
            m_currentFade = 0.0f;

            if(m_checkable) {
                setChecked(!m_checked);
            }
            pressed();
        }

        if(Input::isMouseButtonUp(Input::MOUSE_LEFT) || (Input::touchCount() > 0 && Input::touchState(0) == Input::TOUCH_ENDED)) {
            m_currentFade = 0.0f;

            clicked();
        }

        if(Input::isMouseButton(Input::MOUSE_LEFT) || Input::touchCount() > 0) {
            color = m_pressedColor;
        }
    }

    if(m_currentFade < 1.0f) {
        m_currentFade += 1.0f / m_fadeDuration * Timer::deltaTime();
        m_currentFade = CLAMP(m_currentFade, 0.0f, 1.0f);

        updateBackgroundColor(MIX(m_currentColor, color, m_currentFade));
    }
}
/*!
    \internal
    Applies style settings assigned to widget.
*/
void AbstractButton::applyStyle() {
    Widget::applyStyle();

    // Background color
    auto it = m_styleRules.find(gCssBackgroundColor);
    if(it != m_styleRules.end()) {
        setBackgroundColor(StyleSheet::toColor(it->second.second));
    }
}
/*!
    \internal
    Internal method to handle the button's checked state, ensuring exclusivity in exclusive mode.
*/
void AbstractButton::checkStateSet() {
    if(m_exclusive && m_checked) {
        for(auto it : actor()->getChildren()) {
            Actor *actor = dynamic_cast<Actor *>(it);
            if(actor) {
                AbstractButton *btn = actor->getComponent<AbstractButton>();
                if(btn && btn != this) {
                    btn->setChecked(false);
                }
            }
        }
    }
}
/*!
    \internal
    Internal method called to compose the button component by adding background, label, and icon components.
*/
void AbstractButton::composeComponent() {
    Widget::composeComponent();

    RectTransform *rect = rectTransform();
    rect->blockSignals(true);
    rect->setSize(Vector2(100.0f, 30.0f));
    rect->blockSignals(false);
}
/*!
    \internal
*/
void AbstractButton::boundChanged(const Vector2 &) {
    m_dirtyBackground = true;

    if(m_frameMaterial) {
        RectTransform *rect = rectTransform();
        if(rect) {
            Vector4 normCorners(m_borderRadius / rect->size().y);
            m_frameMaterial->setVector4(gBorderRadius, &normCorners);
        }
    }
}

void AbstractButton::updateBackgroundColor(const Vector4 &color) {
    m_currentColor = color;
    if(m_backgroundImage) {
        m_imageMaterial->setVector4(gColor, &color);
    } else {
        m_frameMaterial->setVector4(gBackgroundColor, &color);
    }
}
