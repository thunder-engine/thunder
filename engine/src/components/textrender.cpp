#include "components/textrender.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/mesh.h"

#include "commandbuffer.h"
#include "utils.h"

#define FONT  "Font"

#define OVERRIDE "uni.texture0"

TextRender::TextRender() :
        m_pFont(nullptr),
        m_Size(16),
        m_Space(0),
        m_Line(0),
        m_Color(1.0f),
        m_Alignment(Left),
        m_Kerning(true),
        m_pMaterial(nullptr) {

    m_pMesh = Engine::objectCreate<Mesh>();
    m_pMesh->makeDynamic();
    m_pMesh->setFlags(Mesh::ATTRIBUTE_UV0);

    Material *material  = Engine::loadResource<Material>(".embedded/DefaultFont.mtl");
    if(material) {
        m_pMaterial = material->createInstance();
    }
}

void TextRender::draw(ICommandBuffer &buffer, uint32_t layer) {
    Actor *a    = actor();
    if(m_pMesh && layer & a->layers()) {
        if(layer & ICommandBuffer::RAYCAST) {
            buffer.setColor(ICommandBuffer::idToColor(a->uuid()));
        }

        if(m_pMesh->surfacesCount() > 0) {
            buffer.drawMesh(a->transform()->worldTransform(), m_pMesh, 0, layer, m_pMaterial);
        }

        buffer.setColor(Vector4(1.0f));
    }
}

void TextRender::composeMesh() {
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

            Vector3 bb[2];
            for(uint32_t i = 0; i < length; i++) {
                uint32_t ch = text[i];
                switch(ch) {
                    case ' ': {
                        pos = Vector3(pos.x + m_Space, pos.y, 0.0f);
                    } break;
                    case '\t': {
                        pos = Vector3(pos.x + m_Space * 4, pos.y, 0.0f);
                    } break;
                    case '\r': break;
                    case '\n': {
                        width.push_back(pos.x);
                        position.push_back(it);
                        bb[1].x = MAX(bb[1].x, pos.x);
                        pos = Vector3(0.0f, pos.y - m_Line, 0.0f);
                        bb[1].y = MAX(bb[1].y, pos.y);
                    } break;
                    default: {
                        if(m_Kerning) {
                            pos.x += m_pFont->requestKerning(ch, previous);
                        }
                        uint32_t index  = m_pFont->atlasIndex(ch);
                        Vector2Vector shape = m_pFont->shape(index);

                        bb[0].x = MIN(bb[0].x, shape[0].x);
                        bb[0].y = MIN(bb[0].y, shape[0].y);

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
            bb[1].x = MAX(bb[1].x, pos.x);
            bb[1].y = MAX(bb[1].y, pos.y);

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
                        lod.vertices[i].x += shift;
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

Mesh *TextRender::mesh() const {
    return m_pMesh;
}

string TextRender::text() const {
    return m_Text;
}
void TextRender::setText(const string &text) {
    m_Text  = text;
    composeMesh();
}

Font *TextRender::font() const {
    return m_pFont;
}
void TextRender::setFont(Font *font) {
    m_pFont = font;
    if(m_pFont && m_pMaterial) {
        m_pMaterial->setTexture(OVERRIDE, m_pFont->texture());
    }
    composeMesh();
}

int TextRender::fontSize() const {
    return m_Size;
}
void TextRender::setFontSize(int size) {
    m_Size  = size;
    composeMesh();
}

Vector4 TextRender::color() const {
    return m_Color;
}
void TextRender::setColor(const Vector4 &color) {
    m_Color = color;
    if(m_pMaterial) {
        m_pMaterial->setVector4("uni.color0", &m_Color);
    }
}

Alignment TextRender::align() const {
    return m_Alignment;
}
void TextRender::setAlign(Alignment align) {
    m_Alignment = align;
    composeMesh();
}

bool TextRender::kerning() const {
    return m_Kerning;
}
void TextRender::setKerning(const bool kerning) {
    m_Kerning = kerning;
    composeMesh();
}

void TextRender::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(FONT);
        if(it != data.end()) {
            setFont(Engine::loadResource<Font>((*it).second.toString()));
        }
    }
}

VariantMap TextRender::saveUserData() const {
    VariantMap result   = Component::saveUserData();
    {
        Font *f = font();
        string ref  = Engine::reference(f);
        if(!ref.empty()) {
            result[FONT]    = ref;
        }
    }
    return result;
}
