#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include "renderable.h"

class Material;
class Font;
class Mesh;

class TextRenderPrivate;

enum Alignment {
    Left    = (1<<0),
    Center  = (1<<1),
    Right   = (1<<2),

    Top     = (1<<4),
    Middle  = (1<<5),
    Bottom  = (1<<6)
};

class NEXT_LIBRARY_EXPORT TextRender : public Renderable {
    A_REGISTER(TextRender, Renderable, Components/2D)

    A_PROPERTIES(
        A_PROPERTY(string, text, TextRender::text, TextRender::setText),
        A_PROPERTYEX(int, alignment, TextRender::align, TextRender::setAlign, "editor=Alignment"),
        A_PROPERTYEX(Font *, font, TextRender::font, TextRender::setFont, "editor=Template"),
        A_PROPERTYEX(Material *, material, TextRender::material, TextRender::setMaterial, "editor=Template"),
        A_PROPERTY(int, fontSize, TextRender::fontSize, TextRender::setFontSize),
        A_PROPERTYEX(Vector4, color, TextRender::color, TextRender::setColor, "editor=Color"),
        A_PROPERTY(bool, wordWrap, TextRender::wordWrap, TextRender::setWordWrap),
        A_PROPERTY(Vector2, size, TextRender::size, TextRender::setSize),
        A_PROPERTY(bool, kerning, TextRender::kerning, TextRender::setKerning)
    )
    A_NOMETHODS()

public:
    TextRender();
    ~TextRender();

    string text() const;
    void setText(const string &text);

    Font *font() const;
    void setFont(Font *font);

    Material *material() const;
    void setMaterial(Material *material);

    int fontSize() const;
    void setFontSize(int size);

    Vector4 &color() const;
    void setColor(const Vector4 &color);

    bool wordWrap() const;
    void setWordWrap(bool wrap);

    Vector2 &size() const;
    void setSize(const Vector2 &size);

    int align() const;
    void setAlign(int alignment);

    bool kerning() const;
    void setKerning(const bool kerning);

    static void composeMesh(Font *font, Mesh *mesh, int size, const string &text, int alignment, bool kerning, bool wrap, const Vector2 &boundaries);

private:
    void draw(ICommandBuffer &buffer, uint32_t layer) override;

    AABBox bound() const override;

#ifdef NEXT_SHARED
    bool drawHandles(ObjectList &selected) override;
#endif

    void loadData(const VariantList &data) override;
    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

    bool event(Event *ev) override;
private:
   TextRenderPrivate *p_ptr;

};

#endif // TEXTRENDER_H
