#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include "renderable.h"

#include "font.h"

class Mesh;
class MaterialInstance;

class NEXT_LIBRARY_EXPORT TextRender : public Renderable {
    A_REGISTER(TextRender, Renderable, Components)

    A_PROPERTIES(
        A_PROPERTY(string, Text, TextRender::text, TextRender::setText),
        A_PROPERTY(Font, Font_Name, TextRender::font, TextRender::setFont),
        A_PROPERTY(int, Font_Size, TextRender::fontSize, TextRender::setFontSize),
        A_PROPERTY(Color, Color, TextRender::color, TextRender::setColor),
        A_PROPERTY(bool, Use_Kerning, TextRender::kerning, TextRender::setKerning)
    )
    A_NOMETHODS()

public:
    TextRender          ();

    void                draw                (ICommandBuffer &buffer, int8_t layer);

    Mesh               *mesh                () const;

    string              text                () const;
    void                setText             (const string &text);

    Font               *font                () const;
    void                setFont             (Font *font);

    int                 fontSize            () const;
    void                setFontSize         (int size);

    Vector4             color               () const;
    void                setColor            (const Vector4 &color);

    bool                kerning             () const;
    void                setKerning          (const bool kerning);

    void                loadUserData        (const VariantMap &data);
    VariantMap          saveUserData        () const;

protected:
    void                composeMesh         ();

    Font               *m_pFont;

    uint32_t            m_Size;

    int32_t             m_Space;

    int32_t             m_Line;

    string              m_Text;

    Vector4             m_Color;

    bool                m_Kerning;

    MaterialInstance   *m_pMaterial;

    Mesh               *m_pMesh;

};

#endif // TEXTRENDER_H
