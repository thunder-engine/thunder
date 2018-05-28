#include "resources/amaterialgl.h"

#include "agl.h"
#include "commandbuffergl.h"

#include "resources/text.h"
#include "resources/atexturegl.h"

#include "components/staticmesh.h"

#include <file.h>
#include <log.h>

#include <regex>
#include <sstream>

#include <timer.h>

const regex include("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">][^?]*");
const regex pragma("^[ ]*#[ ]*pragma[ ]+(.*)[^?]*");

const char *gPost       = "PostEffect.frag";
const char *gSurface    = "Surface.frag";
const char *gLight      = "DirectLight.frag";
const char *gVertex     = "BasePass.vert";

const string gEmbedded(".embedded/");

AMaterialGL::AMaterialGL() :
        Material() {

}

AMaterialGL::~AMaterialGL() {

}

void AMaterialGL::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    auto it = data.find("Shader");
    if(it == data.end()) {
        return ;
    }

    addPragma("material", (*it).second.toString());

    ProgramMap fragments;
    // Number of shader pairs calculation
    switch(m_MaterialType) {
        case PostProcess: {
            m_DoubleSided   = true;
            m_Tangent       = false;
            m_BlendMode     = Opaque;
            m_LightModel    = Unlit;
            m_Surfaces      = Static;

            fragments[0]    = buildShader(Fragment, loadIncludes(gEmbedded + gPost), gEmbedded + gPost);
        } break;
        case LightFunction: {
            m_DoubleSided   = true;
            m_Tangent       = false;
            m_BlendMode     = Opaque;
            m_LightModel    = Unlit;
            m_Surfaces      = Static;

            fragments[0]    = buildShader(Fragment, loadIncludes(gEmbedded + gLight), gEmbedded + gPost);
            /// \todo should be removed
            setTexture("normalsMap",    nullptr);
            setTexture("diffuseMap",    nullptr);
            setTexture("paramsMap",     nullptr);
            setTexture("emissiveMap",   nullptr);
            setTexture("depthMap",      nullptr);
            setTexture("shadowMap",     nullptr);

        } break;
        default: { // Surface type
            string define;
            switch(m_BlendMode) {
                case Additive: {
                    define  = "#define BLEND_ADDITIVE 1";
                } break;
                case Translucent: {
                    define  = "#define BLEND_TRANSLUCENT 1";
                } break;
                default: {
                    define  = "#define BLEND_OPAQUE 1";
                } break;
            }
            switch(m_LightModel) {
                case Lit: {
                    define += "\n#define MODEL_LIT 1";
                } break;
                case Subsurface: {
                    define += "\n#define MODEL_SUBSURFACE 1";
                } break;
                default: {
                    define += "\n#define MODEL_UNLIT 1";
                } break;
            }
            if(m_Tangent) {
                define += "\n#define TANGENT 1";
            }

            fragments[0]        = buildShader(Fragment, loadIncludes(gEmbedded + gSurface, define), gEmbedded + gPost);
            define += "\n#define SIMPLE 1";
            fragments[Simple]   = buildShader(Fragment, loadIncludes(gEmbedded + gSurface, define), gEmbedded + gPost);
            // if cast shadows
            fragments[Depth]    = buildShader(Fragment, loadIncludes(gEmbedded + gSurface, define + "\n#define DEPTH 1"), gEmbedded + gPost);
        } break;
    }

    if(m_Surfaces & Static) {
        for(auto it : fragments) {
            m_Programs[Static | it.first]  = buildProgram(it.second, "#define TYPE_STATIC 1");
        }
    }
    if(m_Surfaces & Skinned) {
        for(auto it : fragments) {
            m_Programs[Skinned | it.first]  = buildProgram(it.second, "#define TYPE_SKINNED 1");
        }
    }
    if(m_Surfaces & Billboard) {
        for(auto it : fragments) {
            m_Programs[Billboard | it.first]    = buildProgram(it.second, "#define TYPE_BILLBOARD 1");
        }
    }
    if(m_Surfaces & Oriented) {
        for(auto it : fragments) {
            m_Programs[Oriented | it.first] = buildProgram(it.second, "#define TYPE_AXISALIGNED 1");
        }
    }
}

