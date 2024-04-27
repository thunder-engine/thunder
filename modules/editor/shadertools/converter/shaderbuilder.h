#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <editor/assetconverter.h>

#include "shadernodegraph.h"

#include "spirvconverter.h"

#define FRAGMENT   "Default"
#define VISIBILITY "Visibility"

#define STATIC      "Static"
#define SKINNED     "Skinned"
#define PARTICLE    "Particle"

#define GEOMETRY    "Geometry"

#define ATTRIBUTES "Attributes"

#define TEXTURES   "Textures"
#define UNIFORMS   "Uniforms"
#define PROPERTIES "Properties"

#define BLENDSTATE "BlendState"
#define DEPTHSTATE "DepthState"
#define STENCILSTATE "StencilState"

class ShaderBuilderSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(Rhi CurrentRHI READ rhi WRITE setRhi NOTIFY updated DESIGNABLE true USER true)

public:
    enum class Rhi {
        Invalid = 0,
        OpenGL,
        Vulkan,
        DirectX,
        Metal
    };
    Q_ENUM(Rhi)

public:
    ShaderBuilderSettings();

    Rhi rhi() const;
    void setRhi(Rhi rhi);

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

    bool isOutdated() const Q_DECL_OVERRIDE;

private:
    Rhi m_rhi;

};

struct Uniform {
    string name;

    string typeName;

    int count = 1;

    int type;

};

class ShaderBuilder : public AssetConverter {
public:
    typedef map<string, string> PragmaMap;

    typedef map<string, ShaderBuilderSettings::Rhi> RhiMap;

public:
    ShaderBuilder();

    static uint32_t version();

    static string loadIncludes(const string &path, const string &define, const PragmaMap &pragmas);

    static ShaderBuilderSettings::Rhi currentRhi();

    static void buildInstanceData(const VariantMap &user, PragmaMap &pragmas);

    static VariantList toVariant(Material::BlendState blendState);
    static VariantList toVariant(Material::DepthState depthState);
    static VariantList toVariant(Material::StencilState stencilState);

    static uint32_t toBlendOp(const string &key);
    static string toBlendOp(uint32_t key);

    static uint32_t toBlendFactor(const string &key);
    static string toBlendFactor(uint32_t key);

    static uint32_t toTestFunction(const string &key);
    static string toTestFunction(uint32_t key);

    static uint32_t toActionType(const string &key);
    static string toActionType(uint32_t key);

    static Material::BlendState fromBlendMode(uint32_t mode);

    static Material::BlendState loadBlendState(const QDomElement &element);
    static void saveBlendState(const Material::BlendState &state, QDomDocument &document, QDomElement &parent);

    static Material::DepthState loadDepthState(const QDomElement &element);
    static void saveDepthState(const Material::DepthState &state, QDomDocument &document, QDomElement &parent);

    static Material::StencilState loadStencilState(const QDomElement &element);
    static void saveStencilState(const Material::StencilState &state, QDomDocument &document, QDomElement &parent);

    static void compileData(VariantMap &data);

private:
    static Uniform uniformFromVariant(const Variant &variant);

    QString templatePath() const Q_DECL_OVERRIDE;

    QStringList suffixes() const Q_DECL_OVERRIDE;
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const Q_DECL_OVERRIDE;

    static Variant compile(ShaderBuilderSettings::Rhi rhi, const string &buff, SpirVConverter::Inputs &inputs, int stage);

    bool parseShaderFormat(const QString &path, VariantMap &data, bool compute = false);
    bool saveShaderFormat(const QString &path, const map<string, string> &shaders, const VariantMap &user);

    bool parseProperties(const QDomElement &element, VariantMap &user);

    VariantList parsePassProperties(const QDomElement &element, int &materialType, int &lightingModel);
    void parsePassV0(const QDomElement &element, VariantMap &user);
    void parsePassV11(const QDomElement &element, VariantMap &user);

    static string loadShader(const string &data, const string &define, const PragmaMap &pragmas);

};

#endif // SHADERBUILDER_H
