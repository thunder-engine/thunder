#include "components/menu.h"

#include "components/recttransform.h"
#include "components/canvas.h"

#include "resources/font.h"
#include "resources/material.h"

#include <input.h>

namespace  {
    const char *gTexture("mainTexture");
    const char *gUseSDF("useSdf");

    const char *gBackgroundColor("backgroundColor");
    const char *gBorderWidth("borderWidth");
    const char *gBorderRadius("borderRadius");
    const char *gBorderColor("borderColor");

    const char *gDefaultFrame(".embedded/Frame.shader");
}

/*!
    \class Menu
    \brief The Menu class represents a graphical user interface menu with options and actions.
    \inmodule Gui

    The Menu class represents a graphical user interface (GUI) menu that contains a list of options or actions that users can select.
    It provides a way to organize and display commands or choices in a structured manner, typically within a dropdown or context menu format.
    Menus are essential in GUI design for providing users with accessible options and actions within an application.
*/

Menu::Menu() :
        m_textMesh(Engine::objectCreate<Mesh>()),
        m_dirtyText(true) {

    Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.shader");
    if(material) {
        m_textMaterial = material->createInstance();
        int sdf = 0;
        m_textMaterial->setInteger(gUseSDF, &sdf);
    }

    Material *frameMaterial = Engine::loadResource<Material>(gDefaultFrame);
    if(frameMaterial) {
        m_selectionMaterial = frameMaterial->createInstance();

        Vector4 width(0.0f);
        m_selectionMaterial->setVector4(gBorderWidth, &width);
        m_selectionMaterial->setVector4(gBorderRadius, &m_borderRadius);
        m_selectionMaterial->setVector4(gBorderColor, &m_borderColor);
        m_selectionMaterial->setVector4(gBackgroundColor, &m_selectionColor);
    }

}

Menu::~Menu() {
    delete m_textMesh;
}
/*!
    Adds a section to the menu with the specified \a text and optional \a icon.
*/
void Menu::addAction(const TString &text, Sprite *icon) {
    MenuItem item;
    item.type = MenuItem::Action;
    item.text = text;
    item.icon = icon;
    m_items.push_back(item);

    m_dirtyText = true;
    updateSize();
}

void Menu::addSeparator() {
    MenuItem item;
    item.type = MenuItem::Separator;
    m_items.push_back(item);

    updateSize();
}

void Menu::addSubmenu(const TString &text, Menu *submenu, Sprite *icon) {
    MenuItem item;
    item.type = MenuItem::Submenu;
    item.text = text;
    item.submenu = submenu;
    item.icon = icon;
    m_items.push_back(item);

    m_dirtyText = true;
    updateSize();
}
/*!
    Displays the menu at the specified \a position.
*/
void Menu::show(const Vector2 &position) {
    aboutToShow();
    setEnabled(true);
    rectTransform()->setPosition(Vector3(position, 0.0f));
    repaint();
}
/*!
    Hides the menu.
*/
void Menu::hide() {
    if(isEnabled()) {
        aboutToHide();
    }
    setEnabled(false);
    repaint();
}
/*!
    Returns the text of the item at the specified \a index.
*/
TString Menu::itemText(int index) {
    if(index > -1 && index < m_items.size()) {
        return m_items[index].text;
    }
    return TString();
}

void Menu::setItemText(int index, const TString &text) {
    if(index > -1 && index < m_items.size()) {
        m_items[index].text = text;
        m_dirtyText = true;
        repaint();
    }
}

Sprite *Menu::itemIcon(int index) {
    if(index > -1 && index < m_items.size()) {
        return m_items[index].icon;
    }
    return nullptr;
}

