#include "components/textmesh.h"
#include "components/actor.h"
#include "components/transform.h"

#include "resources/mesh.h"

#include "commandbuffer.h"
#include "utils.h"

#define FONT  "Font"

TextMesh::TextMesh() :
        m_pFont(nullptr),
        m_Size(16),
        m_Space(0),
        m_Line(0),
        m_Color(1.0f) {
    m_pMesh = Engine::objectCreate<Mesh>();
    m_pMesh->makeDynamic();
    m_pMesh->setFlags(Mesh::ATTRIBUTE_UV0);

    Material *material  = Engine::loadResource<Material>(".embedded/DefaultFont.mtl");
    if(material) {
        m_Materials.push_back(material->createInstance());
    }
}

void TextMesh::composeMesh() {
    if(m_pFont) {
        m_Space = m_pFont->spaceWidth(m_Size);
        m_Line  = m_pFont->lineHeight(m_Size);

        //m_pMesh->clear();

        u32string text  = Utils::utf8ToUtf32(m_Text);
        m_pFont->requestCharacters(text, m_Size);

        uint32_t length = m_pFont->length(text);
        if(length) {
            Mesh::Lod lod;
            lod.vertices.resize(length * 4);
            lod.indices.resize(length * 6);
            lod.uv0.resize(length * 4);

            Vector3 pos;
            for(uint32_t i = 0; i < length; i++) {
                uint32_t ch = text[i];
                switch(ch) {
                    case ' ': {
                        pos = Vector3(pos.x + m_Space, pos.y, 0.0f);
                    } break;
                    case '\n': {
                        pos = Vector3(0.0f, pos.y - m_Line, 0.0f);
                    } break;
                    default: {
                        uint32_t index  = m_pFont->atlasIndex(ch, m_Size);
                        Vector2Vector shape = m_pFont->shape(index);
                        lod.vertices[i * 4 + 0] = pos + Vector3(shape[0], 0.0f);
                        lod.vertices[i * 4 + 1] = pos + Vector3(shape[1], 0.0f);
                        lod.vertices[i * 4 + 2] = pos + Vector3(shape[2], 0.0f);
                        lod.vertices[i * 4 + 3] = pos + Vector3(shape[3], 0.0f);

                        Vector4 uv  = m_pFont->uv(index);
                        lod.uv0[i * 4 + 0]  = Vector2(uv.x, uv.y);
                        lod.uv0[i * 4 + 1]  = Vector2(uv.z, uv.y);
                        lod.uv0[i * 4 + 2]  = Vector2(uv.z, uv.w);
                        lod.uv0[i * 4 + 3]  = Vector2(uv.x, uv.w);

                        lod.indices[i * 6 + 0]  = i * 4 + 0;
                        lod.indices[i * 6 + 1]  = i * 4 + 1;
                        lod.indices[i * 6 + 2]  = i * 4 + 2;

                        lod.indices[i * 6 + 3]  = i * 4 + 0;
                        lod.indices[i * 6 + 4]  = i * 4 + 2;
                        lod.indices[i * 6 + 5]  = i * 4 + 3;

                        pos += Vector3(shape[2].x - shape[0].x, 0.0f, 0.0f);
                    } break;
                }
            }

            Vector3 bb[2];
            for(uint32_t i = 0; i < lod.vertices.size(); i++) {
                bb[0].x = MIN(bb[0].x, lod.vertices[i].x);
                bb[0].y = MIN(bb[0].y, lod.vertices[i].y);
                bb[0].z = MIN(bb[0].z, lod.vertices[i].z);

                bb[1].x = MAX(bb[1].x, lod.vertices[i].x);
                bb[1].y = MAX(bb[1].y, lod.vertices[i].y);
                bb[1].z = MAX(bb[1].z, lod.vertices[i].z);
            }

            Mesh::Surface surface;
            surface.mode    = Mesh::MODE_TRIANGLES;
            surface.lods.push_back(lod);
            surface.aabb.setBox(bb[0], bb[1]);
            m_pMesh->setSurface(0, surface);
            m_pMesh->apply();
        }
    }
}

string TextMesh::text() const {
    return m_Text;
}

void TextMesh::setText(const string &text) {
    m_Text  = text;
    composeMesh();
}

Font *TextMesh::font() const {
    return m_pFont;
}

void TextMesh::setFont(Font *font) {
    m_pFont = font;
    if(m_pFont && !m_Materials.empty()) {
        m_Materials[0]->setTexture("texture0", m_pFont->texture());
    }
    composeMesh();
}

int TextMesh::fontSize() const {
    return m_Size;
}

void TextMesh::setFontSize(int size) {
    m_Size  = size;
    composeMesh();
}

Vector4 TextMesh::color() const {
    return m_Color;
}

void TextMesh::setColor(const Vector4 &color) {
    m_Color = color;
    if(!m_Materials.empty()) {
        m_Materials[0]->setVector4("color0", &m_Color);
    }
}

void TextMesh::loadUserData(const VariantMap &data) {
    Component::loadUserData(data);
    {
        auto it = data.find(FONT);
        if(it != data.end()) {
            setFont(Engine::loadResource<Font>((*it).second.toString()));
        }
    }
}

VariantMap TextMesh::saveUserData() const {
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
