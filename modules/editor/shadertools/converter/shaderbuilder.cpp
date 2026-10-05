#include "shaderbuilder.h"

#include <resources/texture.h>

#include <url.h>
#include <log.h>
#include <file.h>

#include <components/actor.h>
#include <components/meshrender.h>

#include <resources/mesh.h>
#include <resources/material.h>

#include <commandbuffer.h>

#include <editor/projectsettings.h>
#include <editor/nativecodebuilder.h>

#include "../../config.h"

#include <QFile>

#include <algorithm>
#include <regex>
#include <pugixml.hpp>

#define SHADER_FORMAT_VERSION 14
#define ASSET_FORMAT_VERSION 15

enum ShaderFlags {
    Compute = (1<<0),
    Static = (1<<1),
    Skinned = (1<<2),
    Particle = (1<<3)
};

namespace  {
    const char *gValue("value");
    const char *gName("name");

    const char *gFragment("fragment");
    const char *gVertex("vertex");
    const char *gGeometry("geometry");
    const char *gCompute("compute");

    const char *gProperties("properties");
    const char *gPass("pass");

    const char *gTwoSided("twoSided");
    const char *gLightModel("lightModel");

    const char *gOperation("op");
    const char *gColorOperation("colorOp");
    const char *gAlphaOperation("alphaOp");

    const char *gDestination("dst");
    const char *gColorDestination("colorDst");
    const char *gAlphaDestination("alphaDst");

    const char *gSource("src");
    const char *gColorSource("colorSrc");
    const char *gAlphaSource("alphaSrc");

    const char *gTest("test");
    const char *gWrite("write");
    const char *gCompare("comp");
    const char *gFail("fail");
    const char *gZFail("zFail");

    const char *gCompBack("compBack");
    const char *gCompFront("compFront");
    const char *gFailBack("failBack");
    const char *gFailFront("failFront");
    const char *gPassBack("passBack");
    const char *gPassFront("passFront");
    const char *gZFailBack("zFailBack");
    const char *gZFailFront("zFailFront");

    const char *gReadMask("readMask");
    const char *gWriteMask("writeMask");
    const char *gReference("ref");

    const char *gTexture2D("texture2d");
    const char *gTextureCubemap("samplercube");

    void normalizeTextureReferences(VariantMap &data) {
        auto it = data.find(TEXTURES);
        if(it == data.end()) {
            return;
        }

        VariantList textures = it->second.toList();
        for(auto &texture : textures) {
            VariantList fields = texture.toList();
            if(fields.empty()) {
                continue;
            }

            auto path = fields.begin();
            TString texturePath = path->toString();
            if(!texturePath.isEmpty()) {
                Texture *resource = Engine::loadResource<Texture>(texturePath);
                *path = resource ? Engine::reference(resource) : TString();
                texture = fields;
            }
        }

        it->second = textures;
    }

    bool hasCompiledShaderPayload(const Variant &shader) {
        VariantList fields = shader.toList();
        if(fields.size() < 3) {
            return false;
        }

        Variant payload = fields.front();
        if(payload.userType() == MetaType::BYTEARRAY) {
            return !payload.toByteArray().empty();
        }
        return !payload.toString().isEmpty();
    }
};

ShaderBuilderSettings::ShaderBuilderSettings() {
    setVersion(ASSET_FORMAT_VERSION);
}

StringList ShaderBuilderSettings::typeNames() const {
    return { MetaType::name<Material>() };
}

AssetConverterSettings *ShaderBuilder::createSettings() {
    return new ShaderBuilderSettings();
}

ShaderBuilder::ShaderBuilder() :
        AssetConverter() {

}

ShaderBuilderSettings::Rhi ShaderBuilder::currentRhi() {
    static RhiMap rhiMap = {
        {"RenderGL", ShaderBuilderSettings::Rhi::OpenGL},
        {"RenderVK", ShaderBuilderSettings::Rhi::Vulkan},
        {"RenderMT", ShaderBuilderSettings::Rhi::Metal},
    };

    static ShaderBuilderSettings::Rhi rhi = ShaderBuilderSettings::Rhi::Invalid;
    if(rhi == ShaderBuilderSettings::Rhi::Invalid) {
        if(qEnvironmentVariableIsSet(qPrintable(gRhi))) {
            rhi = rhiMap[qEnvironmentVariable(qPrintable(gRhi)).toStdString()];
        } else {
            rhi = ShaderBuilderSettings::Rhi::OpenGL;
        }
    }

    return rhi;
}

TString uniformDataHelper(const Uniform &uniform, int &offset, int &sub) {
    static const char *compNames = "xyzw";

    TString comp = ".";

    TString prefix;

    switch(uniform.type) {
    case MetaType::BOOLEAN: {
        prefix = "floatBitsToInt(";
        comp += TString(1, compNames[sub]) + ") != 0";
        sub++;
    } break;
    case MetaType::INTEGER: {
        prefix = "floatBitsToInt(";
        comp += TString(1, compNames[sub]) + ")";
        sub++;
    } break;
    case MetaType::FLOAT: {
        comp += TString(1, compNames[sub]);
        sub++;
    } break;
    case MetaType::VECTOR2: {
        if(sub > 2) {
            sub = 0;
            offset++;
        }

        comp += TString(1, compNames[sub]);
        sub++;
        comp += compNames[sub];
        sub++;
    } break;
    case MetaType::VECTOR3: {
        if(sub > 1) {
            sub = 0;
            offset++;
        }

        comp += TString(1, compNames[sub]);
        sub++;
        comp += compNames[sub];
        sub++;
        comp += compNames[sub];
        sub++;
    } break;
    case MetaType::VECTOR4: {
        comp.clear();
        sub = 4;
    } break;
    case MetaType::MATRIX4: {
        comp.clear();
        sub = 0;
        TString data = "mat4(\n";
        data += "        instance.data[_instanceOffset + " + std::to_string(offset) + "],\n";
        offset++;
        data += "        instance.data[_instanceOffset + " + std::to_string(offset) + "],\n";
        offset++;
        data += "        instance.data[_instanceOffset + " + std::to_string(offset) + "],\n";
        offset++;
        data += "        instance.data[_instanceOffset + " + std::to_string(offset) + "])";
        offset++;

        return data;
    } break;
    default: break;
    }

    return prefix + "instance.data[_instanceOffset + " + std::to_string(offset) + "]" + comp;
}

