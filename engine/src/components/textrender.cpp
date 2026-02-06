#include "components/textrender.h"

#include "resources/mesh.h"
#include "resources/font.h"
#include "resources/material.h"

#include "commandbuffer.h"
#include "gizmos.h"

namespace {
    const char *gColor("mainColor");
    const char *gTexture("mainTexture");
    const char *gWeight("weight");
};

/*!
    \class TextRender
    \brief Draws a text for the 2D and 3D graphics.
    \inmodule Components

    The TextRender component allows you to display a text in both 2D and 3D scenes.
*/

TextRender::TextRender() :
        m_color(1.0f),
        m_font(nullptr),
        m_mesh(Engine::objectCreate<Mesh>()),
        m_size(16),
        m_alignment(Left),
        m_priority(0),
        m_fontWeight(0.5f),
        m_flags(Font::Kerning | Font::Sdf),
        m_dirtyMesh(true),
        m_dirtyMaterial(true),
        m_translated(false) {

    m_mesh->makeDynamic();

    Material *material = Engine::loadResource<Material>(".embedded/DefaultFont.shader");
    if(material) {
        MaterialInstance *instance = material->createInstance();
        instance->setFloat(gWeight, &m_fontWeight);

        m_materials.push_back(instance);
    }
}

TextRender::~TextRender() {
    if(m_font) {
        m_font->unsubscribe(this);
    }
}
/*!
    \internal
*/
Mesh *TextRender::meshToDraw(int instance) {
    A_UNUSED(instance);
    if(m_dirtyMesh && m_font && !m_text.isEmpty()) {
        m_font->composeMesh(m_mesh, m_translated ?  Engine::translate(m_text) : m_text, m_size, m_alignment, m_flags, m_boundaries);
        m_mesh->setColors(Vector4Vector(m_mesh->vertices().size(), Vector4(1.0f)));
        m_dirtyMesh = false;
    }

    return m_text.isEmpty() ? nullptr : m_mesh;
}
/*!
    Returns the text which will be drawn.
*/
TString TextRender::text() const {
    return m_text;
}
/*!
    Changes the \a text which will be drawn.
*/
void TextRender::setText(const TString &text) {
    if(text != m_text) {
        m_text = text;

        m_dirtyMesh = m_dirtyMaterial = true;
    }
}
/*!
    Returns the font which will be used to draw a text.
*/
Font *TextRender::font() const {
    return m_font;
}
/*!
    \fn void TextRender::setFont(Font *font)

    Sets the new \a font asset used to render a text.
*/
void TextRender::setFont(Font *font) {
    if(m_font != font)  {
        if(m_font) {
            m_font->unsubscribe(this);
        }

        m_font = font;
        if(m_font) {
            m_font->subscribe(&TextRender::fontUpdated, this);

            m_dirtyMaterial = true;
        }
        m_dirtyMesh = true;
    }
}
/*!
    Creates a new instance of \a material and assigns it.
*/
void TextRender::setMaterial(Material *material) {
    Renderable::setMaterial(material);

    m_dirtyMaterial = true;
}
/*!
    Returns the size of the font.
*/
int TextRender::fontSize() const {
    return m_size;
}
/*!
    Changes the \a size of the font.
*/
void TextRender::setFontSize(int size) {
    m_size = size;
    m_dirtyMesh = true;
}
/*!
    Returns the color of the text to be drawn.
*/
Vector4 TextRender::color() const {
    return m_color;
}
/*!
    Changes the \a color of the text to be drawn.
*/
void TextRender::setColor(const Vector4 &color) {
    m_color = color;
    m_dirtyMaterial = true;
}
/*!
    Returns true if text in text render must be translated; othewise returns false.
*/
bool TextRender::translated() const {
    return m_translated;
}
/*!
    Sets \a enable or disable translation from dictionary for current text render.
*/
void TextRender::setTranslated(bool enable) {
    if(m_translated != enable) {
        m_translated = enable;
        m_dirtyMesh = true;
    }
}
/*!
    Returns true if word wrap enabled; otherwise returns false.
*/
bool TextRender::wordWrap() const {
    return m_flags & Font::Wrap;
}
/*!
    Sets the word \a wrap policy. Set true to enable word wrap and false to disable.
*/
void TextRender::setWordWrap(bool wrap) {
    if(wordWrap() != wrap) {
        if(wrap) {
            m_flags |= Font::Wrap;
        } else {
            m_flags &= ~Font::Wrap;
        }
    }
    m_dirtyMesh = true;
}
/*!
    Returns the boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
Vector2 TextRender::size() const {
    return m_boundaries;
}
/*!
    Changes the size of \a boundaries of the text area. This parameter is involved in Word Wrap calculations.
*/
void TextRender::setSize(const Vector2 &boundaries) {
    m_boundaries = boundaries;
    m_dirtyMesh = true;
}
/*!
    Returns text alignment policy.
*/
int TextRender::align() const {
    return m_alignment;
}
/*!
    Sets text \a alignment policy.
*/
void TextRender::setAlign(int alignment) {
    m_alignment = alignment;
    m_dirtyMesh = true;
}
/*!
    Returns true if glyph kerning enabled; otherwise returns false.
*/
bool TextRender::kerning() const {
    return m_flags & Font::Kerning;
}
/*!
    Set true to \a enable glyph kerning and false to disable.
    \note Glyph kerning functionality depends on fonts which you are using. In case of font doesn't support kerning, you will not see the difference.
*/
void TextRender::setKerning(const bool enable) {
    if(kerning() != enable) {
        if(enable) {
            m_flags |= Font::Kerning;
        } else {
            m_flags &= ~Font::Kerning;
        }
        m_dirtyMesh = true;
    }
}
/*!
    Returns the redering layer.
*/
int TextRender::layer() const {
    return m_priority;
}
/*!
    Sets the redering \a layer.
*/
void TextRender::setLayer(int layer) {
    m_priority = layer;
    m_dirtyMaterial = true;
}
/*!
    \internal
*/
MaterialInstance *TextRender::materialInstance(int index) {
    if(m_dirtyMaterial) {
        for(auto it : m_materials) {
            if(it) {
                if(m_font) {
                    it->setTexture(gTexture, m_font->page());
                }
                it->setVector4(gColor, &m_color);
                it->setFloat(gWeight, &m_fontWeight);
                it->setTransform(transform());
                it->setPriority(m_priority);
            }
        }
        m_dirtyMaterial = false;
    }
    return Renderable::materialInstance(index);
}
/*!
    \internal
*/
bool TextRender::event(Event *ev) {
    if(ev->type() == Event::LanguageChange) {
        m_dirtyMesh = true;
    }

    return true;
}
/*!
    \internal
*/
void TextRender::composeComponent() {
    setFont(Engine::loadResource<Font>(".embedded/Roboto.ttf"));
    setText("Text");
    setColor(Vector4(1.0f));
}
/*!
    \internal
*/
AABBox TextRender::localBound() {
    if(m_mesh) {
        return m_mesh->bound();
    }

    return Renderable::localBound();
}
/*!
    \internal
*/
void TextRender::setMaterialsList(const std::list<Material *> &materials) {
    Renderable::setMaterialsList(materials);

    m_dirtyMaterial = true;
}
/*!
    \internal
*/
void TextRender::drawGizmosSelected() {
    AABBox box = bound();
    Gizmos::drawWireBox(box.center, box.extent * 2.0f, Vector4(1.0f));
}
/*!
    \internal
*/
void TextRender::fontUpdated(int state, void *ptr) {
    if(state == Resource::Ready) {
        static_cast<TextRender *>(ptr)->m_dirtyMesh = true;
    }
}
