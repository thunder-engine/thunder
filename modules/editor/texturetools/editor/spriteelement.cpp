#include "spriteelement.h"

#include "spritecontroller.h"

SpriteElement::SpriteElement() :
        m_controller(nullptr),
        m_settings(nullptr) {

}

void SpriteElement::setController(SpriteController *controller) {
    m_controller = controller;
}

void SpriteElement::setSettings(TextureImportSettings *settings) {
    m_settings = settings;
}

void SpriteElement::setKey(const TString &key) {
    m_key = key;
}

TString SpriteElement::name() const {
    return m_key;
}

void SpriteElement::setName(const TString &name) {
    if(name != m_key) {
        auto it = m_settings->elements().find(m_key);
        if(it != m_settings->elements().end()) {
            m_controller->undoRedo()->push(new RenameSprite(m_key, name, m_controller));

            m_key = name;
        }
    }
}

Vector2 SpriteElement::position() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return Vector2(it->second.min.x, it->second.max.y);
    }

    return Vector2();
}

void SpriteElement::setPosition(const Vector2 &position) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        if(position.x != element.min.x || position.y != element.max.y) {
            element.min.x = position.x;
            element.max.y = position.y;

            updateController(element);
        }
    }
}

Vector2 SpriteElement::size() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return Vector2(it->second.max.x - it->second.min.x,
                       it->second.max.y - it->second.min.y);
    }

    return Vector2();
}

void SpriteElement::setSize(const Vector2 &position) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        float x = int(element.min.x + position.x);
        float y = int(element.max.y - position.y);

        if(x != element.max.x || y != element.min.y) {
            element.max.x = x;
            element.min.y = y;

            updateController(element);
        }
    }
}

Vector2 SpriteElement::pivot() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return it->second.pivot;
    }

    return Vector2();
}

void SpriteElement::setPivot(const Vector2 &pivot) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        if(pivot != element.pivot) {
            element.pivot = pivot;

            updateController(element);
        }
    }
}

Vector4 SpriteElement::border() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return Vector4(it->second.borderMax.y, it->second.borderMax.x, it->second.borderMin.y, it->second.borderMin.x);
    }

    return Vector4();
}

void SpriteElement::setBorder(const Vector4 &border) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        if(border.x != element.borderMax.y ||
           border.y != element.borderMax.x ||
           border.z != element.borderMin.y ||
           border.w != element.borderMin.x) {

            element.borderMax.y = border.x;
            element.borderMax.x = border.y;
            element.borderMin.y = border.z;
            element.borderMin.x = border.w;

            updateController(element);
        }
    }
}

void SpriteElement::updateController(const TextureImportSettings::Element &element) {
    m_controller->undoRedo()->push(new UpdateSprite({element}, m_controller));
}