void ShaderBuilder::buildInstanceData(const VariantMap &user, PragmaMap &pragmas) {
    TString result;

    TString objectId =
    "    _objectId = vec4(instance.data[_instanceOffset + 0].w,\n"
    "                     instance.data[_instanceOffset + 1].w,\n"
    "                     instance.data[_instanceOffset + 2].w,\n"
    "                     instance.data[_instanceOffset + 3].w);";

    int offset = 4;

    TString uniforms;
    auto it = user.find(UNIFORMS);
    if(it != user.end()) {
        int sub = 0;

        TString tab("    ");

        for(auto &p : it->second.toList()) {
            Uniform uniform = uniformFromVariant(p);

            TString data;

            TString array;
            if(uniform.count > 1) {
                array = TString("[%1]").arg(TString::number(uniform.count));
                data = uniform.typeName + array + "(\n";
            }

            for(int i = 0; i < uniform.count; i++) {
                if(uniform.count > 1) {
                    data += tab + tab;
                }
                data += uniformDataHelper(uniform, offset, sub);
                if(uniform.count > 1) {
                    if(i < uniform.count - 1) {
                        data += ",\n";
                    } else {
                        data += "\n";
                    }
                } else {
                    data += ";\n";
                }

                if(sub >= 4) {
                    sub = 0;
                    offset++;
                }
            }

            if(uniform.count > 1) {
                data += tab + ");\n";
            }

            uniforms += tab + uniform.typeName + " " + uniform.name + array + " = " + data;
        }

        if(sub > 0) {
            offset++;
        }
    }

    result += uniforms;

    pragmas["instance"] = result;
    pragmas["objectId"] = objectId;
    pragmas["offset"] = "    _instanceOffset = gl_InstanceIndex * " + std::to_string(offset) + ";\n";
    pragmas["skinOffset"] = "const int skinOffset = " + std::to_string(offset) + ";\n";
}

TString ShaderBuilder::rhiDefines(ShaderBuilderSettings::Rhi rhi) {
    TString result;
    if(rhi == ShaderBuilderSettings::Rhi::Vulkan) {
        result += "\n#define VULKAN";
    } else if(rhi == ShaderBuilderSettings::Rhi::Metal) {
        result += "\n#define METAL";
    }
    return result;
}

void ShaderBuilder::setMaterialProperties(VariantMap &data, int materialType, bool doubleSided, int lightingModel) {
    VariantList properties;
    properties.push_back(materialType);
    properties.push_back(doubleSided);
    properties.push_back(lightingModel);
    data[PROPERTIES] = properties;
}

void ShaderBuilder::setMaterialBlendState(VariantMap &data, const Material::BlendState &state) {
    data[BLENDSTATE] = toVariant(state);
}

void ShaderBuilder::setMaterialDepthState(VariantMap &data, const Material::DepthState &state) {
    data[DEPTHSTATE] = toVariant(state);
}

void ShaderBuilder::setMaterialStencilState(VariantMap &data, const Material::StencilState &state) {
    data[STENCILSTATE] = toVariant(state);
}

void ShaderBuilder::setMaterialResources(VariantMap &data, const VariantList &textures, const VariantList &uniforms) {
    data[TEXTURES] = textures;
    data[UNIFORMS] = uniforms;
}

VariantList ShaderBuilder::materialTexture(const TString &path, const TString &name, int32_t binding, int32_t flags) {
    return {path, name, binding, flags};
}

VariantList ShaderBuilder::materialUniform(const Variant &value, uint32_t size, const TString &name) {
    return {value, size, name};
}

Actor *ShaderBuilder::createActor(const AssetConverterSettings *settings, const TString &guid) const {
    Actor *object = Engine::composeActor<MeshRender>("");
    MeshRender *mesh = object->getComponent<MeshRender>();
    if(mesh) {
        Mesh *m = Engine::loadResource<Mesh>(".embedded/sphere.fbx/Sphere001");
        if(m) {
            mesh->setMesh(m);
            Material *mat = Engine::loadResource<Material>(guid);
            if(mat) {
                mesh->setMaterial(mat);
            }
        }
    }

    return object;
}

TString ShaderBuilder::templatePath() const {
    return ":/templates/Material.mtl";
}

StringList ShaderBuilder::suffixes() const {
    return {"mtl", "shader", "compute"};
}

AssetConverter::ReturnCode ShaderBuilder::convertFile(AssetConverterSettings *settings) {
    ShaderBuilderSettings *builderSettings = static_cast<ShaderBuilderSettings *>(settings);
    static const RhiMap rhis = {
        {"RenderGL", ShaderBuilderSettings::Rhi::OpenGL},
        {"RenderVK", ShaderBuilderSettings::Rhi::Vulkan},
        {"RenderMT", ShaderBuilderSettings::Rhi::Metal},
    };

    Url info(builderSettings->source());
    VariantMap dataByRhi;

    ShaderGraph nodeGraph;
    if(info.suffix() == "mtl") {
        nodeGraph.load(builderSettings->source());
        if(!nodeGraph.buildGraph()) {
            return InternalError;
        }
        if(builderSettings->currentVersion() != builderSettings->version()) {
            nodeGraph.save(builderSettings->source());
        }
    }

    for(const auto &rhi : rhis) {
        VariantMap data;
        if(info.suffix() == "mtl") {
            data = nodeGraph.dataForRhi(static_cast<int32_t>(rhi.second));
        } else if(info.suffix() == "shader") {
            parseShaderFormat(rhi.second, builderSettings->source(), data);
        } else if(info.suffix() == "compute") {
            parseShaderFormat(rhi.second, builderSettings->source(), data, Compute);
        }

        if(data.empty()) {
            return InternalError;
        }

        if(!compileData(rhi.second, data)) {
            aError() << "Failed to compile shader variant for " << rhi.first << ".";
            return InternalError;
        }
        normalizeTextureReferences(data);
        dataByRhi[rhi.first] = data;
    }

    Material *material = Engine::loadResource<Material>(settings->destination());
    if(material == nullptr) {
        material = Engine::objectCreate<Material>(settings->destination());
    }
    if(material == nullptr) {
        return InternalError;
    }

    uint32_t uuid = settings->info().id;
    if(uuid == 0) {
        uuid = Engine::generateUUID();
        settings->info().id = uuid;
    }

    if(material->uuid() != uuid) {
        Engine::replaceUUID(material, uuid);
    }

    auto activeRhi = std::find_if(rhis.begin(), rhis.end(), [](const auto &rhi) {
        return rhi.second == currentRhi();
    });
    if(activeRhi == rhis.end()) {
        return InternalError;
    }

    auto activeData = dataByRhi.find(activeRhi->first);
    if(activeData == dataByRhi.end()) {
        return InternalError;
    }

    VariantMap activeRhiData;
    activeRhiData[activeRhi->first] = activeData->second;
    ResourceSystem::loadResourceData(material, activeRhiData);
    material->setRhiData(dataByRhi);

    return settings->saveBinary(material, settings->absoluteDestination());
}

