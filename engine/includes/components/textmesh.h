#ifndef TEXTMESH_H
#define TEXTMESH_H

#include "component.h"

#include "font.h"

class Mesh;
class MaterialInstance;

class NEXT_LIBRARY_EXPORT TextMesh : public Component {
    A_REGISTER(TextMesh, Component, Components);

    A_PROPERTIES(
        A_PROPERTY(string, Text, TextMesh::text, TextMesh::setText),
        A_PROPERTY(Font, Font_Name, TextMesh::font, TextMesh::setFont),
        A_PROPERTY(int, Font_Size, TextMesh::size, TextMesh::setSize),
        A_PROPERTY(Color, Color, TextMesh::color, TextMesh::setColor)
    );
    A_NOMETHODS();

public:
    TextMesh                    ();

    void                        draw                (ICommandBuffer &buffer, int8_t layer);

    string                      text                () const;

    void                        setText             (const string &text);

    Font                       *font                () const;

    void                        setFont             (Font *font);

    int                         size                () const;

    void                        setSize             (int size);

    Vector4                     color               () const;

    void                        setColor            (const Vector4 &color);

    void                        loadUserData        (const VariantMap &data);

    VariantMap                  saveUserData        () const;

    Mesh                       *mesh                () const;

protected:
    void                        composeMesh         ();

    Mesh                       *m_pMesh;

    MaterialInstance           *m_pMaterial;

    Font                       *m_pFont;

    uint32_t                    m_Size;

    int32_t                     m_Space;

    int32_t                     m_Line;

    string                      m_Text;

    Vector4                     m_Color;

};

#endif // TEXTMESH_H
