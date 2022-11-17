#ifndef SHADERNODEGRAPH_H
#define SHADERNODEGRAPH_H

#include <QFileInfo>

#include <resources/material.h>

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

typedef map<string, string> PragmaMap;

class ShaderRootNode : public GraphNode {
    Q_OBJECT

    Q_PROPERTY(Type Material_Type READ materialType WRITE setMaterialType DESIGNABLE true USER true)
    Q_PROPERTY(Blend Blending_Mode READ blend WRITE setBlend DESIGNABLE true USER true)
    Q_PROPERTY(LightModel Lighting_Model READ lightModel WRITE setLightModel DESIGNABLE true USER true)
    Q_PROPERTY(bool Two_Sided READ isDoubleSided WRITE setDoubleSided DESIGNABLE true USER true)
    Q_PROPERTY(bool Depth_Test READ isDepthTest WRITE setDepthTest DESIGNABLE true USER true)
    Q_PROPERTY(bool Depth_Write READ isDepthWrite WRITE setDepthWrite DESIGNABLE true USER true)

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

    ShaderRootNode() :
        m_blendMode(Opaque),
        m_lightModel(Lit),
        m_materialType(Surface),
        m_doubleSided(false),
        m_depthTest(true),
        m_depthWrite(true),
        m_viewSpace(true) {

    }

    bool isDoubleSided() const { return m_doubleSided; }
    void setDoubleSided(bool value) { m_doubleSided = value; emit graphUpdated(); }

    bool isDepthTest() const { return m_depthTest; }
    void setDepthTest(bool value) { m_depthTest = value; emit graphUpdated(); }

    bool isDepthWrite() const { return m_depthWrite; }
    void setDepthWrite(bool value) { m_depthWrite = value; emit graphUpdated(); }

    Type materialType() const { return m_materialType; }
    void setMaterialType(Type type) { m_materialType = type; }

    Blend blend() const { return m_blendMode; }
    void setBlend(Blend mode) { m_blendMode = mode; emit graphUpdated(); }

    LightModel lightModel() const { return m_lightModel; }
    void setLightModel(LightModel model) { m_lightModel = model; emit graphUpdated(); }

    Vector4 color() const override { return Vector4(0.141f, 0.384f, 0.514f, 1.0f); }

signals:
    void graphUpdated();

private:
    Blend m_blendMode;

    LightModel m_lightModel;

    Type m_materialType;

    bool m_doubleSided;

    bool m_depthTest;

    bool m_depthWrite;

    bool m_viewSpace;

};

Q_DECLARE_METATYPE(ShaderRootNode::LightModel)
Q_DECLARE_METATYPE(ShaderRootNode::Blend)
Q_DECLARE_METATYPE(ShaderRootNode::Type)

class ShaderNodeGraph : public AbstractNodeGraph {
    Q_OBJECT

public:
    ShaderNodeGraph();
    ~ShaderNodeGraph() Q_DECL_OVERRIDE;

    VariantMap data(bool editor = false) const;

    bool buildGraph();

    QStringList buildFrom(GraphNode *node);

    int setTexture(const QString &path, Vector4 &sub, int32_t flags = 0);

    void addUniform(const QString &name, uint8_t type, const QVariant &value);

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

    void loadUserValues(GraphNode *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(GraphNode *node, QVariantMap &values) Q_DECL_OVERRIDE;

    void loadTextures(const QVariantMap &data);
    QVariantMap saveTextures() const;

    void loadUniforms(const QVariantList &data);
    QVariantList saveUniforms() const;

    GraphNode *nodeCreate(const QString &path, int &index) Q_DECL_OVERRIDE;
    GraphNode *createRoot() Q_DECL_OVERRIDE;

    Variant compile(int32_t rhi, const QString &source, const string &define, int stage) const;

    void cleanup();

    void addPragma(const string &key, const string &value);

private:
    struct Uniform {
        QString name;

        uint32_t type;

        size_t count;

        QVariant value;
    };

    QStringList m_functions;

    list<Uniform> m_uniforms;

    QList<QPair<QString, int32_t>> m_textures;

    PragmaMap m_pragmas;

    list<MaterialInput> m_inputs;

};

#endif // SHADERNODEGRAPH_H