bool ShaderBuilder::compileData(ShaderBuilderSettings::Rhi rhi, VariantMap &data) {
    uint32_t version = 430;
    bool es = false;

    NativeCodeBuilder *builder = Editor::project()->currentBuilder();
    if(builder->isEmbedded()) {
        version = 300;
        es = true;
    }
    SpirVConverter::setGlslVersion(version, es);

    auto shader = data.find("Shader");
    if(shader != data.end()) {
        shader->second = compile(rhi, shader->second.toString(), data, EShLangCompute);
        return hasCompiledShaderPayload(shader->second);
    }

    auto compileStage = [rhi, &data](const TString &key, EShLanguage stage, bool required) {
        auto it = data.find(key);
        if(it == data.end()) {
            return !required;
        }

        it->second = compile(rhi, it->second.toString(), data, stage);
        return hasCompiledShaderPayload(it->second);
    };

    if(!compileStage(FRAGMENT, EShLangFragment, true)) {
        return false;
    }
    {
        auto it = data.find(VISIBILITY);
        if(it != data.end()) {
            if(!compileStage(VISIBILITY, EShLangFragment, false)) {
                return false;
            }
        }
    }

    if(!compileStage(STATIC, EShLangVertex, true)) {
        return false;
    }

    if(!compileStage(PARTICLE, EShLangVertex, false)) {
        return false;
    }

    if(!compileStage(SKINNED, EShLangVertex, false)) {
        return false;
    }

    return true;
}

Variant ShaderBuilder::compile(ShaderBuilderSettings::Rhi rhi, const TString &buff, VariantMap &data, EShLanguage stage) {
    SpirVConverter::Inputs inputs;

    VariantList result;
    std::vector<uint32_t> spv = SpirVConverter::glslToSpv(buff, inputs, static_cast<EShLanguage>(stage));
    if(!spv.empty()) {
        switch(rhi) {
            case ShaderBuilderSettings::Rhi::OpenGL: result.push_back(Variant(SpirVConverter::spvToGlsl(spv))); break;
            case ShaderBuilderSettings::Rhi::Metal: result.push_back(Variant(SpirVConverter::spvToMetal(spv, inputs, stage))); break;
            //case ShaderBuilderSettings::Rhi::DirectX: result.push_back(Variant(SpirVConverter::spvToHlsl(spv))); break;
            default: {
                ByteArray array;
                array.resize(spv.size() * sizeof(uint32_t));
                memcpy(&array[0], &spv[0], array.size());
                result.push_back(Variant(array));
                break;
            }
        }
    }

    std::sort(inputs.begin(), inputs.end(), [](const SpirVConverter::Input &left, const SpirVConverter::Input &right) {
        return left.location < right.location;
    });

    VariantList attributes;
    VariantList uniforms;
    for(auto &input : inputs) {
        switch(input.type) {
        case SpirVConverter::Stage: {
            VariantList attribute;
            attribute.push_back(input.format);
            attribute.push_back(input.location);

            attributes.push_back(attribute);
        } break;
        case SpirVConverter::Uniform: {
            VariantList uniform;
            uniform.push_back(input.name);
            uniform.push_back(input.location);

            uniforms.push_back(uniform);
        } break;
        case SpirVConverter::Image: {
            auto texturesBlock = data.find(TEXTURES);
            if(texturesBlock != data.end()) {
                VariantList list = (*texturesBlock).second.value<VariantList>();
                for(auto &texture : list) {
                    VariantList fields = texture.value<VariantList>();
                    auto field = fields.begin();
                    ++field; // Skip path field
                    TString name = field->toString();
                    if(name == input.name) {
                        ++field;
                        *field = input.location;

                        texture = fields;
                        data[TEXTURES] = list;

                        break;
                    }
                }
            }
        } break;
        default: break;
        }
    }
    result.push_back(uniforms);
    result.push_back(attributes);

    return result;
}

