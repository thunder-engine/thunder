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
    A_OBJECT(ShaderBuilderSettings, AssetConverterSettings, Editor)

    A_PROPERTIES(
        A_PROPERTYEX(Rhi, CurrentRHI, ShaderBuilderSettings::rhi, ShaderBuilderSettings::setRhi, "enum=Rhi")
    )
    A_ENUMS(
        A_ENUM(Rhi,
               A_VALUE(Invalid),
               A_VALUE(OpenGL),
               A_VALUE(Vulkan),
               A_VALUE(DirectX),
               A_VALUE(Metal))
    )

public:
    enum Rhi {
        Invalid = 0,
        OpenGL,
        Vulkan,
        DirectX,
        Metal
    };

public:
    ShaderBuilderSettings();

    int rhi() const;
    void setRhi(int rhi);

private:
    StringList typeNames() const override;

    TString defaultIconPath(const TString &) const override;

    bool isOutdated() const override;

private:
    int m_rhi;

};

struct Uniform {
    TString name;

    TString typeName;

    int count = 1;

    int type;

};

class ShaderBuilder : public AssetConverter {
public:
    typedef std::map<TString, TString> PragmaMap;

    typedef std::map<TString, ShaderBuilderSettings::Rhi> RhiMap;

public:
    ShaderBuilder();

    static uint32_t version();

    static TString loadIncludes(const TString &path, const TString &define, const PragmaMap &pragmas);

    static ShaderBuilderSettings::Rhi currentRhi();

    static void buildInstanceData(const VariantMap &user, PragmaMap &pragmas);

    static VariantList toVariant(Material::BlendState blendState);
    static VariantList toVariant(Material::DepthState depthState);
    static VariantList toVariant(Material::StencilState stencilState);

    static int32_t toMaterialType(const TString &key);
    static TString toMaterialType(uint32_t key);

    static int32_t toLightModel(const TString &key);
    static TString toLightModel(uint32_t key);

    static int32_t toBlendOp(const TString &key);
    static TString toBlendOp(uint32_t key);

    static int32_t toBlendFactor(const TString &key);
    static TString toBlendFactor(uint32_t key);

    static int32_t toTestFunction(const TString &key);
    static TString toTestFunction(uint32_t key);

    static int32_t toActionType(const TString &key);
    static TString toActionType(uint32_t key);

    static Material::BlendState fromBlendMode(uint32_t mode);

    static Material::BlendState loadBlendState(const pugi::xml_node &element);
    static void saveBlendState(const Material::BlendState &state, pugi::xml_node &parent);

    static Material::DepthState loadDepthState(const pugi::xml_node &element);
    static void saveDepthState(const Material::DepthState &state, pugi::xml_node &parent);

    static Material::StencilState loadStencilState(const pugi::xml_node &element);
    static void saveStencilState(const Material::StencilState &state, pugi::xml_node &parent);

    static void compileData(VariantMap &data);

private:
    static Uniform uniformFromVariant(const Variant &variant);

    TString templatePath() const override;

    StringList suffixes() const override;
    ReturnCode convertFile(AssetConverterSettings *) override;

    AssetConverterSettings *createSettings() override;

    Actor *createActor(const AssetConverterSettings *settings, const TString &guid) const override;

    static Variant compile(ShaderBuilderSettings::Rhi rhi, const TString &buff, VariantMap &data, EShLanguage stage);

    bool parseShaderFormat(const TString &path, VariantMap &data, int flags = false);
    bool saveShaderFormat(const TString &path, const std::map<TString, TString> &shaders, const VariantMap &user);

    bool parseProperties(const pugi::xml_node &parent, VariantMap &user);

    VariantList parsePassProperties(const pugi::xml_node &element, int &materialType, int &lightingModel);
    void parsePassV0(const pugi::xml_node &parent, VariantMap &user);
    void parsePassV11(const pugi::xml_node &parent, VariantMap &user);

    static TString loadShader(const TString &data, const TString &define, const PragmaMap &pragmas);

};

#endif // SHADERBUILDER_H
