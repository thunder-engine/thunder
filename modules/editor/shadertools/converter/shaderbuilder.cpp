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

#include <editor/projectmanager.h>

#include "../../config.h"

#include <regex>

#define FORMAT_VERSION 8

namespace  {
    const char *gValue("value");
    const char *gName("name");

    const char *gFragment("Fragment");
    const char *gVertex("Vertex");
    const char *gCompute("Compute");

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
}

QString ShaderBuilderSettings::defaultIcon(QString) const {
    return ":/Style/styles/dark/images/material.svg";
}

bool ShaderBuilderSettings::isOutdated() const {
    bool result = AssetConverterSettings::isOutdated();
    result |= (m_rhi != ShaderBuilder::currentRhi());
    return result;
}

AssetConverterSettings *ShaderBuilder::createSettings() const {
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
            rhi = rhiMap.value(qEnvironmentVariable(qPrintable(gRhi)));
        } else {
            rhi = ShaderBuilderSettings::Rhi::OpenGL;
        }
    }

    return rhi;
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
        ShaderNodeGraph nodeGraph;
        nodeGraph.load(builderSettings->source());
        if(nodeGraph.buildGraph()) {
            if(builderSettings->currentVersion() != builderSettings->version()) {
                nodeGraph.save(builderSettings->source());
            }
            data = nodeGraph.data();
        }
    } else if(info.suffix() == "shader") {
        parseShaderFormat(builderSettings->source(), data);
    } else if(info.suffix() == "compute") {
        parseShaderFormat(builderSettings->source(), data, true);
    }

    if(data.empty()) {
        return InternalError;
    }

    uint32_t version = 430;
    bool es = false;

    ShaderBuilderSettings::Rhi rhi = currentRhi();

    if(ProjectManager::instance()->currentPlatformName() != "desktop") {
        version = 300;
        es = true;
    }
    SpirVConverter::setGlslVersion(version, es);

    SpirVConverter::Inputs inputs;
    data[FRAGMENT] = compile(rhi, data[FRAGMENT].toString(), inputs, EShLangFragment);
    {
        auto it = data.find(VISIBILITY);
        if(it != data.end()) {
            data[VISIBILITY] = compile(rhi, it->second.toString(), inputs, EShLangFragment);
        }
    }

    data[STATIC] = compile(rhi, data[STATIC].toString(), inputs, EShLangVertex);

    std::sort(inputs.begin(), inputs.end(), [](const SpirVConverter::Input &left, const SpirVConverter::Input &right) { return left.location < right.location; });

    VariantList attributes;
    for(auto &it : inputs) {
        VariantList attribute;
        attribute.push_back(it.format);
        attribute.push_back(it.location);

        attributes.push_back(attribute);
    }
    data[ATTRIBUTES] = attributes;

    auto it = data.find(STATICINST);
    if(it != data.end()) {
        data[STATICINST] = compile(rhi, it->second.toString(), inputs, EShLangVertex);
    }

    it = data.find(PARTICLE);
    if(it != data.end()) {
        data[PARTICLE] = compile(rhi, it->second.toString(), inputs, EShLangVertex);
    }

    it = data.find(SKINNED);
    if(it != data.end()) {
        data[SKINNED] = compile(rhi, it->second.toString(), inputs, EShLangVertex);
    }

    VariantList result;

    VariantList object;

    object.push_back(Material::metaClass()->name()); // type
    object.push_back(0); // id
    object.push_back(0); // parent
    object.push_back(builderSettings->destination().toStdString()); // name

    object.push_back(VariantMap()); // properties

    object.push_back(VariantList()); // links
    object.push_back(data); // user data

    result.push_back(object);

    QFile file(builderSettings->absoluteDestination());
    if(file.open(QIODevice::WriteOnly)) {
        ByteArray data = Bson::save(result);
        file.write(reinterpret_cast<const char *>(&data[0]), data.size());
        file.close();
        builderSettings->setCurrentVersion(builderSettings->version());
        builderSettings->setRhi(rhi);
        return Success;
    }

    return InternalError;
}

Variant ShaderBuilder::compile(ShaderBuilderSettings::Rhi rhi, const string &buff, SpirVConverter::Inputs &inputs, int stage) const {
    inputs.clear();

    Variant data;
    vector<uint32_t> spv = SpirVConverter::glslToSpv(buff, static_cast<EShLanguage>(stage), inputs);
    if(!spv.empty()) {
        switch(rhi) {
            case ShaderBuilderSettings::Rhi::OpenGL: data = SpirVConverter::spvToGlsl(spv); break;
            case ShaderBuilderSettings::Rhi::Metal: data = SpirVConverter::spvToMetal(spv); break;
            case ShaderBuilderSettings::Rhi::DirectX: data = SpirVConverter::spvToHlsl(spv); break;
            default: {
                ByteArray array;
                array.resize(spv.size() * sizeof(uint32_t));
                memcpy(&array[0], &spv[0], array.size());
                data = array;
                break;
            }
        }
    }
    return data;
}