bool ShaderBuilder::parseShaderFormat(ShaderBuilderSettings::Rhi rhi, const TString &path, VariantMap &user, int flags) {
    File file(path.data());
    if(file.open(File::Read | File::Text)) {
        ByteArray data = file.readAll();
        file.close();

        pugi::xml_document doc;
        if(doc.load_string(TString(data).data())) {
            std::map<TString, TString> shaders;

            int materialType = Material::Surface;
            int lightingModel = Material::Unlit;

            pugi::xml_node shader = doc.document_element();

            uint32_t version = shader.attribute("version").as_int();

            pugi::xml_node element = shader.first_child();
            while(element) {
                std::string name(element.name());
                if(name == gFragment || name == gVertex || name == gGeometry) {
                    shaders[name] = element.child_value();
                } else if(name == gCompute) {
                    shaders[name] = element.child_value();
                } else if(name == gProperties) {
                    if(!parseProperties(element, user)) {
                        return false;
                    }
                } else if(name == gPass) {
                    parsePassProperties(element, user, materialType, lightingModel);

                    if(version == 0) {
                        parsePassV0(element, user);
                    } else if(version >= 11) {
                        parsePassV11(element, user);
                    }
                }

                element = element.next_sibling();
            }

            if(version != SHADER_FORMAT_VERSION) {
                saveShaderFormat(path, shaders, user);
            }

            TString define;
            PragmaMap pragmas;
            buildInstanceData(user, pragmas);

            if(flags & Compute) {
                define += rhiDefines(rhi);
                TString str = shaders[gCompute];
                if(!str.isEmpty()) {
                    user["Shader"] = loadShader(str, define, pragmas);
                }
            } else {
                define += rhiDefines(rhi);
                define += "\n#define USE_GBUFFER";

                NativeCodeBuilder *builder = Editor::project()->currentBuilder();
                if(builder && !builder->isEmbedded()) {
                    define += "\n#define USE_SSBO";
                }

                if(lightingModel == Material::Lit) {
                    define += "\n#define USE_TBN";
                }

                TString str = shaders[gFragment];
                if(!str.isEmpty()) {
                    user[FRAGMENT] = loadShader(str, define, pragmas);
                    if(materialType == Material::Surface) {
                        user[VISIBILITY] = loadShader(str, define + "\n#define VISIBILITY_BUFFER", pragmas);
                    }
                } else {
                    user[FRAGMENT] = loadIncludes("Default.frag", define, pragmas);
                    if(materialType == Material::Surface) {
                        user[VISIBILITY] = loadIncludes("Default.frag", define + "\n#define VISIBILITY_BUFFER", pragmas);
                    }
                }

                str = shaders[gVertex];
                if(!str.isEmpty()) {
                    user[STATIC] = loadShader(str, define, pragmas);
                } else {
                    TString file = "Static.vert";
                    if(materialType == Material::PostProcess) {
                        file = "Fullscreen.vert";
                    }
                    user[STATIC] = loadIncludes(file, define, pragmas);

                    flags = materialType == Material::Surface ? (Skinned | Particle) : 0;
                    if(flags & Skinned) {
                        user[SKINNED] = loadIncludes("Skinned.vert", define, pragmas);
                    }
                    if(flags & Particle) {
                        user[PARTICLE] = loadIncludes("Billboard.vert", define, pragmas);
                    }
                }

                str = shaders[gGeometry];
                if(!str.isEmpty()) {
                    user[GEOMETRY] = loadShader(str, define, pragmas);
                }
            }
        }
    }

    return true;
}

Uniform ShaderBuilder::uniformFromVariant(const Variant &variant) {
    Uniform uniform;

    VariantList fields = variant.toList();

    auto field = fields.begin();
    Variant value = *field;
    ++field;
    uint32_t size = field->toInt();
    ++field;
    uniform.name = field->toString();

    uniform.type = value.userType();

    switch(uniform.type) {
        case MetaType::BOOLEAN: {
           uniform.typeName = "bool";
           size /= sizeof(bool);
        } break;
        case MetaType::INTEGER: {
           uniform.typeName = "int";
           size /= sizeof(int);
        } break;
        case MetaType::FLOAT: {
           uniform.typeName = "float";
           size /= sizeof(float);
        } break;
        case MetaType::VECTOR2: {
           uniform.typeName = "vec2";
           size /= sizeof(Vector2);
        } break;
        case MetaType::VECTOR3: {
           uniform.typeName = "vec3";
           size /= sizeof(Vector3);
        } break;
        case MetaType::VECTOR4: {
           uniform.typeName = "vec4";
           size /= sizeof(Vector4);
        } break;
        case MetaType::MATRIX4: {
           uniform.typeName = "mat4";
           size /= sizeof(Matrix4);
        } break;
        default: break;
    }

    uniform.count = size;

    return uniform;
}

bool ShaderBuilder::saveShaderFormat(const TString &path, const std::map<TString, TString> &shaders, const VariantMap &user) {
    pugi::xml_document xml;
    pugi::xml_node shader = xml.append_child("shader");

    shader.append_attribute("version") = SHADER_FORMAT_VERSION;

    pugi::xml_node properties = shader.append_child(gProperties);

    auto it = user.find(UNIFORMS);
    if(it != user.end()) {
         for(auto &p : it->second.toList()) {
             pugi::xml_node property = properties.append_child("property");

             Uniform uniform = uniformFromVariant(p);

             property.append_attribute("name") = uniform.name.data();
             property.append_attribute("type") = uniform.typeName.data();
             if(uniform.count > 1) {
                 property.append_attribute("count") = uniform.count;
             }
         }
    }

    it = user.find(TEXTURES);
    if(it != user.end()) {
        for(auto &p : it->second.toList()) {
            pugi::xml_node property = properties.append_child("property");

            VariantList fields = p.toList();
            auto field = fields.begin();

            TString path = field->toString();
            if(!path.isEmpty()) {
                property.append_attribute("path") = path.data();
            }
            ++field;
            property.append_attribute("name") = field->toString().data();
            ++field;
            property.append_attribute("binding") = field->toInt() - UNIFORM_BIND;
            ++field;
            int32_t flags = field->toInt();

            TString type(gTexture2D);
            if(flags & ShaderRootNode::Cube) {
                type = gTextureCubemap;
            }
            property.append_attribute("type") = type.data();

            if(flags & ShaderRootNode::Target) {
                property.append_attribute("target") = true;
            }
        }
    }

    for(auto &it : shaders) {
        pugi::xml_node code = shader.append_child(it.first.data());
        pugi::xml_node text = code.append_child(pugi::node_cdata);
        text.set_value(it.second.data());
    }

    pugi::xml_node pass = shader.append_child(gPass);

    it = user.find(PROPERTIES);
    if(it != user.end()) {
        VariantList fields = it->second.toList();
        auto field = fields.begin();

        pass.append_attribute("type") = toMaterialType(field->toInt()).data();
        ++field;
        pass.append_attribute(gTwoSided) = field->toBool();
        ++field;
        pass.append_attribute(gLightModel) = toLightModel(field->toInt()).data();
    }

    it = user.find(BLENDSTATE);
    if(it != user.end()) {
        VariantList fields = it->second.toList();
        auto field = fields.begin();

        Material::BlendState state;
        state.alphaOperation = field->toInt();
        ++field;
        state.colorOperation = field->toInt();
        ++field;
        state.destinationAlphaBlendMode = field->toInt();
        ++field;
        state.destinationColorBlendMode = field->toInt();
        ++field;
        state.sourceAlphaBlendMode = field->toInt();
        ++field;
        state.sourceColorBlendMode = field->toInt();
        ++field;
        state.enabled = field->toBool();

        saveBlendState(state, pass);
    }

    it = user.find(DEPTHSTATE);
    if(it != user.end()) {
        Material::DepthState state;

        VariantList fields = it->second.toList();
        auto field = fields.begin();
        state.compareFunction = field->toInt();
        ++field;
        state.writeEnabled = field->toBool();
        ++field;
        state.enabled = field->toBool();

        if(state.enabled) {
            saveDepthState(state, pass);
        }
    }

    it = user.find(STENCILSTATE);
    if(it != user.end()) {
        Material::StencilState state;

        VariantList fields = it->second.toList();
        auto field = fields.begin();
        state.compareFunctionBack = field->toInt();
        ++field;
        state.compareFunctionFront = field->toInt();
        ++field;
        state.failOperationBack = field->toInt();
        ++field;
        state.failOperationFront = field->toInt();
        ++field;
        state.passOperationBack = field->toInt();
        ++field;
        state.passOperationFront = field->toInt();
        ++field;
        state.zFailOperationBack = field->toInt();
        ++field;
        state.zFailOperationFront = field->toInt();
        ++field;
        state.readMask = field->toInt();
        ++field;
        state.writeMask = field->toInt();
        ++field;
        state.reference = field->toInt();
        ++field;
        state.enabled = field->toBool();

        if(state.enabled) {
            saveStencilState(state, pass);
        }
    }

    return xml.save_file(path.data(), "    ");
}

