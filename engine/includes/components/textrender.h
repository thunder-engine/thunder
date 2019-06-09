#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include "renderable.h"

class Mesh;
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
        A_PROPERTY(string, Text, TextRender::text, TextRender::setText),
        A_PROPERTY(Alignment, Alignment, TextRender::align, TextRender::setAlign),
        A_PROPERTY(Font, Font_Name, TextRender::font, TextRender::setFont),
        A_PROPERTY(Material *, Material, TextRender::material, TextRender::setMaterial),
        A_PROPERTY(int, Font_Size, TextRender::fontSize, TextRender::setFontSize),
        A_PROPERTY(Color, Color, TextRender::color, TextRender::setColor),
        A_PROPERTY(bool, Word_Wrap, TextRender::wrap, TextRender::setWrap),
        A_PROPERTY(Vector2, Boundaries, TextRender::boundaries, TextRender::setBoundaries),
        A_PROPERTY(bool, Use_Kerning, TextRender::kerning, TextRender::setKerning)
    )
    A_NOMETHODS()

public:


public:
    TextRender          ();

    void                draw                (ICommandBuffer &buffer, uint32_t layer);

    string              text                () const;
    void                setText             (const string &text);

    Font               *font                () const;
    void                setFont             (Font *font);

    Material           *material            () const;
    void                setMaterial         (Material *material);

    int                 fontSize            () const;
    void                setFontSize         (int size);

    Vector4             color               () const;
    void                setColor            (const Vector4 &color);

    bool                wrap                () const;
    void                setWrap             (bool wrap);

    Vector2             boundaries          () const;
    void                setBoundaries       (const Vector2 &value);

    Alignment           align               () const;
    void                setAlign            (Alignment align);

    bool                kerning             () const;
    void                setKerning          (const bool kerning);

private:
    void                loadUserData        (const VariantMap &data);
    VariantMap          saveUserData        () const;

    TextRenderPrivate  *p_ptr;
};

#endif // TEXTRENDER_H
