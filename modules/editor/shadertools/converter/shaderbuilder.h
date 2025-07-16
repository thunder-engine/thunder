#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <editor/assetconverter.h>

#include "shadergraph.h"

#include "spirvconverter.h"

#define FRAGMENT   "Default"
#define VISIBILITY "Visibility"

#define STATIC      "Static"
#define SKINNED     "Skinned"
#define PARTICLE    "Particle"

#define GEOMETRY    "Geometry"

#define TEXTURES   "Textures"
#define UNIFORMS   "Uniforms"
#define PROPERTIES "Properties"

#define BLENDSTATE "BlendState"
#define DEPTHSTATE "DepthState"
#define STENCILSTATE "StencilState"

class ShaderBuilderSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(ShaderBuilderSettings::Rhi CurrentRHI READ rhi WRITE setRhi NOTIFY updated DESIGNABLE true USER true)

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
    QString defaultIconPath(const QString &) const override;

    bool isOutdated() const override;

private:
    Rhi m_rhi;

};

struct Uniform {
    String name;

    String typeName;

    int count = 1;

    int type;

};

class ShaderBuilder : public AssetConverter {
public:
    typedef std::map<String, String> PragmaMap;

    typedef std::map<String, ShaderBuilderSettings::Rhi> RhiMap;

public:
    ShaderBuilder();

    static uint32_t version();

    static String loadIncludes(const String &path, const String &define, const PragmaMap &pragmas);

    static ShaderBuilderSettings::Rhi currentRhi();

    static void buildInstanceData(const VariantMap &user, PragmaMap &pragmas);

    static VariantList toVariant(Material::BlendState blendState);
    static VariantList toVariant(Material::DepthState depthState);
    static VariantList toVariant(Material::StencilState stencilState);

    static int32_t toMaterialType(const String &key);
    static String toMaterialType(uint32_t key);

    static int32_t toLightModel(const String &key);
    static String toLightModel(uint32_t key);

    static int32_t toBlendOp(const String &key);
    static String toBlendOp(uint32_t key);

    static int32_t toBlendFactor(const String &key);
    static String toBlendFactor(uint32_t key);

    static int32_t toTestFunction(const String &key);
    static String toTestFunction(uint32_t key);

    static int32_t toActionType(const String &key);
    static String toActionType(uint32_t key);

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

    QString templatePath() const override;

    QStringList suffixes() const override;
    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const override;

    static Variant compile(ShaderBuilderSettings::Rhi rhi, const String &buff, VariantMap &data, EShLanguage stage);

    bool parseShaderFormat(const QString &path, VariantMap &data, int flags = false);
    bool saveShaderFormat(const QString &path, const std::map<String, String> &shaders, const VariantMap &user);

    bool parseProperties(const QDomElement &element, VariantMap &user);

    VariantList parsePassProperties(const QDomElement &element, int &materialType, int &lightingModel);
    void parsePassV0(const QDomElement &element, VariantMap &user);
    void parsePassV11(const QDomElement &element, VariantMap &user);

    static String loadShader(const String &data, const String &define, const PragmaMap &pragmas);

};

#endif // SHADERBUILDER_H
