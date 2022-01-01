#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <map>
#include <list>

#include <editor/assetconverter.h>

#include "shaderschememodel.h"

class Log;

class ShaderBuilder : public AssetConverter {
private:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"mtl"}; }
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;

    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const QString &guid) const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/Templates/Material.mtl"; }

private:
    ShaderSchemeModel m_schemeModel;

};

#endif // SHADERBUILDER_H