bool ShaderBuilder::parseProperties(const pugi::xml_node &parent, VariantMap &user) {
    int binding = UNIFORM_BIND;
    VariantList textures;
    VariantList uniforms;

    pugi::xml_node property = parent.first_child();
    while(property) {
        // Parse properties
        TString name = property.attribute(gName).as_string();
        TString type = property.attribute("type").as_string();
        if(name.isEmpty() || type.isEmpty()) {
            return false;
        }

        if(type.toLower() == gTexture2D || type.toLower() == gTextureCubemap) { // Texture sampler
            ++binding;

            int flags = 0;
            if(property.attribute("target").as_bool()) {
                flags |= ShaderRootNode::Target;
            }
            if(type.toLower() == gTextureCubemap) {
                flags |= ShaderRootNode::Cube;
            }

            int localBinding = binding;
            TString binding(property.attribute("binding").as_string());
            if(!binding.isEmpty()) {
                localBinding = UNIFORM_BIND + binding.toInt();
            }

            TString path((flags & ShaderRootNode::Target) ? "" : property.attribute("path").as_string());
            textures.push_back(materialTexture(path, name, localBinding, flags));
        } else { // Uniform
            uint32_t size = 0;
            uint32_t count = property.attribute("count").as_int(1);
            Variant value;
            if(type == "bool") {
                value = Variant(bool(property.attribute(gValue).as_int() != 0));
                size = sizeof(bool);
            } else if(type == "int") {
                value = Variant(property.attribute(gValue).as_int());
                size = sizeof(int);
            } else if(type == "float") {
                value = Variant(property.attribute(gValue).as_float());
                size = sizeof(float);
            } else if(type == "vec2") {
                value = Variant(Vector2(1.0f));
                size = sizeof(Vector2);
            } else if(type == "vec3") {
                value = Variant(Vector3(1.0f));
                size = sizeof(Vector3);
            } else if(type == "vec4") {
                value = Variant(Vector4(1.0f));
                size = sizeof(Vector4);
            } else if(type == "mat4") {
                value = Variant(Matrix4());
                size = sizeof(Matrix4);
            }

            uniforms.push_back(materialUniform(value, size * count, name));
        }
        property = property.next_sibling();
    }

    setMaterialResources(user, textures, uniforms);

    return true;
}

void ShaderBuilder::parsePassProperties(const pugi::xml_node &element, VariantMap &user, int &materialType, int &lightingModel) {
    materialType = toMaterialType(element.attribute("type").as_string());
    bool doubleSided = element.attribute(gTwoSided).as_bool(true);
    lightingModel = toLightModel(element.attribute(gLightModel).as_string());
    setMaterialProperties(user, materialType, doubleSided, lightingModel);
}

void ShaderBuilder::parsePassV0(const pugi::xml_node &parent, VariantMap &user) {
    static const std::map<TString, int> blend = {
        {"Opaque", OldBlendType::Opaque},
        {"Additive", OldBlendType::Additive},
        {"Translucent", OldBlendType::Translucent},
    };

    int mode = OldBlendType::Opaque;
    auto blendIt = blend.find(parent.attribute("blendMode").as_string());
    if(blendIt != blend.end()) {
        mode = blendIt->second;
    }
    Material::BlendState blendState = fromBlendMode(mode);
    setMaterialBlendState(user, blendState);

    Material::DepthState depthState;

    depthState.enabled = parent.attribute("depthTest").as_bool(true);
    depthState.writeEnabled = parent.attribute("depthWrite").as_bool(true);

    setMaterialDepthState(user, depthState);
}

void ShaderBuilder::parsePassV11(const pugi::xml_node &parent, VariantMap &user) {
    pugi::xml_node element = parent.first_child();
    while(element) {
        std::string name(element.name());
        if(name == "blend") {
            setMaterialBlendState(user, loadBlendState(element));
        } else if(name == "depth") {
            setMaterialDepthState(user, loadDepthState(element));
        } else if(name == "stencil") {
            setMaterialStencilState(user, loadStencilState(element));
        }

        element = element.next_sibling();
    }
}

uint32_t ShaderBuilder::version() {
    return SHADER_FORMAT_VERSION;
}

void ShaderBuilder::init() {
    AssetConverter::init();

    for(auto &it : suffixes()) {
        AssetConverterSettings::setDefaultIconPath(it, ":/Style/styles/dark/images/material.svg");
    }
}

TString ShaderBuilder::loadIncludes(const TString &path, const TString &define, const PragmaMap &pragmas) {
    StringList paths;
    paths.push_back(Editor::project()->contentPath() + "/");
    paths.push_back(":/shaders/");
    paths.push_back(Editor::project()->resourcePath() + "/engine/shaders/");
    paths.push_back(Editor::project()->resourcePath() + "/editor/shaders/");

    for(const TString &it : paths) {
        QFile file((it + path).data());
        if(file.open(QFile::ReadOnly | QFile::Text)) {
            TString result = loadShader(file.readAll().toStdString(), define, pragmas);
            file.close();
            return result;
        }
    }

    return TString();
}

