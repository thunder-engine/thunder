#ifndef FONT_H
#define FONT_H

#include "engine.h"
#include "atlas.h"

class FT_FaceRec_;

class NEXT_LIBRARY_EXPORT Font : public Atlas {
    A_REGISTER(Font, Atlas, Resources)

public:
    Font                        ();

    virtual ~Font               ();

    virtual void                clear                       ();

    int32_t                     scale                       () const;

    uint32_t                    atlasIndex                  (uint32_t glyph) const;

    void                        requestCharacters           (const u32string &characters);

    int32_t                     requestKerning              (uint32_t glyph, uint32_t previous) const;

    void                        setFontName                 (const string &name);

    uint32_t                    length                      (const u32string &characters) const;

    int32_t                     spaceWidth                  () const;

    int32_t                     lineHeight                  () const;

protected:
    void                        loadUserData                (const VariantMap &data);

protected:
    int32_t                     m_Scale;

    string                      m_FontName;

    FT_FaceRec_                *m_pFace;

    typedef unordered_map<uint32_t, uint32_t>   GlyphMap;

    GlyphMap                    m_GlyphMap;

    typedef unordered_map<uint32_t, Vector2>    SpecialMap;

    ByteArray                   m_Data;

    bool                        m_UseKerning;

};


#endif // FONT_H
