#include "shaderbuilder.h"

#include <QDomDocument>

#include <resources/texture.h>

#include <file.h>
#include <log.h>
#include <bson.h>

#include <components/actor.h>
#include <components/meshrender.h>

#include <resources/mesh.h>
#include <resources/material.h>

#include <commandbuffer.h>

#include <editor/projectsettings.h>

#include "../../config.h"

#include <regex>

#define FORMAT_VERSION 14

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
    const char *gWireFrame("wireFrame");

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
};

ShaderBuilderSettings::ShaderBuilderSettings() {
    setType(MetaType::type<Material *>());
    setVersion(FORMAT_VERSION);
    setRhi(Rhi::OpenGL);
}

ShaderBuilderSettings::Rhi ShaderBuilderSettings::rhi() const {
    return m_rhi;
}
void ShaderBuilderSettings::setRhi(Rhi rhi) {
    m_rhi = rhi;

    emit updated();
}

QString ShaderBuilderSettings::defaultIconPath(const QString &) const {
    return ":/Style/styles/dark/images/material.svg";
}

bool ShaderBuilderSettings::isOutdated() const {
    bool result = AssetConverterSettings::isOutdated();
    result |= (m_rhi != ShaderBuilder::currentRhi());
    return result;
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
        {"RenderDX", ShaderBuilderSettings::Rhi::DirectX},
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

void ShaderBuilder::buildInstanceData(const VariantMap &user, PragmaMap &pragmas) {
    String result;

    String modelMatrix =
    "    mat4 modelMatrix = mat4(vec4(instance.data[_instanceOffset + 0].xyz, 0.0f),\n"
    "                            vec4(instance.data[_instanceOffset + 1].xyz, 0.0f),\n"
    "                            vec4(instance.data[_instanceOffset + 2].xyz, 0.0f),\n"
    "                            vec4(instance.data[_instanceOffset + 3].xyz, 1.0f));\n";

    String objectId =
    "    _objectId = vec4(instance.data[_instanceOffset + 0].w,\n"
    "                     instance.data[_instanceOffset + 1].w,\n"
    "                     instance.data[_instanceOffset + 2].w,\n"
    "                     instance.data[_instanceOffset + 3].w);";

    int offset = 4;

    String uniforms;
    auto it = user.find(UNIFORMS);
    if(it != user.end()) {
        int sub = 0;
        const char *compNames = "xyzw";
        for(auto &p : it->second.toList()) {
            Uniform uniform = uniformFromVariant(p);

            String comp;

            if(uniform.type == MetaType::MATRIX4) {
                if(sub > 0) {
                    sub = 0;
                    offset++;
                }

            } else {
                String prefix;

                switch(uniform.type) {
                    case MetaType::BOOLEAN: {
                        prefix = "floatBitsToInt(";
                        comp = String(&compNames[sub]);
                        comp += ") != 0";
                        sub++;
                    } break;
                    case MetaType::INTEGER: {
                        prefix = "floatBitsToInt(";
                        comp = String(&compNames[sub]);
                        comp += ")";
                        sub++;
                    } break;
                    case MetaType::FLOAT: {
                        comp = String(&compNames[sub]);
                        sub++;
                    } break;
                    case MetaType::VECTOR2: {
                        if(sub > 2) {
                            sub = 0;
                            offset++;
                        }

                        comp = String(&compNames[sub]);
                        sub++;
                        comp += compNames[sub];
                        sub++;
                    } break;
                    case MetaType::VECTOR3: {
                        if(sub > 1) {
                            sub = 0;
                            offset++;
                        }

                        comp = String(&compNames[sub]);
                        sub++;
                        comp += compNames[sub];
                        sub++;
                        comp += compNames[sub];
                        sub++;
                    } break;
                    case MetaType::VECTOR4: {
                        comp = compNames;
                        sub = 4;
                    } break;
                    default: break;
                }

                String data = prefix + "instance.data[_instanceOffset + " + std::to_string(offset) + "]." + comp + ";\n";

                if(sub >= 4) {
                    sub = 0;
                    offset++;
                }

                uniforms += uniform.typeName + " " + uniform.name + " = " + data;
            }
        }

        if(sub > 0) {
            offset++;
        }
    }

    pragmas["offset"] = "_instanceOffset = gl_InstanceIndex * " + std::to_string(offset) + ";\n";
    result += modelMatrix;
    result += uniforms;

    pragmas["instance"] = result;
    pragmas["objectId"] = objectId;
}

Actor *ShaderBuilder::createActor(const AssetConverterSettings *settings, const QString &guid) const {
    Actor *object = Engine::composeActor("MeshRender", "");

    MeshRender *mesh = static_cast<MeshRender *>(object->component("MeshRender"));
    if(mesh) {
        Mesh *m = Engine::loadResource<Mesh>(".embedded/sphere.fbx/Sphere001");
        if(m) {
            mesh->setMesh(m);
            Material *mat = Engine::loadResource<Material>(guid.toStdString());
            if(mat) {
                mesh->setMaterial(mat);
            }
        }
    }

    return object;
}

QString ShaderBuilder::templatePath() const {
    return ":/templates/Material.mtl";
}

QStringList ShaderBuilder::suffixes() const {
    return {"mtl", "shader", "compute"};
}

AssetConverter::ReturnCode ShaderBuilder::convertFile(AssetConverterSettings *settings) {
    VariantMap data;

    ShaderBuilderSettings *builderSettings = static_cast<ShaderBuilderSettings *>(settings);

    QFileInfo info(builderSettings->source());
    if(info.suffix() == "mtl") {
        ShaderGraph nodeGraph;
        nodeGraph.load(builderSettings->source().toStdString());
        if(nodeGraph.buildGraph()) {
            if(builderSettings->currentVersion() != builderSettings->version()) {
                nodeGraph.save(builderSettings->source().toStdString());
            }
            data = nodeGraph.data();
        }
    } else if(info.suffix() == "shader") {
        parseShaderFormat(builderSettings->source(), data);
    } else if(info.suffix() == "compute") {
        parseShaderFormat(builderSettings->source(), data, Compute);
    }

    if(data.empty()) {
        return InternalError;
    }

    compileData(data);

    VariantList object;

    object.push_back(Material::metaClass()->name()); // type
    object.push_back(Engine::generateUUID()); // id
    object.push_back(0); // parent
    object.push_back(String(builderSettings->destination().toStdString())); // name

    object.push_back(VariantMap()); // properties

    object.push_back(VariantList()); // links
    object.push_back(data); // user data

    VariantList result;
    result.push_back(object);

    QFile file(builderSettings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save(result);
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();

        builderSettings->setRhi(currentRhi());

        return Success;
    }

    return InternalError;
}

void ShaderBuilder::compileData(VariantMap &data) {
    ShaderBuilderSettings::Rhi rhi = currentRhi();

    uint32_t version = 430;
    bool es = false;

    if(ProjectSettings::instance()->currentPlatformName() != "desktop") {
        version = 300;
        es = true;
    }
    SpirVConverter::setGlslVersion(version, es);

    data[FRAGMENT] = compile(rhi, data[FRAGMENT].toString(), data, EShLangFragment);
    {
        auto it = data.find(VISIBILITY);
        if(it != data.end()) {
            data[VISIBILITY] = compile(rhi, it->second.toString(), data, EShLangFragment);
        }
    }

    data[STATIC] = compile(rhi, data[STATIC].toString(), data, EShLangVertex);

    auto it = data.find(PARTICLE);
    if(it != data.end()) {
        data[PARTICLE] = compile(rhi, it->second.toString(), data, EShLangVertex);
    }

    it = data.find(SKINNED);
    if(it != data.end()) {
        data[SKINNED] = compile(rhi, it->second.toString(), data, EShLangVertex);
    }

    it = data.find(GEOMETRY);
    if(it != data.end()) {
        data[GEOMETRY] = compile(rhi, it->second.toString(), data, EShLangGeometry);
    }
}

Variant ShaderBuilder::compile(ShaderBuilderSettings::Rhi rhi, const String &buff, VariantMap &data, EShLanguage stage) {
    SpirVConverter::Inputs inputs;

    VariantList result;
    std::vector<uint32_t> spv = SpirVConverter::glslToSpv(buff, inputs, static_cast<EShLanguage>(stage));
    if(!spv.empty()) {
        switch(rhi) {
            case ShaderBuilderSettings::Rhi::OpenGL: result.push_back(Variant(SpirVConverter::spvToGlsl(spv))); break;
            case ShaderBuilderSettings::Rhi::Metal: result.push_back(Variant(SpirVConverter::spvToMetal(spv, inputs, stage))); break;
            case ShaderBuilderSettings::Rhi::DirectX: result.push_back(Variant(SpirVConverter::spvToHlsl(spv))); break;
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
                    String name = field->toString();
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

bool ShaderBuilder::parseShaderFormat(const QString &path, VariantMap &user, int flags) {
    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = file.readAll();
        file.close();

        QDomDocument doc;
        if(doc.setContent(data)) {
            std::map<String, String> shaders;

            int materialType = Material::Surface;
            int lightingModel = Material::Unlit;

            QDomElement shader = doc.documentElement();

            uint32_t version = shader.attribute("version", "0").toInt();

            QDomNode n = shader.firstChild();
            while(!n.isNull()) {
                QDomElement element = n.toElement();
                if(!element.isNull()) {
                    if(element.tagName() == gFragment || element.tagName() == gVertex || element.tagName() == gGeometry) {
                        shaders[element.tagName().toStdString()] = element.text().toStdString();
                    } else if(element.tagName() == gCompute) {
                        shaders[element.tagName().toStdString()] = element.text().toStdString();
                    } else if(element.tagName() == gProperties) {
                        if(!parseProperties(element, user)) {
                            return false;
                        }
                    } else if(element.tagName() == gPass) {
                        user[PROPERTIES] = parsePassProperties(element, materialType, lightingModel);

                        if(version == 0) {
                            parsePassV0(element, user);
                        } else if(version >= 11) {
                            parsePassV11(element, user);
                        }
                    }
                }
                n = n.nextSibling();
            }

            if(version != FORMAT_VERSION) {
                saveShaderFormat(path, shaders, user);
            }

            String define;
            PragmaMap pragmas;
            buildInstanceData(user, pragmas);

            if(flags & Compute) {
                String str = shaders[gCompute];
                if(!str.isEmpty()) {
                    user["Shader"] = loadShader(str, define, pragmas);
                }
            } else {
                if(currentRhi() == ShaderBuilderSettings::Rhi::Vulkan) {
                    define += "\n#define VULKAN";
                }

                if(currentRhi() == ShaderBuilderSettings::Rhi::Metal) {
                    define += "\n#define ORIGIN_TOP";
                }

                define += "\n#define USE_GBUFFER";

                if(materialType == Material::Surface && ProjectSettings::instance()->currentPlatformName() == "desktop") {
                    define += "\n#define USE_SSBO";
                }

                if(lightingModel == Material::Lit) {
                    define += "\n#define USE_TBN";
                }

                String str;
                str = shaders[gFragment];
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

                if(materialType == Material::LightFunction) {
                    define += "\n#define NO_INSTANCE";
                }

                str = shaders[gVertex];
                if(!str.isEmpty()) {
                    user[STATIC] = loadShader(str, define, pragmas);
                } else {
                    String file = "Static.vert";
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

bool ShaderBuilder::saveShaderFormat(const QString &path, const std::map<String, String> &shaders, const VariantMap &user) {
    QDomDocument xml;
    QDomElement shader = xml.createElement("shader");

    shader.setAttribute("version", FORMAT_VERSION);

    QDomElement properties(xml.createElement(gProperties));

    auto it = user.find(UNIFORMS);
    if(it != user.end()) {
         for(auto &p : it->second.toList()) {
             QDomElement property(xml.createElement("property"));

             Uniform uniform = uniformFromVariant(p);

             property.setAttribute("name", uniform.name.data());
             property.setAttribute("type", uniform.typeName.data());
             if(uniform.count > 1) {
                 property.setAttribute("count", uniform.count);
             }

             properties.appendChild(property);
         }
    }

    it = user.find(TEXTURES);
    if(it != user.end()) {
        for(auto &p : it->second.toList()) {
            QDomElement property(xml.createElement("property"));

            VariantList fields = p.toList();
            auto field = fields.begin();

            String path = field->toString();
            if(!path.isEmpty()) {
                property.setAttribute("path", path.data());
            }
            ++field;
            property.setAttribute("name", field->toString().data());
            ++field;
            property.setAttribute("binding", field->toInt() - UNIFORM_BIND);
            ++field;
            int32_t flags = field->toInt();

            String type(gTexture2D);
            if(flags & ShaderRootNode::Cube) {
                type = gTextureCubemap;
            }
            property.setAttribute("type", type.data());

            if(flags & ShaderRootNode::Target) {
                property.setAttribute("target", "true");
            }

            properties.appendChild(property);
        }
    }

    shader.appendChild(properties);

    for(auto &it : shaders) {
        QDomElement code(xml.createElement(it.first.data()));
        QDomText text(xml.createCDATASection(it.second.data()));
        code.appendChild(text);

        shader.appendChild(code);
    }

    QDomElement pass(xml.createElement(gPass));

    it = user.find(PROPERTIES);
    if(it != user.end()) {
        VariantList fields = it->second.toList();
        auto field = fields.begin();

        pass.setAttribute("type", toMaterialType(field->toInt()).data());
        ++field;
        pass.setAttribute(gTwoSided, field->toBool() ? "true" : "false");
        ++field;
        pass.setAttribute(gLightModel, toLightModel(field->toInt()).data());
        ++field;
        pass.setAttribute(gWireFrame, field->toBool() ? "true" : "false");
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

        saveBlendState(state, xml, pass);
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
            saveDepthState(state, xml, pass);
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
            saveStencilState(state, xml, pass);
        }
    }

    shader.appendChild(pass);

    xml.appendChild(shader);

    QFile saveFile(path);
    if(saveFile.open(QIODevice::WriteOnly)) {
        saveFile.write(xml.toByteArray(4));
        saveFile.close();
    }

    return true;
}

bool ShaderBuilder::parseProperties(const QDomElement &element, VariantMap &user) {
    int binding = UNIFORM_BIND;
    VariantList textures;
    VariantList uniforms;

    QDomNode p = element.firstChild();
    while(!p.isNull()) {
        QDomElement property = p.toElement();
        if(!property.isNull()) {
            // Parse properties
            QString name = property.attribute(gName);
            QString type = property.attribute("type");
            if(name.isEmpty() || type.isEmpty()) {
                return false;
            }

            if(type.toLower() == gTexture2D || type.toLower() == gTextureCubemap) { // Texture sampler
                ++binding;

                int flags = 0;
                if(property.attribute("target", "false") == "true") {
                    flags |= ShaderRootNode::Target;
                }
                if(type.toLower() == gTextureCubemap) {
                    flags |= ShaderRootNode::Cube;
                }

                int localBinding = binding;
                QString b = property.attribute("binding");
                if(!b.isEmpty()) {
                    localBinding = UNIFORM_BIND + b.toInt();
                }

                VariantList texture;
                texture.push_back((flags & ShaderRootNode::Target) ? "" : String(property.attribute("path").toStdString())); // path
                texture.push_back(String(name.toStdString())); // name
                texture.push_back(localBinding); // binding
                texture.push_back(flags); // flags

                textures.push_back(texture);
            } else { // Uniform
                VariantList data;

                uint32_t size = 0;
                uint32_t count = property.attribute("count", "1").toInt();
                Variant value;
                if(type == "bool") {
                    value = Variant(bool(property.attribute(gValue).toInt() != 0));
                    size = sizeof(bool);
                } else if(type == "int") {
                    value = Variant(property.attribute(gValue).toInt());
                    size = sizeof(int);
                } else if(type == "float") {
                    value = Variant(property.attribute(gValue).toFloat());
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

                data.push_back(value);
                data.push_back(size * count);
                data.push_back(String(name.toStdString()));

                uniforms.push_back(data);
            }
        }
        p = p.nextSibling();
    }

    user[TEXTURES] = textures;
    user[UNIFORMS] = uniforms;

    return true;
}

VariantList ShaderBuilder::parsePassProperties(const QDomElement &element, int &materialType, int &lightingModel) {
    VariantList properties;
    materialType = toMaterialType(element.attribute("type").toStdString());
    properties.push_back(materialType);
    properties.push_back(element.attribute(gTwoSided, "true") == "true");
    lightingModel = toLightModel(element.attribute(gLightModel).toStdString());
    properties.push_back(lightingModel);
    properties.push_back(element.attribute(gWireFrame, "false") == "true");

    return properties;
}

void ShaderBuilder::parsePassV0(const QDomElement &element, VariantMap &user) {
    static const QMap<QString, int> blend = {
        {"Opaque", OldBlendType::Opaque},
        {"Additive", OldBlendType::Additive},
        {"Translucent", OldBlendType::Translucent},
    };

    Material::BlendState blendState = fromBlendMode(blend.value(element.attribute("blendMode"), OldBlendType::Opaque));
    user[BLENDSTATE] = toVariant(blendState);

    Material::DepthState depthState;

    depthState.enabled = (element.attribute("depthTest", "true") == "true");
    depthState.writeEnabled = (element.attribute("depthWrite", "true") == "true");

    user[DEPTHSTATE] = toVariant(depthState);
}

void ShaderBuilder::parsePassV11(const QDomElement &element, VariantMap &user) {
    QDomNode p = element.firstChild();
    while(!p.isNull()) {
        QDomElement element = p.toElement();
        if(!element.isNull()) {
            if(element.tagName() == "blend") {
                user[BLENDSTATE] = toVariant(loadBlendState(element));
            } else if(element.tagName() == "depth") {
                user[DEPTHSTATE] = toVariant(loadDepthState(element));
            } else if(element.tagName() == "stencil") {
                user[STENCILSTATE] = toVariant(loadStencilState(element));
            }
        }

        p = p.nextSiblingElement();
    }
}

uint32_t ShaderBuilder::version() {
    return FORMAT_VERSION;
}

String ShaderBuilder::loadIncludes(const String &path, const String &define, const PragmaMap &pragmas) {
    QStringList paths;
    paths << ProjectSettings::instance()->contentPath() + "/";
    paths << ":/shaders/";
    paths << ProjectSettings::instance()->resourcePath() + "/engine/shaders/";
    paths << ProjectSettings::instance()->resourcePath() + "/editor/shaders/";

    foreach(QString it, paths) {
        QFile file(it + path.data());
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            String result = loadShader(file.readAll().toStdString(), define, pragmas);
            file.close();
            return result;
        }
    }

    return String();
}

String ShaderBuilder::loadShader(const String &data, const String &define, const PragmaMap &pragmas) {
    String output;
    QStringList lines(QString(data.data()).split("\n"));

    static std::regex pragma("^[ ]*#[ ]*pragma[ ]+(.*)[^?]*");
    static std::regex include("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">][^?]*");

    for(auto &line : lines) {
        std::smatch matches;
        String data = line.simplified().toStdString();
        if(std::regex_match(data.toStdString(), matches, include)) {
            String next(matches[1]);
            output += loadIncludes(next.data(), define, pragmas) + "\n";
        } else if(std::regex_match(data.toStdString(), matches, pragma)) {
            if(matches[1] == "flags") {
                output += define + '\n';
            } else {
                auto it = pragmas.find(String(matches[1]));
                if(it != pragmas.end()) {
                    output += pragmas.at(String(matches[1])) + '\n';
                }
            }
        } else {
            output += line.toStdString() + "\n";
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

int32_t ShaderBuilder::toMaterialType(const String &key) {
    static const std::map<String, int> materialTypes = {
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

String ShaderBuilder::toMaterialType(uint32_t key) {
    static const std::map<int, String> materialTypes = {
        {Material::Surface, "Surface"},
        {Material::PostProcess, "PostProcess"},
        {Material::LightFunction, "LightFunction"},
    };

    auto it = materialTypes.find(key);
    if(it != materialTypes.end()) {
        return it->second;
    }

    return String();
}

int32_t ShaderBuilder::toLightModel(const String &key) {
    static const std::map<String, int> lightingModels = {
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

String ShaderBuilder::toLightModel(uint32_t key) {
    static const std::map<int, String> lightingModels = {
        {Material::Unlit, "Unlit"},
        {Material::Lit, "Lit"},
        {Material::Subsurface, "Subsurface"},
    };

    auto it = lightingModels.find(key);
    if(it != lightingModels.end()) {
        return it->second;
    }

    return String();
}

int32_t ShaderBuilder::toBlendOp(const String &key) {
    const std::map<String, uint32_t> blendMode = {
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

String ShaderBuilder::toBlendOp(uint32_t key) {
    const std::map<uint32_t, String> blendMode = {
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

int32_t ShaderBuilder::toBlendFactor(const String &key) {
    const std::map<String, uint32_t> blendFactor = {
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

String ShaderBuilder::toBlendFactor(uint32_t key) {
    const std::map<uint32_t, String> blendFactor = {
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

int32_t ShaderBuilder::toTestFunction(const String &key) {
    const std::map<String, uint32_t> functions = {
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

String ShaderBuilder::toTestFunction(uint32_t key) {
    const std::map<uint32_t, String> functions = {
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

int32_t ShaderBuilder::toActionType(const String &key) {
    const std::map<String, uint32_t> actionType = {
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

String ShaderBuilder::toActionType(uint32_t key) {
    const std::map<uint32_t, String> actionType = {
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

Material::BlendState ShaderBuilder::loadBlendState(const QDomElement &element) {
    Material::BlendState blendState;

    if(!element.isNull()) {
        blendState.enabled = true;
        if(element.hasAttribute(gOperation)) {
            blendState.alphaOperation = toBlendOp(element.attribute(gOperation, "Add").toStdString());
            blendState.colorOperation = blendState.alphaOperation;
        } else {
            blendState.alphaOperation = toBlendOp(element.attribute(gAlphaOperation, "Add").toStdString());
            blendState.colorOperation = toBlendOp(element.attribute(gColorOperation, "Add").toStdString());
        }

        if(element.hasAttribute(gDestination)) {
            blendState.destinationAlphaBlendMode = toBlendFactor(element.attribute(gDestination, "One").toStdString());
            blendState.destinationColorBlendMode = blendState.destinationAlphaBlendMode;
        } else {
            blendState.destinationAlphaBlendMode = toBlendFactor(element.attribute(gAlphaDestination, "One").toStdString());
            blendState.destinationColorBlendMode = toBlendFactor(element.attribute(gColorDestination, "One").toStdString());
        }

        if(element.hasAttribute(gSource)) {
            blendState.sourceAlphaBlendMode = toBlendFactor(element.attribute(gSource, "Zero").toStdString());
            blendState.sourceColorBlendMode = blendState.sourceAlphaBlendMode;
        } else {
            blendState.sourceAlphaBlendMode = toBlendFactor(element.attribute(gAlphaSource, "Zero").toStdString());
            blendState.sourceColorBlendMode = toBlendFactor(element.attribute(gColorSource, "Zero").toStdString());
        }
    }

    return blendState;
}

void ShaderBuilder::saveBlendState(const Material::BlendState &state, QDomDocument &document, QDomElement &parent) {
    if(state.enabled) {
        QDomElement blend(document.createElement("blend"));

        if(state.colorOperation == state.alphaOperation) {
            blend.setAttribute(gOperation, toBlendOp(state.colorOperation).data());
        } else {
            blend.setAttribute(gAlphaOperation, toBlendOp(state.alphaOperation).data());
            blend.setAttribute(gColorOperation, toBlendOp(state.colorOperation).data());
        }

        if(state.destinationColorBlendMode == state.destinationAlphaBlendMode) {
            blend.setAttribute(gDestination, toBlendFactor(state.destinationColorBlendMode).data());
        } else {
            blend.setAttribute(gAlphaDestination, toBlendFactor(state.destinationAlphaBlendMode).data());
            blend.setAttribute(gColorDestination, toBlendFactor(state.destinationColorBlendMode).data());
        }

        if(state.sourceColorBlendMode == state.sourceAlphaBlendMode) {
            blend.setAttribute(gSource, toBlendFactor(state.sourceColorBlendMode).data());
        } else {
            blend.setAttribute(gAlphaSource, toBlendFactor(state.sourceAlphaBlendMode).data());
            blend.setAttribute(gColorSource, toBlendFactor(state.sourceColorBlendMode).data());
        }

        parent.appendChild(blend);
    }
}

Material::DepthState ShaderBuilder::loadDepthState(const QDomElement &element) {
    Material::DepthState depthState;

    if(!element.isNull()) {
        depthState.enabled = (element.attribute(gTest, "true") == "true");
        depthState.writeEnabled = (element.attribute(gWrite, "true") == "true");
        depthState.compareFunction = toTestFunction(element.attribute(gCompare, "Less").toStdString());
    }

    return depthState;
}

void ShaderBuilder::saveDepthState(const Material::DepthState &state, QDomDocument &document, QDomElement &parent) {
    QDomElement depth(document.createElement("depth"));

    depth.setAttribute(gCompare, toTestFunction(state.compareFunction).data());
    depth.setAttribute(gWrite, state.writeEnabled ? "true" : "false");
    depth.setAttribute(gTest, state.enabled ? "true" : "false");

    parent.appendChild(depth);
}

Material::StencilState ShaderBuilder::loadStencilState(const QDomElement &element) {
    Material::StencilState stencilState;

    if(!element.isNull()) {
        if(element.hasAttribute(gCompare)) {
            stencilState.compareFunctionBack = toTestFunction(element.attribute(gCompare, "Always").toStdString());
            stencilState.compareFunctionFront = stencilState.compareFunctionBack;
        } else {
            stencilState.compareFunctionBack = toTestFunction(element.attribute(gCompBack, "Always").toStdString());
            stencilState.compareFunctionFront = toTestFunction(element.attribute(gCompFront, "Always").toStdString());
        }

        if(element.hasAttribute(gFail)) {
            stencilState.failOperationBack = toTestFunction(element.attribute(gFail, "Keep").toStdString());
            stencilState.failOperationFront = stencilState.compareFunctionBack;
        } else {
            stencilState.failOperationBack = toActionType(element.attribute(gFailBack, "Keep").toStdString());
            stencilState.failOperationFront = toActionType(element.attribute(gFailFront, "Keep").toStdString());
        }

        if(element.hasAttribute(gPass)) {
            stencilState.passOperationBack = toTestFunction(element.attribute(gPass, "Keep").toStdString());
            stencilState.passOperationFront = stencilState.passOperationBack;
        } else {
            stencilState.passOperationBack = toActionType(element.attribute(gPassBack, "Keep").toStdString());
            stencilState.passOperationFront = toActionType(element.attribute(gPassFront, "Keep").toStdString());
        }

        if(element.hasAttribute(gPass)) {
            stencilState.zFailOperationBack = toTestFunction(element.attribute(gZFail, "Keep").toStdString());
            stencilState.zFailOperationFront = stencilState.zFailOperationBack;
        } else {
            stencilState.zFailOperationBack = toActionType(element.attribute(gZFailBack, "Keep").toStdString());
            stencilState.zFailOperationFront = toActionType(element.attribute(gZFailFront, "Keep").toStdString());
        }

        stencilState.readMask = element.attribute(gReadMask, "1").toInt();
        stencilState.writeMask = element.attribute(gWriteMask, "1").toInt();
        stencilState.reference = element.attribute(gReference, "0").toInt();
        stencilState.enabled = (element.attribute(gTest, "true") == "true");
    }

    return stencilState;
}

void ShaderBuilder::saveStencilState(const Material::StencilState &state, QDomDocument &document, QDomElement &parent) {
    QDomElement stencil(document.createElement("stencil"));

    if(state.compareFunctionBack == state.compareFunctionFront) {
        stencil.setAttribute(gCompare, toTestFunction(state.compareFunctionBack).data());
    } else {
        stencil.setAttribute(gCompBack, toTestFunction(state.compareFunctionBack).data());
        stencil.setAttribute(gCompFront, toTestFunction(state.compareFunctionFront).data());
    }

    if(state.failOperationBack == state.failOperationFront) {
        stencil.setAttribute(gFail, toTestFunction(state.failOperationBack).data());
    } else {
        stencil.setAttribute(gFailBack, toActionType(state.failOperationBack).data());
        stencil.setAttribute(gFailFront, toActionType(state.failOperationFront).data());
    }

    if(state.passOperationBack == state.passOperationFront) {
        stencil.setAttribute(gPass, toTestFunction(state.passOperationBack).data());
    } else {
        stencil.setAttribute(gPassBack, toActionType(state.passOperationBack).data());
        stencil.setAttribute(gPassFront, toActionType(state.passOperationFront).data());
    }

    if(state.zFailOperationBack == state.zFailOperationFront) {
        stencil.setAttribute(gZFail, toTestFunction(state.zFailOperationBack).data());
    } else {
        stencil.setAttribute(gZFailBack, toActionType(state.zFailOperationBack).data());
        stencil.setAttribute(gZFailFront, toActionType(state.zFailOperationFront).data());
    }

    stencil.setAttribute(gReadMask, state.readMask);
    stencil.setAttribute(gWriteMask, state.writeMask);
    stencil.setAttribute(gReference, state.reference);
    stencil.setAttribute(gTest, state.enabled ? "true" : "false");

    parent.appendChild(stencil);
}
