#ifndef FONT_H
#define FONT_H

#include "resource.h"

enum Alignment {
    Left    = (1<<0),
    Center  = (1<<1),
    Right   = (1<<2),

    Top     = (1<<4),
    Middle  = (1<<5),
    Bottom  = (1<<6)
};

class Mesh;
class Texture;

class ENGINE_EXPORT Font : public Resource {
    A_OBJECT(Font, Resource, Resources)

    A_NOPROPERTIES()
    A_METHODS(
        A_METHOD(int, Font::atlasIndex),
        A_METHOD(int, Font::requestKerning),
        A_METHOD(void, Font::requestCharacters),
        A_METHOD(int, Font::length),
        A_METHOD(float, Font::spaceWidth),
        A_METHOD(float, Font::lineHeight)
    )

public:
    Font();
    ~Font();

    Texture *page(int index = 0);

    int atlasIndex(int glyph) const;

    int requestKerning(int glyph, int previous) const;

    void requestCharacters(const TString &characters);

    int length(const TString &characters) const;

    float spaceWidth() const;

    float lineHeight() const;

    float textWidth(const TString &text, int size, bool kerning);

    void composeMesh(Mesh *mesh, const TString &text, int size, int alignment, bool kerning, bool wrap, const Vector2 &boundaries);

    void loadUserData(const VariantMap &data) override;

private:
    void clear();

    Mesh *shape(int key) const;

    int addElement(Texture *texture);

    void packSheets(int padding);

    void addPage(Texture *texture);

protected:
    VariantMap saveUserData() const override;

private:
    std::unordered_map<uint32_t, uint32_t> m_glyphMap;
    std::unordered_map<uint32_t, Mesh *> m_shapes;

    std::vector<Texture *> m_pages;
    std::vector<Texture *> m_sources;

    ByteArray m_data;

    int32_t *m_face;

    int32_t m_scale;

    float m_spaceWidth;
    float m_lineHeight;

    bool m_useKerning;

};


#endif // FONT_H
