#ifndef SHADERSCHEMEMODEL_H
#define SHADERSCHEMEMODEL_H

#include <QFileInfo>

#include <resources/material.h>

#include "editor/scheme/abstractschememodel.h"

#define SHADER      "Shader"
#define SIMPLE      "Simple"

#define STATIC      "Static"
#define INSTANCED   "StaticInst"
#define PARTICLE    "Particle"
#define SKINNED     "Skinned"

#define UNIFORM 4

#define TYPE        "Type"
#define BLEND       "Blend"
#define MODEL       "Model"
#define SIDE        "Side"
#define DEPTH       "Depth"
#define DEPTHWRITE  "DepthWrite"
#define RAW         "Raw"
#define TEXTURES    "Textures"
#define UNIFORMS    "Uniforms"
#define PROPERTIES  "Properties"

typedef map<string, string> PragmaMap;

class ShaderSchemeModel : public AbstractSchemeModel {
    Q_OBJECT

    Q_PROPERTY(Type Material_Type READ materialType WRITE setMaterialType DESIGNABLE true USER true)
    Q_PROPERTY(Blend Blending_Mode READ blend WRITE setBlend DESIGNABLE true USER true)
    Q_PROPERTY(LightModel Lighting_Model READ lightModel WRITE setLightModel DESIGNABLE true USER true)
    Q_PROPERTY(bool Two_Sided READ isDoubleSided WRITE setDoubleSided DESIGNABLE true USER true)
    Q_PROPERTY(bool Depth_Test READ isDepthTest WRITE setDepthTest DESIGNABLE true USER true)
    Q_PROPERTY(QFileInfo Raw_Path READ rawPath WRITE setRawPath DESIGNABLE true USER true)

public:
    enum LightModel {
        Unlit      = Material::Unlit,
        Lit        = Material::Lit,
        Subsurface = Material::Subsurface
    };

    enum Blend {
        Opaque      = Material::Opaque,
        Additive    = Material::Additive,
        Translucent = Material::Translucent
    };

    enum Type {
        Surface       = Material::Surface,
        PostProcess   = Material::PostProcess,
        LightFunction = Material::LightFunction
    };

    enum Flags {
        Cube   = (1<<0),
        Target = (1<<1)
    };

    Q_ENUM(Type)
    Q_ENUM(LightModel)
    Q_ENUM(Blend)

public:
    ShaderSchemeModel();
    ~ShaderSchemeModel() Q_DECL_OVERRIDE;

    VariantMap data(bool editor = false) const;

    bool buildGraph();

    bool isDoubleSided() const { return m_DoubleSided; }
    void setDoubleSided(bool value) { m_DoubleSided = value; emit schemeUpdated(); }

    bool isDepthTest() const { return m_DepthTest; }
    void setDepthTest(bool value) { m_DepthTest = value; emit schemeUpdated(); }

    bool isDepthWrite() const { return m_DepthWrite; }
    void setDepthWrite(bool value) { m_DepthWrite = value; emit schemeUpdated(); }

    Type materialType() const { return m_MaterialType; }
    void setMaterialType(Type type) { m_MaterialType = type; }

    Blend blend() const { return m_BlendMode; }
    void setBlend(Blend mode) { m_BlendMode = mode; emit schemeUpdated(); }

    LightModel lightModel() const { return m_LightModel; }
    void setLightModel(LightModel model) { m_LightModel = model; emit schemeUpdated(); }

    int setTexture(const QString &path, Vector4 &sub, uint8_t flags = 0);

    void addUniform(const QString &name, uint8_t type, const QVariant &value);

    QFileInfo rawPath() const { return m_RawPath; }
    void setRawPath(const QFileInfo &path) { m_RawPath = path; }

    QStringList nodeList() const Q_DECL_OVERRIDE;

    void load(const QString &path) Q_DECL_OVERRIDE;
    void save(const QString &path) Q_DECL_OVERRIDE;

private:
    struct MaterialInput {
        MaterialInput(QString name, QVariant value, bool vertex = false) :
            m_name(name),
            m_value(value),
            m_vertex(vertex) {

        }

        QString m_name;

        QVariant m_value;

        bool m_vertex;
    };

    void loadUserValues(Node *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(Node *node, QVariantMap &values) Q_DECL_OVERRIDE;

    void loadTextures(const QVariantMap &data);
    QVariantMap saveTextures() const;

    void loadUniforms(const QVariantList &data);
    QVariantList saveUniforms() const;

    Node *nodeCreate(const QString &path, int &index) Q_DECL_OVERRIDE;

    Variant compile(int32_t rhi, const QString &source, const string &define, int stage) const;

    QStringList buildRoot();

    void cleanup();

    void addPragma(const string &key, const string &value);

private:
    struct Uniform {
        QString name;

        uint32_t type;

        size_t count;

        QVariant value;
    };

    typedef QList<Uniform> UniformList;

    typedef QPair<QString, uint8_t> TexturePair;

    typedef QList<TexturePair> TextureList;

    typedef QList<MaterialInput> InputList;

    QStringList m_Functions;

    UniformList m_Uniforms;

    TextureList m_Textures;

    PragmaMap m_Pragmas;

    InputList m_Inputs;

    Blend m_BlendMode;

    LightModel m_LightModel;

    Type m_MaterialType;

    bool m_DoubleSided;

    bool m_DepthTest;

    bool m_DepthWrite;

    bool m_ViewSpace;

    QFileInfo m_RawPath;

};

Q_DECLARE_METATYPE(ShaderSchemeModel::LightModel)
Q_DECLARE_METATYPE(ShaderSchemeModel::Blend)
Q_DECLARE_METATYPE(ShaderSchemeModel::Type)

#endif // SHADERSCHEMEMODEL_H