uint32_t AMaterialGL::getProgram(uint16_t type) const {
    auto it = m_Programs.find(type);
    if(it != m_Programs.end()) {
        return it->second;
    }
    return 0;
}

uint32_t AMaterialGL::bind(ICommandBuffer &buffer, MaterialInstance *instance, uint8_t layer, uint16_t type) {
    uint8_t b   = blendMode();

    if(!instance) {
        return 0;
    }

    if((layer & ICommandBuffer::DEFAULT || layer & ICommandBuffer::SHADOWCAST) && //  || layer & ICommandBuffer::RAYCAST
       (b == Material::Additive || b == Material::Translucent)) {
        return 0;
    }
    if(layer & ICommandBuffer::TRANSLUCENT && b == Material::Opaque) {
        return 0;
    }

    switch(layer) {
        case ICommandBuffer::RAYCAST:    {
            type   |= AMaterialGL::Simple;
        } break;
        case ICommandBuffer::SHADOWCAST: {
            type   |= AMaterialGL::Depth;
        } break;
        default: break;
    }
    uint32_t program    = getProgram(type);
    if(!program) {
        return 0;
    }

    glUseProgram(program);

    int location    = glGetUniformLocation(program, "_time");
    if(location > -1) {
        glUniform1f(location, Timer::time());
    }
    // Push uniform values to shader
    for(const auto &it : instance->params()) {
        location    = glGetUniformLocation(program, it.first.c_str());
        if(location > -1) {
            const MaterialInstance::Info &data  = it.second;
            for(uint32_t i = 0; i < data.count; i++) {
                switch(data.type) {
                    case 0: break;
                    case MetaType::INTEGER: glUniform1i         (location + i, *static_cast<const int32_t *>(data.ptr)); break;
                    case MetaType::VECTOR2: glUniform2fv        (location + i, 1, static_cast<const float *>(data.ptr)); break;
                    case MetaType::VECTOR3: glUniform3fv        (location + i, 1, static_cast<const float *>(data.ptr)); break;
                    case MetaType::VECTOR4: glUniform4fv        (location + i, 1, static_cast<const float *>(data.ptr)); break;
                    case MetaType::MATRIX4: glUniformMatrix4fv  (location + i, 1, GL_FALSE, static_cast<const float *>(data.ptr)); break;
                    default:                glUniform1f         (location + i, *static_cast<const float *>(data.ptr)); break;
                }
            }
        }
    }

    if(m_DepthTest) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if(!isDoubleSided() && !(layer & ICommandBuffer::RAYCAST)) {
        glEnable    ( GL_CULL_FACE );
        glCullFace  ( GL_BACK );
    }

    uint8_t blend   = blendMode();
    if(blend != Material::Opaque && !(layer & ICommandBuffer::RAYCAST)) {
        glEnable    ( GL_BLEND );
        if(blend == Material::Translucent) {
            glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        } else {
            glBlendFunc ( GL_SRC_ALPHA, GL_ONE );
        }
        glBlendEquation(GL_FUNC_ADD);
    }

    glEnable(GL_TEXTURE_2D);
    uint8_t i   = 0;
    for(auto it : m_Textures) {
        int location    = glGetUniformLocation(program, it.first.c_str());
        if(location > -1) {
            glUniform1i(location, i);
        }

        const Texture *texture  = static_cast<const ATextureGL *>(it.second);
        glActiveTexture(GL_TEXTURE0 + i);
        if(instance) {
            const Texture *t    = instance->texture(it.first.c_str());
            if(t) {
                texture = t;
            } else {
                t = buffer.texture(it.first.c_str());
                if(t) {
                    texture = t;
                }
            }
        }
        if(texture) {
            glBindTexture((texture->isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, (uint32_t)texture->nativeHandle());
        }
        i++;
    }

    return program;
}

void AMaterialGL::unbind(uint8_t) {
    uint8_t t   = 0;
    for(auto it : m_Textures) {
        glActiveTexture(GL_TEXTURE0 + t);
        const Texture *texture    = it.second;
        if(texture) {
            glBindTexture((texture->isCubemap()) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, 0);
        }
        t++;
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);

    glUseProgram(0);

    uint8_t blend   = blendMode();
    if(blend == Material::Additive || blend == Material::Translucent) {
        glDisable   ( GL_BLEND );
    }
    glDisable( GL_CULL_FACE );
}

void AMaterialGL::clear() {
    Material::clear();

    m_Pragmas.clear();
#ifdef GL_ES_VERSION_2_0
    addPragma("version", "");
#else
    addPragma("version", "#version 410 core");
    //addPragma("version", "#version 300 es");
#endif
    for(auto it : m_Programs) {
        glDeleteProgram(it.second);
    }
    m_Programs.clear();
}

uint32_t AMaterialGL::buildShader(uint8_t type, const string &src, const string &path) {
    uint32_t shader = 0;
    switch(type) {
        case Vertex:    shader  = glCreateShader(GL_VERTEX_SHADER);   break;
#ifndef GL_ES_VERSION_2_0
        case Geometry:  shader  = glCreateShader(GL_GEOMETRY_SHADER); break;
#endif
        default:        shader  = glCreateShader(GL_FRAGMENT_SHADER); break;
    }
    const char *data    = src.c_str();

    if(shader) {
        glShaderSource  (shader, 1, &data, NULL);
        glCompileShader (shader);
        bool result = true;
        result     &= checkShader(shader, path);
        if(result) {
            return shader;
        }
        glDeleteShader(shader);
    }
    return 0;
}

uint32_t AMaterialGL::buildProgram(uint32_t fragment, const string &define) {
    uint32_t vertex     = buildShader(Vertex, loadIncludes(gEmbedded + gVertex, define));

    if(fragment && vertex) {
        uint32_t program    = glCreateProgram();
        if(program) {
            glAttachShader  (program, vertex);
            glAttachShader  (program, fragment);

            glLinkProgram   (program);
            checkShader(program, string(), true);

            glDetachShader  (program, fragment);
            glDetachShader  (program, vertex);

            glUseProgram(program);
            uint8_t t   = 0;
            for(auto it : m_Textures) {
                int location    = glGetUniformLocation(program, it.first.c_str());
                if(location > -1) {
                    glUniform1i(location, t);
                }
                t++;
            }
            glUseProgram(0);
            return program;
        }
    }
    return 0;
}

bool AMaterialGL::checkShader(uint32_t shader, const string &path, bool link) {
    int value   = 0;

    if(!link) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &value);
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &value);
    }

    if(value != GL_TRUE) {
        if(!link) {
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &value);
        } else {
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &value);
        }
        if(value) {
            char *buff  = new char[value + 1];
            if(!link) {
                glGetShaderInfoLog(shader, value, NULL, buff);
            } else {
                glGetProgramInfoLog(shader, value, NULL, buff);
            }
            Log(Log::ERR) << "[ Render::ShaderGL ]" << path.c_str() << "\n[ Said ]" << buff;
            delete []buff;
        } else {
            //m_Valid = false;
        }
        return false;
    }
    return true;
}

void AMaterialGL::addPragma(const string &key, const string &value) {
    m_Pragmas[key]  = m_Pragmas[key].append(value).append("\r\n");
}

string AMaterialGL::parseData(const string &data, const string &define) {
    stringstream input;
    stringstream output;
    input << data;

    string line;
    while( getline(input, line) ) {
        smatch matches;
        if(regex_match(line, matches, include)) {
            output << loadIncludes(string(matches[1]), define) << endl;
        } else if(regex_match(line, matches, pragma)) {
            if(matches[1] == "flags") {
                output << define << endl;
            } else {
                auto it = m_Pragmas.find(matches[1]);
                if(it != m_Pragmas.end()) {
                    output << m_Pragmas[matches[1]] << endl;
                }
            }
        } else {
            output << line << endl;
        }
    }
    return output.str();
}

string AMaterialGL::loadIncludes(const string &path, const string &define) {
    Text *text  = Engine::loadResource<Text>(path);
    if(text) {
        return parseData(text->text(), define);
    }

    return string();
}