bool ShaderBuilder::parseShaderFormat(const QString &path, VariantMap &user, bool compute) {
    QFile file(path);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QDomDocument doc;
        if(doc.setContent(&file)) {
            QMap<QString, QString> shaders;

            int materialType = Material::Surface;

            QDomElement shader = doc.documentElement();
            QDomNode n = shader.firstChild();
            while(!n.isNull()) {
                QDomElement element = n.toElement();
                if(!element.isNull()) {
                    if(element.tagName() == gFragment || element.tagName() == gVertex || element.tagName() == gCompute) {
                        shaders[element.tagName()] = element.text();
                    } else if(element.tagName() == "Properties") {
                        if(!parseProperties(element, user)) {
                            return false;
                        }
                    } else if(element.tagName() == "Pass") {
                        if(!parsePass(element, materialType, user)) {
                            return false;
                        }
                    }
                }
                n = n.nextSibling();
            }

            QString define;
            const PragmaMap pragmas;

            if(compute) {
                QString str = shaders.value(gCompute);
                if(!str.isEmpty()) {
                    user[FRAGMENT] = loadShader(str, define, pragmas).toStdString();
                }
            } else {
                if(materialType == Material::PostProcess) {
                    define += "\n#define TYPE_FULLSCREEN\n";
                }

                if(currentRhi() == ShaderBuilderSettings::Rhi::Vulkan) {
                    define += "\n#define VULKAN\n";
                }

                QString str;
                str = shaders.value(gFragment);
                if(!str.isEmpty()) {
                    user[FRAGMENT] = loadShader(str, define, pragmas).toStdString();
                } else {
                    user[FRAGMENT] = loadIncludes("Default.frag", define, pragmas).toStdString();
                }

                str = shaders.value(gVertex);
                if(!str.isEmpty()) {
                    user[STATIC] = loadShader(shaders.value(gVertex), define, pragmas).toStdString();
                } else {
                    user[STATIC] = loadIncludes("Default.vert", define, pragmas).toStdString();
                }
            }
        }
        file.close();
    }

    return false;
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
                texture.push_back((flags & ShaderRootNode::Target) ? "" : property.attribute("path").toStdString()); // path
                texture.push_back(localBinding); // binding
                texture.push_back(name.toStdString()); // name
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
                data.push_back("uni." + name.toStdString());

                uniforms.push_back(data);
            }
        }
        p = p.nextSibling();
    }

    user[TEXTURES] = textures;
    user[UNIFORMS] = uniforms;

    return true;
}

bool ShaderBuilder::parsePass(const QDomElement &element, int &materialType, VariantMap &user) {
    VariantList properties;

    static const QMap<QString, int> types = {
        {"Surface", Material::Surface},
        {"PostProcess", Material::PostProcess},
        {"LightFunction", Material::LightFunction},
    };

    static const QMap<QString, int> blend = {
        {"Opaque", Material::Opaque},
        {"Additive", Material::Additive},
        {"Translucent", Material::Translucent},
    };

    static const QMap<QString, int> light = {
        {"Unlit", Material::Unlit},
        {"Lit", Material::Lit},
        {"Subsurface", Material::Subsurface},
    };

    materialType = types.value(element.attribute("type"), Material::Surface);
    properties.push_back(materialType);
    properties.push_back(element.attribute("twoSided", "true") == "true");
    properties.push_back((materialType != ShaderRootNode::Surface) ? Material::Static :
                         (Material::Static | Material::Skinned | Material::Billboard | Material::Oriented));
    properties.push_back(blend.value(element.attribute("blendMode"), Material::Opaque));
    properties.push_back(light.value(element.attribute("lightModel"), Material::Unlit));
    properties.push_back(element.attribute("depthTest", "true") == "true");
    properties.push_back(element.attribute("depthWrite", "true") == "true");
    properties.push_back(element.attribute("wireFrame", "false") == "true");

    user[PROPERTIES] = properties;

    return true;
}

QString ShaderBuilder::loadIncludes(const QString &path, const QString &define, const PragmaMap &pragmas) {
    QStringList paths;
    paths << ProjectManager::instance()->contentPath() + "/";
    paths << ":/shaders/";
    paths << ProjectManager::instance()->resourcePath() + "/engine/shaders/";
    paths << ProjectManager::instance()->resourcePath() + "/editor/shaders/";

    foreach(QString it, paths) {
        QFile file(it + path);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString result = loadShader(file.readAll(), define, pragmas);
            file.close();
            return result;
        }
    }

    return QString();
}

QString ShaderBuilder::loadShader(const QString &data, const QString &define, const PragmaMap &pragmas) {
    QString output;
    QStringList lines(data.split("\n"));

    static regex pragma("^[ ]*#[ ]*pragma[ ]+(.*)[^?]*");
    static regex include("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">][^?]*");

    for(auto &line : lines) {
        smatch matches;
        string data = line.simplified().toStdString();
        if(regex_match(data, matches, include)) {
            string next(matches[1]);
            output += loadIncludes(next.c_str(), define, pragmas) + "\n";
        } else if(regex_match(data, matches, pragma)) {
            if(matches[1] == "flags") {
                output += define + "\n";
            } else {
                auto it = pragmas.find(matches[1]);
                if(it != pragmas.end()) {
                    output += QString(pragmas.at(matches[1]).c_str()) + "\n";
                }
            }
        } else {
            output += line + "\n";
        }
    }
    return output;
}
