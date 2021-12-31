#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <map>
#include <list>

#include <resources/material.h>

#include <QFileInfo>

#include "editor/scheme/abstractschememodel.h"
#include "editor/assetconverter.h"

class Log;
class ShaderSchemeModel;

static const char *a("A");
static const char *b("B");
static const char *x("X");
static const char *y("Y");

class ShaderFunction : public QObject {
    Q_OBJECT

public:
    ShaderFunction() :
            m_pModel(nullptr),
            m_pNode(nullptr) {
        reset();
    }

    void reset() {
        m_Position  = -1;
    }

    virtual AbstractSchemeModel::Node *createNode(ShaderSchemeModel *model, const QString &path) {
        m_pNode = new AbstractSchemeModel::Node;
        m_pNode->root = false;
        m_pNode->name = path;
        m_pNode->ptr = this;
        m_pNode->pos = QPoint();

        m_pModel = model;

        return m_pNode;
    }

    virtual int32_t build(QString &value, const AbstractSchemeModel::Link &link, int32_t &depth, uint8_t &size) {
        Q_UNUSED(value)
        Q_UNUSED(link)
        Q_UNUSED(size)

        if(m_Position == -1) {
            m_Position  = depth;
            depth++;
        }
        return m_Position;
    }

    static QString convert(const QString &value, uint8_t current, uint32_t target, uint8_t component = 0) {
        QString prefix;
        QString suffix;

        const char *names[] = {".x", ".y", ".z", ".w"};

        switch(target) {
            case QMetaType::QVector2D: {
                switch(current) {
                    case QMetaType::Double:     { prefix = "vec2("; suffix = ")"; } break;
                    case QMetaType::QVector3D:
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xy"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector3D: {
                switch(current) {
                    case QMetaType::Double:     { prefix = "vec3("; suffix = ")"; } break;
                    case QMetaType::QVector2D:  { prefix = "vec3("; suffix = ", 0.0)"; } break;
                    case QMetaType::QVector4D:  { prefix = ""; suffix = ".xyz"; } break;
                    default: break;
                }
            } break;
            case QMetaType::QVector4D: {
                switch(current) {
                    case QMetaType::Double:     { prefix = "vec4("; suffix = ")"; } break;
                    case QMetaType::QVector2D:  { prefix = "vec4("; suffix = ", 0.0, 1.0)"; } break;
                    case QMetaType::QVector3D:  { prefix = "vec4("; suffix = ", 1.0)"; } break;
                    default: break;
                }
            } break;
            default: {
                switch(current) {
                    case QMetaType::QVector2D:
                    case QMetaType::QVector3D:
                    case QMetaType::QVector4D:  { prefix = ""; suffix = names[component]; } break;
                    default: break;
                }
            } break;
        }
        return(prefix + value + suffix);
    }

signals:
    void updated();

protected:
    ShaderSchemeModel *m_pModel;
    AbstractSchemeModel::Node *m_pNode;

    int32_t m_Position;

};

class ShaderSchemeModel : public AbstractSchemeModel {
    Q_OBJECT

    Q_PROPERTY(Type Material_Type READ materialType WRITE setMaterialType DESIGNABLE true USER true)
    Q_PROPERTY(Blend Blending_Mode READ blend WRITE setBlend DESIGNABLE true USER true)
    Q_PROPERTY(LightModel Lighting_Model READ lightModel WRITE setLightModel DESIGNABLE true USER true)
    Q_PROPERTY(bool Two_Sided READ isDoubleSided WRITE setDoubleSided DESIGNABLE true USER true)
    Q_PROPERTY(bool Depth_Test READ isDepthTest WRITE setDepthTest DESIGNABLE true USER true)
    Q_PROPERTY(bool View_Space READ isViewSpace WRITE setViewSpace DESIGNABLE true USER true)
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
        Cube    =(1<<0),
        Target  =(1<<1)
    };

    enum Rhi {
        OpenGL = 1,
        Vulkan,
        Metal,
        DirectX
    };

    Q_ENUM(Type)
    Q_ENUM(LightModel)
    Q_ENUM(Blend)

public:
    ShaderSchemeModel();
    ~ShaderSchemeModel() Q_DECL_OVERRIDE;

    Variant object() const;

    Variant data(bool editor = false) const;

    bool build();

    QString shader() const { return m_Shader; }

    bool isDoubleSided() const { return m_DoubleSided; }
    void setDoubleSided(bool value) { m_DoubleSided = value; emit schemeUpdated(); }

    bool isDepthTest() const { return m_DepthTest; }
    void setDepthTest(bool value) { m_DepthTest = value; emit schemeUpdated(); }

    bool isDepthWrite() const { return m_DepthWrite; }
    void setDepthWrite(bool value) { m_DepthWrite = value; emit schemeUpdated(); }

    bool isViewSpace() const { return m_ViewSpace; }
    void setViewSpace(bool value) { m_ViewSpace = value; emit schemeUpdated(); }

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
    void loadUserValues(Node *node, const QVariantMap &values) Q_DECL_OVERRIDE;
    void saveUserValues(Node *node, QVariantMap &values) Q_DECL_OVERRIDE;

    void loadTextures(const QVariantMap &data);
    QVariantMap saveTextures() const;

    Node *nodeCreate(const QString &path, int &index) Q_DECL_OVERRIDE;

    Variant compile(int32_t rhi, const QString &source, const string &define, int stage) const;

    void addParam(const QString &param);

    void buildRoot(QString &result);

    void cleanup();

    void addPragma(const string &key, const string &value);
    QString loadIncludes(const QString &path, const string &define) const;

    typedef QPair<uint8_t, QVariant> UniformPair;

    typedef map<QString, UniformPair> UniformMap;

    typedef QPair<QString, uint8_t> TexturePair;

    typedef QList<TexturePair> TextureList;

    QStringList                 m_Functions;

    /// Shader uniforms
    UniformMap                  m_Uniforms;
    /// Shader uniforms
    TextureList                 m_Textures;
    /// Shader params
    QString                     m_Params;
    /// Shader source code
    QString                     m_Shader;

    Blend                       m_BlendMode;

    LightModel                  m_LightModel;

    Type                        m_MaterialType;

    bool                        m_DoubleSided;

    bool                        m_DepthTest;

    bool                        m_DepthWrite;

    bool                        m_ViewSpace;

    typedef map<string, string> PragmaMap;

    PragmaMap                   m_Pragmas;

    QFileInfo                   m_RawPath;
};

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

Q_DECLARE_METATYPE(ShaderSchemeModel::LightModel)
Q_DECLARE_METATYPE(ShaderSchemeModel::Blend)
Q_DECLARE_METATYPE(ShaderSchemeModel::Type)

#endif // SHADERBUILDER_H
