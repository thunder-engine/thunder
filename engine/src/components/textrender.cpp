#include "components/textrender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/mesh.h"
#include "resources/font.h"
#include "resources/material.h"

#include "commandbuffer.h"
#include "utils.h"

#define FONT        "Font"
#define MATERIAL    "Material"

#define OVERRIDE "uni.texture0"

class TextRenderPrivate {
public:
    TextRenderPrivate   () :
        m_pFont(nullptr),
        m_Size(16),
        m_Space(0),
        m_Line(0),
        m_Color(1.0f),
        m_Alignment(Left),
        m_Kerning(true),
        m_pMaterial(nullptr),
        m_Wrap(false) {

        m_pMesh = Engine::objectCreate<Mesh>();
        m_pMesh->makeDynamic();
        m_pMesh->setFlags(Mesh::ATTRIBUTE_UV0);

        Material *material  = Engine::loadResource<Material>(".embedded/DefaultFont.mtl");
        if(material) {
            m_pMaterial = material->createInstance();
        }

    }

    void composeMesh() {
        if(m_pFont) {
            m_Space = m_pFont->spaceWidth() * m_Size;
            m_Line  = m_pFont->lineHeight() * m_Size;

            u32string text  = Utils::utf8ToUtf32(m_Text);
            m_pFont->requestCharacters(text);

            uint32_t length = m_pFont->length(text);
            if(length) {
                Mesh::Lod lod;
                lod.vertices.resize(length * 4);
                lod.indices.resize(length * 6);
                lod.uv0.resize(length * 4);

                list<float> width;
                list<uint32_t> position;

                Vector3 pos(0.0, -m_Size, 0.0f);
                uint32_t previous = 0;
                uint32_t it = 0;
                uint32_t space = 0;

                Vector3 bb[2];
                for(uint32_t i = 0; i < length; i++) {
                    uint32_t ch = text[i];
                    switch(ch) {
                        case ' ': {
                            pos = Vector3(pos.x + m_Space, pos.y, 0.0f);
                            space = it;
                        } break;
                        case '\t': {
                            pos = Vector3(pos.x + m_Space * 4, pos.y, 0.0f);
                            space = it;
                        } break;
                        case '\r': break;
                        case '\n': {
                            width.push_back(pos.x);
                            bb[1].x = MAX(bb[1].x, pos.x);
                            position.push_back(it);
                            pos = Vector3(0.0f, pos.y - m_Line, 0.0f);
                            bb[1].y = MAX(bb[1].y, pos.y);

                            space = 0;
                        } break;
                        default: {
                            if(m_Kerning) {
                                pos.x += m_pFont->requestKerning(ch, previous);
                            }
                            uint32_t index = m_pFont->atlasIndex(ch);
                            Vector2Vector shape = m_pFont->shape(index);

                            bb[0].x = MIN(bb[0].x, shape[0].x * m_Size);
                            bb[0].y = MIN(bb[0].y, shape[0].y * m_Size);

                            float x = pos.x + shape[0].x * m_Size;
                            if(m_Wrap && m_Boundaries.x < x) {
                                float shift = lod.vertices[space * 4 + 0].x;
                                for(uint32_t s = space; s < it; s++) {
                                    lod.vertices[s * 4 + 0] -= Vector3(shift, m_Line, 0.0f);
                                    lod.vertices[s * 4 + 1] -= Vector3(shift, m_Line, 0.0f);
                                    lod.vertices[s * 4 + 2] -= Vector3(shift, m_Line, 0.0f);
                                    lod.vertices[s * 4 + 3] -= Vector3(shift, m_Line, 0.0f);
                                }
                                width.push_back(shift - m_Space);
                                bb[1].x = MAX(bb[1].x, shift - m_Space);
                                position.push_back(space);
                                pos = Vector3(lod.vertices[(it - 1) * 4 + 2].x, pos.y - m_Line, 0.0f);
                                bb[1].y = MAX(bb[1].y, pos.y);
                            }

                            lod.vertices[it * 4 + 0] = pos + Vector3(shape[0], 0.0f) * m_Size;
                            lod.vertices[it * 4 + 1] = pos + Vector3(shape[1], 0.0f) * m_Size;
                            lod.vertices[it * 4 + 2] = pos + Vector3(shape[2], 0.0f) * m_Size;
                            lod.vertices[it * 4 + 3] = pos + Vector3(shape[3], 0.0f) * m_Size;

                            Vector4 uv  = m_pFont->uv(index);
                            lod.uv0[it * 4 + 0] = Vector2(uv.x, uv.y);
                            lod.uv0[it * 4 + 1] = Vector2(uv.z, uv.y);
                            lod.uv0[it * 4 + 2] = Vector2(uv.z, uv.w);
                            lod.uv0[it * 4 + 3] = Vector2(uv.x, uv.w);

                            lod.indices[it * 6 + 0] = it * 4 + 0;
                            lod.indices[it * 6 + 1] = it * 4 + 1;
                            lod.indices[it * 6 + 2] = it * 4 + 2;

                            lod.indices[it * 6 + 3] = it * 4 + 0;
                            lod.indices[it * 6 + 4] = it * 4 + 2;
                            lod.indices[it * 6 + 5] = it * 4 + 3;

                            pos += Vector3(shape[2].x * m_Size, 0.0f, 0.0f);
                            it++;
                        } break;
                    }
                    previous = ch;
                }
                if(m_Wrap) {
                    bb[1].x = m_Boundaries.x;
                    bb[1].y = m_Boundaries.y;
                } else {
                    bb[1].x = MAX(bb[1].x, pos.x);
                    bb[1].y = MAX(bb[1].y, pos.y);
                }

                width.push_back(pos.x);
                position.push_back(it);

                lod.vertices.resize(it * 4);
                lod.indices.resize(it * 6);
                lod.uv0.resize(it * 4);

                if(m_Alignment > Left) {
                    auto w = width.begin();
                    auto p = position.begin();
                    float shift = (bb[1].x - (*w)) / ((m_Alignment == Center) ? 2 : 1);
                    for(uint32_t i = 0; i < lod.vertices.size(); i++) {
                        if(uint32_t(i / 4) >= *p) {
                            w++;
                            p++;
                            shift = (bb[1].x - (*w)) / ((m_Alignment == Center) ? 2 : 1);
                        }
                        lod.vertices[i].x += shift;
                    }
                }

                Mesh::Surface surface;
                surface.mode = Mesh::MODE_TRIANGLES;
                surface.lods.push_back(lod);
                surface.aabb.setBox(bb[0], bb[1]);
                m_pMesh->setSurface(0, surface);
                m_pMesh->apply();
            }
        }
    }