TString ShaderBuilder::loadShader(const TString &data, const TString &define, const PragmaMap &pragmas) {
    TString output;
    StringList lines(data.split("\n"));

    static std::regex pragma("^[ ]*#[ ]*pragma[ ]+(.*)[^?]*");
    static std::regex include("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">][^?]*");

    for(auto &line : lines) {
        std::smatch matches;
        TString simplified = line.simplified();
        if(std::regex_match(simplified.toStdString(), matches, include)) {
            TString next(matches[1]);
            output += loadIncludes(next, define, pragmas) + "\n";
        } else if(std::regex_match(simplified.toStdString(), matches, pragma)) {
            if(matches[1] == "flags") {
                output += define + '\n';
            } else {
                auto it = pragmas.find(TString(matches[1]));
                if(it != pragmas.end()) {
                    output += pragmas.at(TString(matches[1])) + '\n';
                }
            }
        } else {
            output += line + "\n";
        }
    }
    return output;
}

VariantList ShaderBuilder::toVariant(Material::BlendState blendState) {
    VariantList result;
    result.push_back(blendState.alphaOperation);
    result.push_back(blendState.colorOperation);
    result.push_back(blendState.destinationAlphaBlendMode);
    result.push_back(blendState.destinationColorBlendMode);
    result.push_back(blendState.sourceAlphaBlendMode);
    result.push_back(blendState.sourceColorBlendMode);
    result.push_back(blendState.enabled);

    return result;
}

VariantList ShaderBuilder::toVariant(Material::DepthState depthState) {
    VariantList result;
    result.push_back(depthState.compareFunction);
    result.push_back(depthState.writeEnabled);
    result.push_back(depthState.enabled);

    return result;
}

VariantList ShaderBuilder::toVariant(Material::StencilState stencilState) {
    VariantList result;
    result.push_back(stencilState.compareFunctionBack);
    result.push_back(stencilState.compareFunctionFront);
    result.push_back(stencilState.failOperationBack);
    result.push_back(stencilState.failOperationFront);
    result.push_back(stencilState.passOperationBack);
    result.push_back(stencilState.passOperationFront);
    result.push_back(stencilState.zFailOperationBack);
    result.push_back(stencilState.zFailOperationFront);
    result.push_back(stencilState.readMask);
    result.push_back(stencilState.writeMask);
    result.push_back(stencilState.reference);
    result.push_back(stencilState.enabled);

    return result;
}

int32_t ShaderBuilder::toMaterialType(const TString &key) {
    static const std::map<TString, int> materialTypes = {
        {"Surface", Material::Surface},
        {"PostProcess", Material::PostProcess},
        {"LightFunction", Material::LightFunction},
    };

    auto it = materialTypes.find(key);
    if(it != materialTypes.end()) {
        return it->second;
    }

    return Material::Surface;
}

TString ShaderBuilder::toMaterialType(uint32_t key) {
    static const std::map<int, TString> materialTypes = {
        {Material::Surface, "Surface"},
        {Material::PostProcess, "PostProcess"},
        {Material::LightFunction, "LightFunction"},
    };

    auto it = materialTypes.find(key);
    if(it != materialTypes.end()) {
        return it->second;
    }

    return TString();
}

int32_t ShaderBuilder::toLightModel(const TString &key) {
    static const std::map<TString, int> lightingModels = {
        {"Unlit", Material::Unlit},
        {"Lit", Material::Lit},
        {"Subsurface", Material::Subsurface},
    };

    auto it = lightingModels.find(key);
    if(it != lightingModels.end()) {
        return it->second;
    }

    return Material::Unlit;
}

TString ShaderBuilder::toLightModel(uint32_t key) {
    static const std::map<int, TString> lightingModels = {
        {Material::Unlit, "Unlit"},
        {Material::Lit, "Lit"},
        {Material::Subsurface, "Subsurface"},
    };

    auto it = lightingModels.find(key);
    if(it != lightingModels.end()) {
        return it->second;
    }

    return TString();
}

int32_t ShaderBuilder::toBlendOp(const TString &key) {
    const std::map<TString, uint32_t> blendMode = {
        { "Add",            Material::Add },
        { "Subtract",       Material::Subtract },
        { "ReverseSubtract",Material::ReverseSubtract },
        { "Min",            Material::Min },
        { "Max",            Material::Max }
    };

    auto it = blendMode.find(key);
    if(it != blendMode.end()) {
        return it->second;
    }

    return Material::Add;
}

TString ShaderBuilder::toBlendOp(uint32_t key) {
    const std::map<uint32_t, TString> blendMode = {
        { Material::Add,            "Add" },
        { Material::Subtract,       "Subtract" },
        { Material::ReverseSubtract,"ReverseSubtract" },
        { Material::Min,            "Min" },
        { Material::Max,            "Max" }
    };

    auto it = blendMode.find(key);
    if(it != blendMode.end()) {
        return it->second;
    }

    return "Add";
}

int32_t ShaderBuilder::toBlendFactor(const TString &key) {
    const std::map<TString, uint32_t> blendFactor = {
        { "Zero",                       Material::Zero },
        { "One",                        Material::One },
        { "SourceColor",                Material::SourceColor },
        { "OneMinusSourceColor",        Material::OneMinusSourceColor },
        { "DestinationColor",           Material::DestinationColor },
        { "OneMinusDestinationColor",   Material::OneMinusDestinationColor },
        { "SourceAlpha",                Material::SourceAlpha },
        { "OneMinusSourceAlpha",        Material::OneMinusSourceAlpha },
        { "DestinationAlpha",           Material::DestinationAlpha },
        { "OneMinusDestinationAlpha",   Material::OneMinusDestinationAlpha },
        { "SourceAlphaSaturate",        Material::SourceAlphaSaturate },
        { "ConstantColor",              Material::ConstantColor },
        { "OneMinusConstantColor",      Material::OneMinusConstantColor },
        { "ConstantAlpha",              Material::ConstantAlpha },
        { "OneMinusConstantAlpha",      Material::OneMinusConstantAlpha },
    };

    auto it = blendFactor.find(key);
    if(it != blendFactor.end()) {
        return it->second;
    }

    return Material::One;
}

