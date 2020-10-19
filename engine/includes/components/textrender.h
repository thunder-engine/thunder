#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include "renderable.h"

class Material;
class Font;

class TextRenderPrivate;

enum Alignment {
    Left,
    Center,
    Right
};

class NEXT_LIBRARY_EXPORT TextRender : public Renderable {
    A_REGISTER(TextRender, Renderable, Components)

    A_PROPERTIES(
        A_PROPERTY(string, text, TextRender::text, TextRender::setText),
        A_PROPERTYEX(int, alignment, TextRender::align, TextRender::setAlign, "editor=Alignment"),
        A_PROPERTYEX(Font *, font, TextRender::font, TextRender::setFont, "editor=Template"),
        A_PROPERTYEX(Material *, material, TextRender::material, TextRender::setMaterial, "editor=Template"),
        A_PROPERTY(int, fontSize, TextRender::fontSize, TextRender::setFontSize),
        A_PROPERTYEX(Vector4, color, TextRender::color, TextRender::setColor, "editor=Color"),
        A_PROPERTY(bool, wordWrap, TextRender::wordWrap, TextRender::setWordWrap),
        A_PROPERTY(Vector2, boundaries, TextRender::boundaries, TextRender::setBoundaries),
        A_PROPERTY(bool, kerning, TextRender::kerning, TextRender::setKerning)
    )
    A_NOMETHODS()

public:
    TextRender();
    ~TextRender();

    string text () const;
    void setText (const string &text);

    Font *font () const;
    void setFont (Font *font);

    Material *material () const;
    void setMaterial (Material *material);

    int fontSize () const;
    void setFontSize (int size);

    Vector4 color () const;
    void setColor (const Vector4 &color);

    bool wordWrap () const;
    void setWordWrap (bool wrap);

    Vector2 boundaries () const;
    void setBoundaries (const Vector2 &boundaries);

    int align () const;
    void setAlign (int alignment);

    bool kerning () const;
    void setKerning (const bool kerning);

private:
    void draw (ICommandBuffer &buffer, uint32_t layer) override;

    AABBox bound () const override;

#ifdef NEXT_SHARED
    bool drawHandles (ObjectList &selected) override;
#endif

    void loadData (const VariantList &data) override;
    void loadUserData (const VariantMap &data) override;
    VariantMap saveUserData () const override;

    bool event (Event *ev) override;
private:
   TextRenderPrivate *p_ptr;

};

#endif // TEXTRENDER_H
