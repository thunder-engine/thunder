#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <editor/assetconverter.h>

#include "shadernodegraph.h"

#include "spirvconverter.h"

#define FRAGMENT   "Default"
#define VISIBILITY "Visibility"

#define STATIC      "Static"
#define STATICINST  "StaticInst"
#define SKINNED     "Skinned"
#define SKINNEDINST "SkinnedInst"
#define PARTICLE    "Particle"
#define FULLSCREEN  "Fullscreen"

#define ATTRIBUTES "Attributes"

#define TYPE       "Type"
#define BLEND      "Blend"
#define MODEL      "Model"
#define SIDE       "Side"
#define DEPTH      "Depth"
#define DEPTHWRITE "DepthWrite"
#define WIREFRAME  "Wireframe"
#define TEXTURES   "Textures"
#define UNIFORMS   "Uniforms"
#define PROPERTIES "Properties"

class ShaderBuilderSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(Rhi CurrentRHI READ rhi WRITE setRhi DESIGNABLE true USER true)

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

    Rhi m_rhi;

};

class ShaderBuilder : public AssetConverter {
public:
    typedef map<string, string> PragmaMap;

    typedef map<QString, ShaderBuilderSettings::Rhi> RhiMap;

public:
    ShaderBuilder();

    static QString loadIncludes(const QString &path, const QString &define, const PragmaMap &pragmas);

    static ShaderBuilderSettings::Rhi currentRhi();

private:
    QString templatePath() const Q_DECL_OVERRIDE;

    QStringList suffixes() const Q_DECL_OVERRIDE;
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const Q_DECL_OVERRIDE;

    Variant compile(ShaderBuilderSettings::Rhi rhi, const string &buff, SpirVConverter::Inputs &inputs, int stage) const;

    bool parseShaderFormat(const QString &path, VariantMap &data, bool compute = false);

    bool parseProperties(const QDomElement &element, VariantMap &user);
    bool parsePass(const QDomElement &element, int &materialType, VariantMap &user);

    static QString loadShader(const QString &data, const QString &define, const PragmaMap &pragmas);

};

#endif // SHADERBUILDER_H
