#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <map>
#include <list>

#include <editor/assetconverter.h>

#include "shaderschememodel.h"

class QDomElement;

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
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"mtl", "shader"}; }
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const QString &guid) const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/templates/Material.mtl"; }

    Variant compile(ShaderBuilderSettings::Rhi rhi, const string &buff, int stage) const;

    bool parseShaderFormat(const QString &path, VariantMap &data);

    static QString loadShader(const QString &data, const QString &define, const PragmaMap &pragmas);

private:
    typedef QMap<QString, ShaderBuilderSettings::Rhi> RhiMap;

    ShaderSchemeModel m_schemeModel;

};

#endif // SHADERBUILDER_H
