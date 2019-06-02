#ifndef FONT_H
#define FONT_H

#include "atlas.h"

class FontPrivate;

class NEXT_LIBRARY_EXPORT Font : public Atlas {
    A_REGISTER(Font, Atlas, Resources)

public:
    Font                        ();

    virtual ~Font               ();

    virtual void                clear                       ();

    uint32_t                    atlasIndex                  (uint32_t glyph) const;

    void                        requestCharacters           (const u32string &characters);

    int32_t                     requestKerning              (uint32_t glyph, uint32_t previous) const;

    uint32_t                    length                      (const u32string &characters) const;

    float                       spaceWidth                  () const;

    float                       lineHeight                  () const;

protected:
    void                        loadUserData                (const VariantMap &data);

private:
    FontPrivate                *p_ptr;
};


#endif // FONT_H
