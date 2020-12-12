#ifndef FONT_H
#define FONT_H

#include "sprite.h"

class FontPrivate;

class NEXT_LIBRARY_EXPORT Font : public Sprite {
    A_REGISTER(Font, Sprite, Resources)

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

    void requestCharacters(const string &characters);

    int length(const string &characters) const;

    float spaceWidth() const;

    float lineHeight() const;

    void loadUserData(const VariantMap &data) override;
private:
    void clear();

    bool requestCharacter(uint32_t character);

protected:
    VariantMap saveUserData() const override;

private:
    FontPrivate *p_ptr;
};


#endif // FONT_H
