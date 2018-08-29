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

const string gEmbedded(".embedded/");

AMaterialGL::~AMaterialGL() {
    clear();
}

void AMaterialGL::loadUserData(const VariantMap &data) {
    Material::loadUserData(data);

    auto it = data.find("Shader");
    if(it == data.end()) {
        return ;
    }

    addPragma("material", (*it).second.toString());

    // Number of shader pairs calculation
    switch(m_MaterialType) {
        case PostProcess: {
            m_DoubleSided   = true;
            m_Tangent       = false;
            m_BlendMode     = Opaque;
            m_LightModel    = Unlit;
            m_Surfaces      = Static;

            m_Programs[0]    = buildShader(Fragment, gEmbedded + gPost);
        } break;
        case LightFunction: {
            m_DoubleSided   = true;
            m_Tangent       = false;
            m_BlendMode     = Opaque;
            m_LightModel    = Unlit;
            m_Surfaces      = Static;

            /// \todo should be removed
            setTexture("normalsMap",    nullptr);
            setTexture("diffuseMap",    nullptr);
            setTexture("paramsMap",     nullptr);
            setTexture("emissiveMap",   nullptr);
            setTexture("depthMap",      nullptr);
            setTexture("shadowMap",     nullptr);

            m_Programs[0]    = buildShader(Fragment, gEmbedded + gLight);
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

            m_Programs[0]       = buildShader(Fragment, gEmbedded + gSurface, define);
            define += "\n#define SIMPLE 1";
            m_Programs[Simple]  = buildShader(Fragment, gEmbedded + gSurface, define);
            define += "\n#define DEPTH 1";
            m_Programs[Depth]   = buildShader(Fragment, gEmbedded + gSurface, define);
        } break;
    }
}

uint32_t AMaterialGL::getProgram(uint16_t type) const {
    auto it = m_Programs.find(type);
    if(it != m_Programs.end()) {
        return it->second;
    }
    return 0;
}

uint32_t AMaterialGL::bind(ICommandBuffer &buffer, MaterialInstance *instance, uint8_t layer) {
    int32_t b   = blendMode();

    if((layer & ICommandBuffer::DEFAULT || layer & ICommandBuffer::SHADOWCAST) && //  || layer & ICommandBuffer::RAYCAST
       (b == Material::Additive || b == Material::Translucent)) {
        return 0;
    }
    if(layer & ICommandBuffer::TRANSLUCENT && b == Material::Opaque) {
        return 0;
    }

    uint16_t type   = 0;
    switch(layer) {
        case ICommandBuffer::RAYCAST: {
            type    = AMaterialGL::Simple;
        } break;
        case ICommandBuffer::SHADOWCAST: {
            type    = AMaterialGL::Depth;
        } break;
        default: break;
    }
    uint32_t program    = getProgram(type);
    if(!program) {
        return 0;
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

    if(b != Material::Opaque && !(layer & ICommandBuffer::RAYCAST)) {
        glEnable    ( GL_BLEND );
        if(b == Material::Translucent) {
            glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        } else {
            glBlendFunc ( GL_SRC_ALPHA, GL_ONE );
        }
        glBlendEquation(GL_FUNC_ADD);
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

    int32_t blend   = blendMode();
    if(blend == Material::Additive || blend == Material::Translucent) {
        glDisable   ( GL_BLEND );
    }
    glDisable( GL_CULL_FACE );
}

void AMaterialGL::clear() {
    Material::clear();

    m_Pragmas.clear();
#ifndef THUNDER_MOBILE
    addPragma("version", "#version 430 core");
#else
    addPragma("version", "#version 300 es");
#endif
    for(auto it : m_Programs) {
        glDeleteProgram(it.second);
    }
    m_Programs.clear();
}

uint32_t AMaterialGL::buildShader(uint8_t type, const string &path, const string &define) {
    uint32_t result = 0;

    uint32_t t;
    switch(type) {
#ifndef THUNDER_MOBILE
        case Geometry:  t   = GL_GEOMETRY_SHADER; break;
#endif
        case Vertex:    t   = GL_VERTEX_SHADER;   break;
        default:        t   = GL_FRAGMENT_SHADER; break;
    }
    string src  = loadIncludes(path, define);
    const char *data    = src.c_str();

    uint32_t shader = glCreateShader(t);
    glShaderSource(shader, 1, &data, nullptr);
    glCompileShader(shader);
    checkShader(shader, path);

    result = glCreateProgram();
    if (result) {
        GLint compiled = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        glProgramParameteriEXT(result, GL_PROGRAM_SEPARABLE_EXT, GL_TRUE);
        if(compiled) {
            glAttachShader(result, shader);
            glLinkProgram(result);
            glDetachShader(result, shader);
        }
        checkShader(result, path, true);
    }
    glDeleteShader(shader);

    if(type == Fragment) {
        uint8_t t   = 0;
        for(auto it : m_Textures) {
            int location    = glGetUniformLocation(result, it.first.c_str());
            if(location > -1) {
                glProgramUniform1iEXT(result, location, t);
            }
            t++;
        }
    }

    return result;
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
                glGetShaderInfoLog(shader, value, nullptr, buff);
            } else {
                glGetProgramInfoLog(shader, value, nullptr, buff);
            }
            Log(Log::ERR) << "[ Render::ShaderGL ]" << path.c_str() << "\n[ Said ]" << buff;
            delete []buff;
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