void Menu::setItemIcon(int index, Sprite *icon) {
    if(index > -1 && index < m_items.size()) {
        m_items[index].icon = icon;
        repaint();
    }
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *Menu::font() const {
    return m_font;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void Menu::setFont(Font *font) {
    if(m_font != font) {
        if(m_font) {
            m_font->unsubscribe(this);
        }

        m_font = font;
        if(m_font) {
            m_font->subscribe(&Menu::fontUpdated, this);
        }

        m_dirtyText = true;
        repaint();
    }
}

void Menu::aboutToShow() {
    emitSignal(_SIGNAL(aboutToShow()));
}

void Menu::aboutToHide() {
    emitSignal(_SIGNAL(aboutToHide()));
}

void Menu::triggered(int index) {
    emitSignal(_SIGNAL(triggered(int)), index);
}
/*!
    \internal
    Internal method called to draw menu.
*/
void Menu::draw() {
    Frame::draw();

    RectTransform *rect = rectTransform();
    Vector2 size(rect->size());
    Matrix4 transform(rect->worldTransform());

    if(m_dirtyText && m_font) {
        m_textMesh->clear();
        Font::Settings settings = {m_fontSize, Alignment::Left | Alignment::Middle, Font::Additive, m_textColor};
        settings.boundaries = Vector2(size.x - 40.0f, m_rowHeight);
        settings.offset.x = 20.0f;
        settings.offset.y = -settings.boundaries.y - m_rowHeight;
        for(size_t i = 0; i < m_items.size(); ++i) {
            if(m_items[i].type != MenuItem::Separator) {
                m_font->composeMesh(m_textMesh, m_items[i].text, settings);
                settings.offset.y += m_rowHeight;
            } else {
                settings.offset.y -= m_separatorHeight;
            }
        }
        m_textMaterial->setTexture(gTexture, m_font->page());
        m_dirtyText = false;
    }

    Canvas *canvas = Menu::canvas();

    uint32_t hash = rect->hash();
    m_textMaterial->setTransform(transform, 0, hash);

    if(m_hoveredIndex > -1 && m_items[m_hoveredIndex].type != MenuItem::Separator) {
        Matrix4 s;
        s[0] = size.x;
        s[5] = m_rowHeight;
        s[12] = size.x * 0.5f;
        s[13] = size.y - s[5] * 0.5f - m_hoveredIndex * m_rowHeight;
        Mathf::hashCombine(hash, s[13]);

        m_selectionMaterial->setTransform(transform * s, 0, hash);
        canvas->drawRect(m_selectionMaterial, nullptr);
    }

    canvas->drawMesh(m_textMesh, m_textMaterial);
}
/*!
    \internal
    Updates the menu. Handles input and triggers actions based on user interactions.
*/
void Menu::update(const Vector2 &pos) {
    Widget::update(pos);

    RectTransform *rect = rectTransform();
    Vector2 localPos = rect->mapFromGlobal(pos.x, pos.y);

    int newHoveredIndex = -1;
    if(localPos.x >= 0 && localPos.x <= rect->size().x &&
        localPos.y >= 0 && localPos.y <= rect->size().y) {

        newHoveredIndex = static_cast<int>((rect->size().y - localPos.y) / m_rowHeight);
    }

    if(newHoveredIndex != m_hoveredIndex && newHoveredIndex < m_items.size()) {
        m_hoveredIndex = newHoveredIndex;
        repaint();
    }

    if(Input::isMouseButtonDown(0)) {
        if(newHoveredIndex > -1) {
            auto &item = m_items[m_hoveredIndex];

            if(item.type == MenuItem::Action) {
                triggered(newHoveredIndex);
            }
        }

        if(!isHovered(pos)) {
            hide();
        }
    }
}
/*!
    \internal
*/
void Menu::updateSize() {
    float height = 0.0f;
    float width = m_minWidth;

    for(auto &item : m_items) {
        if(item.type == MenuItem::Separator) {
            height += m_separatorHeight;
        } else {
            if(m_font && !item.text.isEmpty()) {
                float textWidth = m_font->textWidth(item.text, m_fontSize, 0);
                width = std::max(width, textWidth + 40.0f);
            }
            height += m_rowHeight;
        }
    }

    rectTransform()->setSize(Vector2(width, height));
    repaint();
}
/*!
    \internal
*/
void Menu::composeComponent() {
    Frame::composeComponent();

    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));

    addAction("Test Action #1");
    addAction("Test Action #2");
    addAction("Test Action #3");
}
/*!
    \internal
*/
void Menu::fontUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        Menu *p = static_cast<Menu *>(ptr);
        p->m_dirtyText = true;
        p->repaint();
    }
}
