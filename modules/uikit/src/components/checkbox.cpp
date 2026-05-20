#include "components/checkbox.h"

#include "components/canvas.h"
#include "components/recttransform.h"

#include <resources/material.h>

#include "stylesheet.h"

namespace {
    const char *gOverride("mainTexture");
    const char *gColor("mainColor");

    const char *gCssColor("color");
    const char *gCssIndicatorSize("indicator-size");

    const char *gDefaultSprite(".embedded/DefaultUI.shader");
}

/*!
    \class CheckBox
    \brief A CheckBox is an option button that can be switched on or off.
    \inmodule Gui

    The CheckBox class represents an option button that can be toggled between two states: "on" or "off."
    It is commonly used in graphical user interfaces (GUIs) to allow users to select or deselect specific options or features, often in forms or settings.
*/

CheckBox::CheckBox() :
        AbstractButton(),
        m_knobColor(1.0f),
        m_knobSize(Vector2(16.0f, 8.0f)),
        m_knobIcon(nullptr),
        m_iconMesh(Engine::objectCreate<Mesh>()),
        m_iconMaterial(nullptr),
        m_foldMode(false),
        m_dirtyIcon(true) {

    setCheckable(true);

    Material *spriteMaterial = Engine::loadResource<Material>(gDefaultSprite);
    if(spriteMaterial) {
        m_iconMaterial = spriteMaterial->createInstance();
        m_iconMaterial->setVector4(gColor, &m_knobColor);
    }
}
/*!
    Returns indicator icon.
*/
Sprite *CheckBox::indicator() const {
    return m_knobIcon;
}
/*!
    Sets indicator \a icon.
*/
void CheckBox::setIndicator(Sprite *icon) {
    m_knobIcon = icon;
    if(m_knobIcon != icon) {
        m_knobIcon = icon;
        m_dirtyIcon = true;
    }
}
/*!
    Returns the color of the graphical knob.
*/
Vector4 CheckBox::indicatorColor() const {
    return m_knobColor;
}
/*!
    Sets the \a color of the graphical knob.
*/
void CheckBox::setIndicatorColor(const Vector4 color) {
    m_knobColor = color;

    if(m_iconMaterial) {
        m_iconMaterial->setVector4(gColor, &m_knobColor);
    }
#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        StyleSheet::setStyleProperty(this, gCssColor, StyleSheet::toColor(m_knobColor));
    }
#endif
}
/*!
    Returns the size of indicator.
*/
Vector2 CheckBox::indicatorSize() const {
    return m_knobSize;
}
/*!
    Sets the \a size of indicator.
*/
void CheckBox::setIndicatorSize(const Vector2 &size) {
    m_knobSize = size;
    m_dirtyIcon = true;

#ifdef SHARED_DEFINE
    if(!isSubWidget() && !isSignalsBlocked()) {
        updateStyleProperty(gCssIndicatorSize, m_knobSize.v, 2);
    }
#endif
}
/*!
    \internal
    Changes knob behaviour to \a fold icon.
*/
void CheckBox::setFoldMode(bool fold) {
    m_foldMode = fold;
}
/*!
    \internal
    Overrides the composeComponent method to create the switch component.
*/
void CheckBox::composeComponent() {
    AbstractButton::composeComponent();

    RectTransform *rect = rectTransform();
    rect->blockSignals(true);
    rect->setSize(Vector2(16.0f));
    rect->blockSignals(false);

    setIndicator(Engine::loadResource<Sprite>(".embedded/ui.png/Check"));
}
/*!
    \internal
*/
void CheckBox::draw() {
    AbstractButton::draw();

    if(m_knobIcon && m_checked) {
        if(m_dirtyIcon) {
            m_iconMaterial->setTexture(gOverride, m_knobIcon->texture());
            m_knobIcon->composeMesh(m_iconMesh, Sprite::Sliced, m_knobSize);

            m_dirtyIcon = false;
        }

        RectTransform *rect = rectTransform();
        Vector2 size(rect->size());

        Matrix4 mat(rect->worldTransform());
        Vector2 scl(rect->worldScale());
        mat[12] += size.x * 0.5f * scl.x;
        mat[13] += size.y * 0.5f * scl.y;

        uint32_t hash = rect->hash();
        Mathf::hashCombine(hash, mat[12]);
        Mathf::hashCombine(hash, mat[13]);

        if(m_foldMode) {
            Matrix3 rot;
            rot.rotate(Vector3(0.0f, 0.0f, 1.0f), 90.0f);
            mat *= rot;
            Mathf::hashCombine(hash, 90.0f);
        }

        m_iconMaterial->setTransform(mat, 0, hash);

        Canvas *canvas = CheckBox::canvas();
        canvas->drawMesh(m_iconMesh, m_iconMaterial);
    }
}
/*!
    \internal
    Applies style settings assigned to widget.
*/
void CheckBox::applyStyle() {
    AbstractButton::applyStyle();

    bool pixels;
    setIndicatorSize(styleBlock2Length(gCssIndicatorSize, m_knobSize, pixels));

    blockSignals(true);
    auto it = m_styleRules.find(gCssColor);
    if(it != m_styleRules.end()) {
        setIndicatorColor(StyleSheet::toColor(it->second.second));
    }
    blockSignals(false);
}
