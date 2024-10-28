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

void SpriteElement::setKey(const std::string &key) {
    m_key = key;
}

QString SpriteElement::name() const {
    return m_key.c_str();
}

void SpriteElement::setName(const QString &name) {
    if(name.toStdString() != m_key) {
        auto it = m_settings->elements().find(m_key);
        if(it != m_settings->elements().end()) {
            UndoManager::instance()->push(new RenameSprite(m_key, name.toStdString(), m_controller));

            m_key = name.toStdString();
        }
    }
}

Vector2 SpriteElement::position() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return Vector2(it->second.m_min.x, it->second.m_max.y);
    }

    return Vector2();
}

void SpriteElement::setPosition(const Vector2 &position) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        if(position.x != element.m_min.x || position.y != element.m_max.y) {
            element.m_min.x = position.x;
            element.m_max.y = position.y;

            updateController(element);
        }
    }
}

Vector2 SpriteElement::size() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return Vector2(it->second.m_max.x - it->second.m_min.x,
                       it->second.m_max.y - it->second.m_min.y);
    }

    return Vector2();
}

void SpriteElement::setSize(const Vector2 &position) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        float x = int(element.m_min.x + position.x);
        float y = int(element.m_max.y - position.y);

        if(x != element.m_max.x || y != element.m_min.y) {
            element.m_max.x = x;
            element.m_min.y = y;

            updateController(element);
        }
    }
}

Vector2 SpriteElement::pivot() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return it->second.m_pivot;
    }

    return Vector2();
}

void SpriteElement::setPivot(const Vector2 &pivot) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        if(pivot != element.m_pivot) {
            element.m_pivot = pivot;

            updateController(element);
        }
    }
}

Vector4 SpriteElement::border() const {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        return Vector4(it->second.m_borderMax.y, it->second.m_borderMax.x, it->second.m_borderMin.y, it->second.m_borderMin.x);
    }

    return Vector4();
}

void SpriteElement::setBorder(const Vector4 &border) {
    auto it = m_settings->elements().find(m_key);
    if(it != m_settings->elements().end()) {
        TextureImportSettings::Element element = it->second;

        if(border.x != element.m_borderMax.y ||
           border.y != element.m_borderMax.x ||
           border.z != element.m_borderMin.y ||
           border.w != element.m_borderMin.x) {

            element.m_borderMax.y = border.x;
            element.m_borderMax.x = border.y;
            element.m_borderMin.y = border.z;
            element.m_borderMin.x = border.w;

            updateController(element);
        }
    }
}

void SpriteElement::updateController(const TextureImportSettings::Element &element) {
    UndoManager::instance()->push(new UpdateSprite({element}, m_controller));
}
