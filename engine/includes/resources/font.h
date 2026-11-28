/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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

    struct GlyphAtlas {
        std::unordered_map<uint32_t, GlyphData> shapes;

        AtlasNode *root = nullptr;

        Texture *page = nullptr;

        uint32_t width = 512;

        uint32_t height = 512;

        int padding = 2;

        bool sdf = false;

        bool dirty = false;
    };

    struct Settings {
        int size = 0;

        int alignment = Alignment::Left | Alignment::Middle;

        int flags = 0;

        Vector4 color;

        Vector2 boundaries;

        Vector2 offset;
    };

    enum Flags {
        Kerning = (1<<0),
        Wrap = (1<<1),
        Sdf = (1<<2),
        Additive = (1<<3)
    };

public:
    Font();
    ~Font();

    Texture *page();

    Texture *sdfPage();

    float textWidth(const TString &text, int size, int flags);

    void composeMesh(Mesh *mesh, const TString &text, const Settings &settings);

private:
    void clear();

    void clearAtlas(GlyphAtlas &atlas);

    void requestCharacters(const std::u32string &characters, uint32_t size, int flags);

    int requestKerning(int glyph, int previous, bool sdf) const;

    GlyphData *glyph(int key, int flags);

    GlyphAtlas &atlasFor(int flags);

    const GlyphAtlas &atlasFor(int flags) const;

    void packSheets(GlyphAtlas &atlas);

    void ensurePacked(int flags);

    void loadUserData(const VariantMap &data) override;
    VariantMap saveUserData() const override;

private:
    GlyphAtlas m_bitmapAtlas;

    GlyphAtlas m_sdfAtlas;

    ByteArray m_data;

    int32_t *m_face;

    bool m_hasKerning;

};


#endif // FONT_H
