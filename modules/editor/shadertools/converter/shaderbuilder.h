#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <map>
#include <list>

#include <editor/assetconverter.h>

#include "shaderschememodel.h"

class QDomElement;

class ShaderBuilder : public AssetConverter {
public:
    enum Rhi {
        OpenGL = 1,
        Vulkan,
        Metal,
        DirectX
    };

public:
    ShaderBuilder();

    static QString loadIncludes(const QString &path, const string &define, const PragmaMap &pragmas);

private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"mtl", "shader"}; }
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const QString &guid) const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Material.mtl"; }

    Variant compile(int32_t rhi, const string &buff, int stage) const;

    bool parseShaderFormat(const QString &path, VariantMap &data);

    static QString loadShader(const QString &data, const string &define, const PragmaMap &pragmas);

private:
    typedef QMap<QString, Rhi> RhiMap;

    ShaderSchemeModel m_schemeModel;

    RhiMap m_rhiMap;

};

#endif // SHADERBUILDER_H