    Font               *m_pFont;

    int32_t             m_Size;

    float               m_Space;

    float               m_Line;

    string              m_Text;

    Vector4             m_Color;

    Alignment           m_Alignment;

    bool                m_Kerning;

    MaterialInstance   *m_pMaterial;

    Mesh               *m_pMesh;

    Vector2             m_Boundaries;

    bool                m_Wrap;
};

TextRender::TextRender() :
        p_ptr(new TextRenderPrivate()) {

}

void TextRender::draw(ICommandBuffer &buffer, uint32_t layer) {
    Actor *a    = actor();
    if(p_ptr->m_pMesh && layer & a->layers()) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a->uuid()));
        }
        if(p_ptr->m_pMesh->surfacesCount() > 0) {
            buffer.drawMesh(a->transform()->worldTransform(), p_ptr->m_pMesh, 0, layer, p_ptr->m_pMaterial);
        }
        buffer.setColor(Vector4(1.0f));
    }
}

string TextRender::text() const {
    return p_ptr->m_Text;
}
void TextRender::setText(const string &text) {
    p_ptr->m_Text  = text;
    p_ptr->composeMesh();
}

Font *TextRender::font() const {
    return p_ptr->m_pFont;
}
void TextRender::setFont(Font *font) {
    p_ptr->m_pFont = font;
    if(p_ptr->m_pFont && p_ptr->m_pMaterial) {
        p_ptr->m_pMaterial->setTexture(OVERRIDE, p_ptr->m_pFont->texture());
    }
    p_ptr->composeMesh();
}

Material *TextRender::material() const {
    if(p_ptr->m_pMaterial) {
        return p_ptr->m_pMaterial->material();
    }
    return nullptr;
}

void TextRender::setMaterial(Material *material) {
    if(p_ptr->m_pMaterial) {
        delete p_ptr->m_pMaterial;
        p_ptr->m_pMaterial = nullptr;
    }

    if(material) {
        p_ptr->m_pMaterial = material->createInstance();
        p_ptr->m_pMaterial->setTexture(OVERRIDE, p_ptr->m_pFont->texture());
    }
}

int TextRender::fontSize() const {
    return p_ptr->m_Size;
}
void TextRender::setFontSize(int size) {
    p_ptr->m_Size  = size;
    p_ptr->composeMesh();
}

Vector4 TextRender::color() const {
    return p_ptr->m_Color;
}
void TextRender::setColor(const Vector4 &color) {
    p_ptr->m_Color = color;
    if(p_ptr->m_pMaterial) {
        p_ptr->m_pMaterial->setVector4("uni.color0", &p_ptr->m_Color);
    }
}

bool TextRender::wrap() const {
    return p_ptr->m_Wrap;
}
void TextRender::setWrap(bool wrap) {
    p_ptr->m_Wrap = wrap;
    p_ptr->composeMesh();
}

Vector2 TextRender::boundaries() const {
    return p_ptr->m_Boundaries;
}
void TextRender::setBoundaries(const Vector2 &value) {
    p_ptr->m_Boundaries = value;
    p_ptr->composeMesh();
}

Alignment TextRender::align() const {
    return p_ptr->m_Alignment;
}
void TextRender::setAlign(Alignment align) {
    p_ptr->m_Alignment = align;
    p_ptr->composeMesh();
}

bool TextRender::kerning() const {
    return p_ptr->m_Kerning;
}
void TextRender::setKerning(const bool kerning) {
    p_ptr->m_Kerning = kerning;
    p_ptr->composeMesh();
}

void TextRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(FONT);
        if(it != data.end()) {
            setFont(Engine::loadResource<Font>((*it).second.toString()));
        }
    }
    {
        auto it = data.find(MATERIAL);
        if(it != data.end()) {
            setMaterial(Engine::loadResource<Material>((*it).second.toString()));
        }
    }
}

VariantMap TextRender::saveUserData() const {
    VariantMap result   = Component::saveUserData();
    {
        Font *o = font();
        string ref  = Engine::reference(o);
        if(!ref.empty()) {
            result[FONT] = ref;
        }
    }
    {
        Material *o = material();
        string ref  = Engine::reference(o);
        if(!ref.empty()) {
            result[MATERIAL] = ref;
        }
    }
    return result;
}
