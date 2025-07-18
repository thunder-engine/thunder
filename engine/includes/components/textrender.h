#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include <renderable.h>

#include <font.h>
#include <material.h>

class Mesh;
class MaterialInstance;

enum Alignment {
    Left    = (1<<0),
    Center  = (1<<1),
    Right   = (1<<2),

    Top     = (1<<4),
    Middle  = (1<<5),
    Bottom  = (1<<6)
};

class ENGINE_EXPORT TextRender : public Renderable {
    A_OBJECT(TextRender, Renderable, Components/2D)

    A_PROPERTIES(
        A_PROPERTY(TString, text, TextRender::text, TextRender::setText),
        A_PROPERTYEX(int, alignment, TextRender::align, TextRender::setAlign, "editor=Alignment"),
        A_PROPERTYEX(Font *, font, TextRender::font, TextRender::setFont, "editor=Asset"),
        A_PROPERTYEX(Material *, material, TextRender::material, TextRender::setMaterial, "editor=Asset"),
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

    TString text() const;
    void setText(const TString text);

    Font *font() const;
    void setFont(Font *font);

    void setMaterial(Material *material) override;

    int fontSize() const;
    void setFontSize(int size);

    Vector4 color() const;
    void setColor(const Vector4 color);

    bool wordWrap() const;
    void setWordWrap(bool wrap);

    Vector2 size() const;
    void setSize(const Vector2 boundaries);

    int align() const;
    void setAlign(int alignment);

    bool kerning() const;
    void setKerning(const bool kerning);

    static void composeMesh(Font *font, Mesh *mesh, int size, const TString &text, int alignment, bool kerning, bool wrap, const Vector2 &boundaries);

    static Vector2 cursorPosition(Font *font, int size, const TString &text, bool kerning, const Vector2 &boundaries);

private:
    Mesh *meshToDraw() const override;

    AABBox localBound() const override;

    void setMaterialsList(const std::list<Material *> &materials) override;

    void drawGizmosSelected() override;

    void loadData(const VariantList &data) override;

    bool event(Event *ev) override;

    void composeComponent() override;

    static void fontUpdated(int state, void *ptr);

private:
    TString m_text;

    Vector4 m_color;

    Vector2 m_boundaries;

    Font *m_font;

    Mesh *m_mesh;

    int32_t m_size;

    int m_alignment;

    float m_fontWeight;

    bool m_kerning;

    bool m_wrap;

};

#endif // TEXTRENDER_H
