#include "components/label.h"

#include "components/recttransform.h"

#include <components/actor.h>
#include <components/textrender.h>
#include <components/transform.h>

#include <resources/resource.h>
#include <resources/material.h>
#include <resources/mesh.h>
#include <resources/font.h>

#include <commandbuffer.h>

#define FONT     "Font"
#define MATERIAL "Material"

#define OVERRIDE "uni.texture0"
#define COLOR "uni.color0"

class LabelPrivate : public Resource::IObserver {
public:
    LabelPrivate(Label *label) :
        m_Color(1.0f),
        m_pLabel(label),
        m_pFont(nullptr),
        m_pMaterial(nullptr),
        m_pMesh(Engine::objectCreate<Mesh>()),
        m_Size(16),
        m_Alignment(Left),
        m_Kerning(true),
        m_Wrap(false) {

        m_pMesh->makeDynamic();
        m_pMesh->setFlags(Mesh::Uv0);

        Material *material = Engine::loadResource<Material>("DefaultFont.mtl");
        if(material) {
            m_pMaterial = material->createInstance();
            m_pMaterial->setVector4(COLOR, &m_Color);
        }
    }

    ~LabelPrivate() {
        if(m_pFont) {
            m_pFont->unsubscribe(this);
        }
    }

    void resourceUpdated(const Resource *resource, Resource::ResourceState state) override {
        if(resource == m_pFont && state == Resource::Ready) {
            composeMesh();
        }
    }

    void composeMesh() {
        RectTransform *t = dynamic_cast<RectTransform *>(m_pLabel->actor()->transform());
        if(t) {
            TextRender::composeMesh(m_pFont, m_pMesh, m_Size, m_Text, m_Alignment, m_Kerning, m_Wrap, t->size());
        }
    }

    string m_Text;

    Vector4 m_Color;

    Label *m_pLabel;

    Font *m_pFont;

    MaterialInstance *m_pMaterial;

    Mesh *m_pMesh;

    int32_t m_Size;

    int m_Alignment;

    bool m_Kerning;

    bool m_Wrap;
};

/*!
    \class Label
    \brief Draws a text for the UI.
    \inmodule Gui

    The Label component allows you to display a text in UI.
*/

Label::Label() :
        p_ptr(new LabelPrivate(this)) {

}

Label::~Label() {
    delete p_ptr;
}
/*!
    \internal
*/
void Label::draw(CommandBuffer &buffer, uint32_t layer) {
    Actor *a = actor();
    if(p_ptr->m_pMesh && !p_ptr->m_Text.empty()) {
        if(layer & CommandBuffer::RAYCAST) {
            buffer.setColor(CommandBuffer::idToColor(a->uuid()));
        }
        buffer.drawMesh(a->transform()->worldTransform(), p_ptr->m_pMesh, 0, layer, p_ptr->m_pMaterial);
        buffer.setColor(Vector4(1.0f));
    }
}
/*!
    Returns the text which will be drawn.
*/
string Label::text() const {
    return p_ptr->m_Text;
}
/*!
    Changes the \a text which will be drawn.
*/
void Label::setText(const string &text) {
    p_ptr->m_Text = text;
    p_ptr->composeMesh();
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *Label::font() const {
    return p_ptr->m_pFont;
}
/*!
    Changes the \a font which will be used to draw a text.
*/
void Label::setFont(Font *font) {
    if(p_ptr->m_pFont) {
        p_ptr->m_pFont->unsubscribe(p_ptr);
    }
    p_ptr->m_pFont = font;
    if(p_ptr->m_pFont) {
        p_ptr->m_pFont->subscribe(p_ptr);
        if(p_ptr->m_pMaterial) {
            p_ptr->m_pMaterial->setTexture(OVERRIDE, p_ptr->m_pFont->texture());
        }
    }
    p_ptr->composeMesh();
}
/*!
    Returns the size of the font.
*/
int Label::fontSize() const {
    return p_ptr->m_Size;
}
/*!
    Changes the \a size of the font.
*/
void Label::setFontSize(int size) {
    p_ptr->m_Size = size;
    p_ptr->composeMesh();
}
/*!
    Returns the color of the text to be drawn.
*/
Vector4 Label::color() const {
    return p_ptr->m_Color;
}
/*!
    Changes the \a color of the text to be drawn.
*/
void Label::setColor(const Vector4 &color) {
    p_ptr->m_Color = color;
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool Label::wordWrap() const {
    return p_ptr->m_Wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void Label::setWordWrap(bool wrap) {
    p_ptr->m_Wrap = wrap;
    p_ptr->composeMesh();
}
/*!
    Returns text alignment policy.
*/
int Label::align() const {
    return p_ptr->m_Alignment;
}
/*!
    Sets text \a alignment policy.
*/
void Label::setAlign(int alignment) {
    p_ptr->m_Alignment = alignment;
    p_ptr->composeMesh();
}
/*!
    Returns true if glyph kerning enabled; otherwise returns false.
*/
bool Label::kerning() const {
    return p_ptr->m_Kerning;
}
/*!
    Set true to enable glyph \a kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void Label::setKerning(const bool kerning) {
    p_ptr->m_Kerning = kerning;
    p_ptr->composeMesh();
}
/*!
    \internal
*/
void Label::loadData(const VariantList &data) {
    Component::loadData(data);
    p_ptr->composeMesh();
}
/*!
    \internal
*/
void Label::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(FONT);
        if(it != data.end()) {
            setFont(Engine::loadResource<Font>((*it).second.toString()));
        }
    }
}
/*!
    \internal
*/
VariantMap Label::saveUserData() const {
    VariantMap result = Widget::saveUserData();
    {
        Font *o = font();
        string ref = Engine::reference(o);
        if(!ref.empty()) {
            result[FONT] = ref;
        }
    }
    return result;
}
/*!
    \internal
*/
bool Label::event(Event *ev) {
    if(ev->type() == Event::LanguageChange) {
        p_ptr->composeMesh();
    }
    return true;
}
/*!
    \internal
*/
void Label::boundChanged() {
    Widget::boundChanged();
    p_ptr->composeMesh();
}