TString ShaderBuilder::toBlendFactor(uint32_t key) {
    const std::map<uint32_t, TString> blendFactor = {
        { Material::Zero,                       "Zero" },
        { Material::One,                        "One" },
        { Material::SourceColor,                "SourceColor" },
        { Material::OneMinusSourceColor,        "OneMinusSourceColor" },
        { Material::DestinationColor,           "DestinationColor" },
        { Material::OneMinusDestinationColor,   "OneMinusDestinationColor" },
        { Material::SourceAlpha,                "SourceAlpha" },
        { Material::OneMinusSourceAlpha,        "OneMinusSourceAlpha" },
        { Material::DestinationAlpha,           "DestinationAlpha" },
        { Material::OneMinusDestinationAlpha,   "OneMinusDestinationAlpha" },
        { Material::SourceAlphaSaturate,        "SourceAlphaSaturate" },
        { Material::ConstantColor,              "ConstantColor" },
        { Material::OneMinusConstantColor,      "OneMinusConstantColor" },
        { Material::ConstantAlpha,              "ConstantAlpha" },
        { Material::OneMinusConstantAlpha,      "OneMinusConstantAlpha" },
    };

    auto it = blendFactor.find(key);
    if(it != blendFactor.end()) {
        return it->second;
    }

    return "One";
}

int32_t ShaderBuilder::toTestFunction(const TString &key) {
    const std::map<TString, uint32_t> functions = {
        { "Never",          Material::Never },
        { "Less",           Material::Less },
        { "LessOrEqual",    Material::LessOrEqual },
        { "Greater",        Material::Greater },
        { "GreaterOrEqual", Material::GreaterOrEqual },
        { "Equal",          Material::Equal },
        { "NotEqual",       Material::NotEqual },
        { "Always",         Material::Always }
    };

    auto it = functions.find(key);
    if(it != functions.end()) {
        return it->second;
    }

    return Material::Less;
}

TString ShaderBuilder::toTestFunction(uint32_t key) {
    const std::map<uint32_t, TString> functions = {
        { Material::Never,          "Never" },
        { Material::Less,           "Less" },
        { Material::LessOrEqual,    "LessOrEqual" },
        { Material::Greater,        "Greater" },
        { Material::GreaterOrEqual, "GreaterOrEqual" },
        { Material::Equal,          "Equal" },
        { Material::NotEqual,       "NotEqual" },
        { Material::Always,         "Always" }
    };

    auto it = functions.find(key);
    if(it != functions.end()) {
        return it->second;
    }

    return "Less";
}

int32_t ShaderBuilder::toActionType(const TString &key) {
    const std::map<TString, uint32_t> actionType = {
        { "Keep",           Material::Keep },
        { "Clear",          Material::Clear },
        { "Replace",        Material::Replace },
        { "Increment",      Material::Increment },
        { "IncrementWrap",  Material::IncrementWrap },
        { "Decrement",      Material::Decrement },
        { "DecrementWrap",  Material::DecrementWrap },
        { "Invert",         Material::Invert }
    };

    auto it = actionType.find(key);
    if(it != actionType.end()) {
        return it->second;
    }

    return Material::Keep;
}

TString ShaderBuilder::toActionType(uint32_t key) {
    const std::map<uint32_t, TString> actionType = {
        { Material::Keep,           "Keep" },
        { Material::Clear,          "Clear" },
        { Material::Replace,        "Replace" },
        { Material::Increment,      "Increment" },
        { Material::IncrementWrap,  "IncrementWrap" },
        { Material::Decrement,      "Decrement" },
        { Material::DecrementWrap,  "DecrementWrap" },
        { Material::Invert,         "Invert" }
    };

    auto it = actionType.find(key);
    if(it != actionType.end()) {
        return it->second;
    }

    return "Keep";
}

Material::BlendState ShaderBuilder::fromBlendMode(uint32_t mode) {
    Material::BlendState blendState;

    switch(mode) {
        case OldBlendType::Opaque: {
            blendState.enabled = false;
            blendState.sourceColorBlendMode = Material::BlendFactor::One;
            blendState.sourceAlphaBlendMode = Material::BlendFactor::One;

            blendState.destinationColorBlendMode = Material::BlendFactor::Zero;
            blendState.destinationAlphaBlendMode = Material::BlendFactor::Zero;
        } break;
        case OldBlendType::Additive: {
            blendState.enabled = true;
            blendState.sourceColorBlendMode = Material::BlendFactor::One;
            blendState.sourceAlphaBlendMode = Material::BlendFactor::One;

            blendState.destinationColorBlendMode = Material::BlendFactor::One;
            blendState.destinationAlphaBlendMode = Material::BlendFactor::One;
        } break;
        case OldBlendType::Translucent: {
            blendState.enabled = true;
            blendState.sourceColorBlendMode = Material::BlendFactor::SourceAlpha;
            blendState.sourceAlphaBlendMode = Material::BlendFactor::SourceAlpha;

            blendState.destinationColorBlendMode = Material::BlendFactor::OneMinusSourceAlpha;
            blendState.destinationAlphaBlendMode = Material::BlendFactor::OneMinusSourceAlpha;
        } break;
        default: break;
    }

    return blendState;
}

Material::BlendState ShaderBuilder::loadBlendState(const pugi::xml_node &element) {
    Material::BlendState blendState;

    if(element) {
        blendState.enabled = true;
        if(element.attribute(gOperation)) {
            blendState.alphaOperation = toBlendOp(element.attribute(gOperation).as_string("Add"));
            blendState.colorOperation = blendState.alphaOperation;
        } else {
            blendState.alphaOperation = toBlendOp(element.attribute(gAlphaOperation).as_string("Add"));
            blendState.colorOperation = toBlendOp(element.attribute(gColorOperation).as_string("Add"));
        }

        if(element.attribute(gDestination)) {
            blendState.destinationAlphaBlendMode = toBlendFactor(element.attribute(gDestination).as_string("One"));
            blendState.destinationColorBlendMode = blendState.destinationAlphaBlendMode;
        } else {
            blendState.destinationAlphaBlendMode = toBlendFactor(element.attribute(gAlphaDestination).as_string("One"));
            blendState.destinationColorBlendMode = toBlendFactor(element.attribute(gColorDestination).as_string("One"));
        }

        if(element.attribute(gSource)) {
            blendState.sourceAlphaBlendMode = toBlendFactor(element.attribute(gSource).as_string("Zero"));
            blendState.sourceColorBlendMode = blendState.sourceAlphaBlendMode;
        } else {
            blendState.sourceAlphaBlendMode = toBlendFactor(element.attribute(gAlphaSource).as_string("Zero"));
            blendState.sourceColorBlendMode = toBlendFactor(element.attribute(gColorSource).as_string("Zero"));
        }
    }

    return blendState;
}

