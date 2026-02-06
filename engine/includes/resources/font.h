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
class AtlasNode;

class ENGINE_EXPORT Font : public Resource {
    A_OBJECT(Font, Resource, Resources)

    A_NOPROPERTIES()
    A_NOMETHODS()

    struct GlyphData {
        Vector3Vector vertices;

        Vector2Vector uvs;

        IndexVector indices;

        ByteArray data;

        AtlasNode *node = nullptr;

        int width = 0;

        int height = 0;

        bool copied = false;

    };

    enum Flags {
        Kerning = (1<<0),
        Wrap = (1<<1),
        Sdf = (1<<2)
    };

public:
    Font();
    ~Font();

    Texture *page();

    float textWidth(const TString &text, int size, int flags);

    void composeMesh(Mesh *mesh, const TString &text, int size, int alignment, int flags, const Vector2 &boundaries);

private:
    void clear();

    void clearAtlas();

    void requestCharacters(const std::u32string &characters, uint32_t size);

    int requestKerning(int glyph, int previous) const;

    GlyphData *glyph(int key);

    void packSheets(int padding);

    VariantMap saveUserData() const override;
    void loadUserData(const VariantMap &data) override;

private:
    std::unordered_map<uint32_t, GlyphData> m_shapes;

    ByteArray m_data;

    int32_t *m_face;

    Texture *m_page;

    AtlasNode *m_root;

    bool m_useKerning;

};


#endif // FONT_H
