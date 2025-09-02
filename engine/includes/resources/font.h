#ifndef FONT_H
#define FONT_H

#include "sprite.h"

enum Alignment {
    Left    = (1<<0),
    Center  = (1<<1),
    Right   = (1<<2),

    Top     = (1<<4),
    Middle  = (1<<5),
    Bottom  = (1<<6)
};

class ENGINE_EXPORT Font : public Sprite {
    A_OBJECT(Font, Sprite, Resources)

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
    void clear() override;

    bool requestCharacter(uint32_t character);

protected:
    VariantMap saveUserData() const override;

private:
    typedef std::unordered_map<uint32_t, uint32_t> GlyphMap;
    typedef std::unordered_map<uint32_t, Vector2> SpecialMap;

    GlyphMap m_glyphMap;
    ByteArray m_data;

    int32_t *m_face;

    int32_t m_scale;

    float m_spaceWidth;
    float m_lineHeight;

    bool m_useKerning;

};


#endif // FONT_H