void ShaderBuilder::saveBlendState(const Material::BlendState &state, pugi::xml_node &parent) {
    if(state.enabled) {
        pugi::xml_node blend = parent.append_child("blend");

        if(state.colorOperation == state.alphaOperation) {
            blend.append_attribute(gOperation) = toBlendOp(state.colorOperation).data();
        } else {
            blend.append_attribute(gAlphaOperation) = toBlendOp(state.alphaOperation).data();
            blend.append_attribute(gColorOperation) = toBlendOp(state.colorOperation).data();
        }

        if(state.destinationColorBlendMode == state.destinationAlphaBlendMode) {
            blend.append_attribute(gDestination) = toBlendFactor(state.destinationColorBlendMode).data();
        } else {
            blend.append_attribute(gAlphaDestination) = toBlendFactor(state.destinationAlphaBlendMode).data();
            blend.append_attribute(gColorDestination) = toBlendFactor(state.destinationColorBlendMode).data();
        }

        if(state.sourceColorBlendMode == state.sourceAlphaBlendMode) {
            blend.append_attribute(gSource) = toBlendFactor(state.sourceColorBlendMode).data();
        } else {
            blend.append_attribute(gAlphaSource) = toBlendFactor(state.sourceAlphaBlendMode).data();
            blend.append_attribute(gColorSource) = toBlendFactor(state.sourceColorBlendMode).data();
        }
    }
}

Material::DepthState ShaderBuilder::loadDepthState(const pugi::xml_node &element) {
    Material::DepthState depthState;

    if(element) {
        depthState.enabled = element.attribute(gTest).as_bool(true);
        depthState.writeEnabled = element.attribute(gWrite).as_bool(true);
        depthState.compareFunction = toTestFunction(element.attribute(gCompare).as_string("Less"));
    }

    return depthState;
}

void ShaderBuilder::saveDepthState(const Material::DepthState &state, pugi::xml_node &parent) {
    pugi::xml_node depth = parent.append_child("depth");

    depth.append_attribute(gCompare) = toTestFunction(state.compareFunction).data();
    depth.append_attribute(gWrite) = state.writeEnabled;
    depth.append_attribute(gTest) = state.enabled;
}

Material::StencilState ShaderBuilder::loadStencilState(const pugi::xml_node &element) {
    Material::StencilState stencilState;

    if(element) {
        if(element.attribute(gCompare)) {
            stencilState.compareFunctionBack = toTestFunction(element.attribute(gCompare).as_string("Always"));
            stencilState.compareFunctionFront = stencilState.compareFunctionBack;
        } else {
            stencilState.compareFunctionBack = toTestFunction(element.attribute(gCompBack).as_string("Always"));
            stencilState.compareFunctionFront = toTestFunction(element.attribute(gCompFront).as_string("Always"));
        }

        if(element.attribute(gFail)) {
            stencilState.failOperationBack = toTestFunction(element.attribute(gFail).as_string("Keep"));
            stencilState.failOperationFront = stencilState.compareFunctionBack;
        } else {
            stencilState.failOperationBack = toActionType(element.attribute(gFailBack).as_string("Keep"));
            stencilState.failOperationFront = toActionType(element.attribute(gFailFront).as_string("Keep"));
        }

        if(element.attribute(gPass)) {
            stencilState.passOperationBack = toTestFunction(element.attribute(gPass).as_string("Keep"));
            stencilState.passOperationFront = stencilState.passOperationBack;
        } else {
            stencilState.passOperationBack = toActionType(element.attribute(gPassBack).as_string("Keep"));
            stencilState.passOperationFront = toActionType(element.attribute(gPassFront).as_string("Keep"));
        }

        if(element.attribute(gZFail)) {
            stencilState.zFailOperationBack = toTestFunction(element.attribute(gZFail).as_string("Keep"));
            stencilState.zFailOperationFront = stencilState.zFailOperationBack;
        } else {
            stencilState.zFailOperationBack = toActionType(element.attribute(gZFailBack).as_string("Keep"));
            stencilState.zFailOperationFront = toActionType(element.attribute(gZFailFront).as_string("Keep"));
        }

        stencilState.readMask = element.attribute(gReadMask).as_int(1);
        stencilState.writeMask = element.attribute(gWriteMask).as_int(1);
        stencilState.reference = element.attribute(gReference).as_int(0);
        stencilState.enabled = element.attribute(gTest).as_bool(true);
    }

    return stencilState;
}

void ShaderBuilder::saveStencilState(const Material::StencilState &state, pugi::xml_node &parent) {
    pugi::xml_node stencil = parent.append_child("stencil");

    if(state.compareFunctionBack == state.compareFunctionFront) {
        stencil.append_attribute(gCompare) = toTestFunction(state.compareFunctionBack).data();
    } else {
        stencil.append_attribute(gCompBack) = toTestFunction(state.compareFunctionBack).data();
        stencil.append_attribute(gCompFront) = toTestFunction(state.compareFunctionFront).data();
    }

    if(state.failOperationBack == state.failOperationFront) {
        stencil.append_attribute(gFail) = toTestFunction(state.failOperationBack).data();
    } else {
        stencil.append_attribute(gFailBack) = toActionType(state.failOperationBack).data();
        stencil.append_attribute(gFailFront) = toActionType(state.failOperationFront).data();
    }

    if(state.passOperationBack == state.passOperationFront) {
        stencil.append_attribute(gPass) = toTestFunction(state.passOperationBack).data();
    } else {
        stencil.append_attribute(gPassBack) = toActionType(state.passOperationBack).data();
        stencil.append_attribute(gPassFront) = toActionType(state.passOperationFront).data();
    }

    if(state.zFailOperationBack == state.zFailOperationFront) {
        stencil.append_attribute(gZFail) = toTestFunction(state.zFailOperationBack).data();
    } else {
        stencil.append_attribute(gZFailBack) = toActionType(state.zFailOperationBack).data();
        stencil.append_attribute(gZFailFront) = toActionType(state.zFailOperationFront).data();
    }

    stencil.append_attribute(gReadMask) = state.readMask;
    stencil.append_attribute(gWriteMask) = state.writeMask;
    stencil.append_attribute(gReference) = state.reference;
    stencil.append_attribute(gTest) = state.enabled;
}
