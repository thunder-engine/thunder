#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <editor/assetconverter.h>

#include "shadernodegraph.h"

#define SHADER    "Shader"
#define SIMPLE    "Simple"

#define STATIC    "Static"
#define INSTANCED "StaticInst"
#define PARTICLE  "Particle"
#define SKINNED   "Skinned"

#define TYPE       "Type"
#define BLEND      "Blend"
#define MODEL      "Model"
#define SIDE       "Side"
#define DEPTH      "Depth"
#define DEPTHWRITE "DepthWrite"
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
        Metal,
        DirectX
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
    ShaderBuilder();

    static QString loadIncludes(const QString &path, const QString &define, const PragmaMap &pragmas);

    static ShaderBuilderSettings::Rhi currentRhi();

private:
    QString templatePath() const Q_DECL_OVERRIDE;

    QStringList suffixes() const Q_DECL_OVERRIDE;
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const Q_DECL_OVERRIDE;

    Variant compile(ShaderBuilderSettings::Rhi rhi, const string &buff, int stage) const;

    bool parseShaderFormat(const QString &path, VariantMap &data, bool compute = false);

    bool parseProperties(const QDomElement &element, VariantMap &user);
    bool parsePass(const QDomElement &element, int &materialType, VariantMap &user);

    static QString loadShader(const QString &data, const QString &define, const PragmaMap &pragmas);

private:
    typedef QMap<QString, ShaderBuilderSettings::Rhi> RhiMap;

};

#endif // SHADERBUILDER_H
