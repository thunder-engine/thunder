#ifndef SHADERBUILDER_H
#define SHADERBUILDER_H

#include <map>
#include <list>

#include <resources/material.h>
#include <module.h>

#include <QFileInfo>

#include "abstractschememodel.h"

class Log;
class ShaderBuilder;

class ShaderFunction : public QObject {
    Q_OBJECT

public:
    ShaderFunction      () { reset(); }

    void reset() {
        m_Position  = -1;
    }

    virtual AbstractSchemeModel::Node  *createNode  (ShaderBuilder *model, const QString &path) {
        m_pNode     = new AbstractSchemeModel::Node;
        m_pNode->root   = false;
        m_pNode->name   = path;
        m_pNode->ptr    = this;
        m_pNode->pos    = QPoint();

        m_pModel    = model;

        return m_pNode;
    }

    virtual uint32_t                    build       (QString &value, const AbstractSchemeModel::Link &link, uint32_t &depth, uint8_t &size) {
        Q_UNUSED(value);
        Q_UNUSED(link);
        Q_UNUSED(size);

        if(m_Position == -1) {
            m_Position  = depth;
            depth++;
        }
        return m_Position;
    }

    static QString                      convert     (const QString &value, uint8_t current, uint8_t target, uint8_t component = 0) {
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
        return (prefix + value + suffix);
    }

signals:
    void                        updated     ();

protected:
    ShaderBuilder              *m_pModel;
    AbstractSchemeModel::Node  *m_pNode;

    int32_t                     m_Position;

};

class ShaderBuilder : public AbstractSchemeModel {
    Q_OBJECT

    Q_PROPERTY(Type Material_Type READ materialType WRITE setMaterialType DESIGNABLE true USER true)
    Q_ENUMS(Type)
    Q_PROPERTY(Blend Blending_Mode READ blend WRITE setBlend DESIGNABLE true USER true)
    Q_ENUMS(Blend)
    Q_PROPERTY(LightModel Lighting_Model READ lightModel WRITE setLightModel DESIGNABLE true USER true)
    Q_ENUMS(LightModel)
    Q_PROPERTY(bool Two_Sided READ isDoubleSided WRITE setDoubleSided DESIGNABLE true USER true)
    Q_PROPERTY(bool Depth_Test READ isDepthTest WRITE setDepthTest DESIGNABLE true USER true)
    Q_PROPERTY(bool View_Space READ isViewSpace WRITE setViewSpace DESIGNABLE true USER true)
    Q_PROPERTY(QFileInfo Raw_Path READ rawPath WRITE setRawPath DESIGNABLE true USER true)

public:
    enum LightModel {
        Unlit       = Material::Unlit,
        Lit         = Material::Lit,
        Subsurface  = Material::Subsurface
    };

    enum Blend {
        Opaque      = Material::Opaque,
        Additive    = Material::Additive,
        Translucent = Material::Translucent
    };

    enum Type {
        Surface         = Material::Surface,
        PostProcess     = Material::PostProcess,
        LightFunction   = Material::LightFunction
    };

    enum Flags {
        Cube    = (1<<0),
        Target  = (1<<1)
    };

public:
    ShaderBuilder               ();
    ~ShaderBuilder              ();

    Node                       *createNode                  (const QString &path);
    void                        deleteNode                  (Node *node);

    void                        createLink                  (Node *sender, Item *sitem, Node *receiver, Item *ritem);
    void                        deleteLink                  (Item *item, bool silent = false);

    const AbstractSchemeModel::Link    *findLink            (const AbstractSchemeModel::Node *node, const char *item) {
        for(const auto it : m_Links) {
            QString str;
            str.compare(item);
            if(it->receiver == node && it->ritem->name.compare(item) == 0) {
                return it;
            }
        }
        return nullptr;
    }

    QAbstractItemModel         *components                  () const;

    void                        load                        (const QString &path);
    void                        save                        (const QString &path);

    Variant                     object                      () const;

    Variant                     data                        () const;

    bool                        build                       ();

    QString                     shader                      () const { return m_Shader; }

    bool                        isDoubleSided               () const { return m_DoubleSided; }
    void                        setDoubleSided              (bool value) { m_DoubleSided = value; emit schemeUpdated(); }

    bool                        isDepthTest                 () const { return m_DepthTest; }
    void                        setDepthTest                (bool value) { m_DepthTest = value; emit schemeUpdated(); }

    bool                        isViewSpace                 () const { return m_ViewSpace; }
    void                        setViewSpace                (bool value) { m_ViewSpace = value; emit schemeUpdated(); }

    Type                        materialType                () const { return m_MaterialType; }
    void                        setMaterialType             (Type type) { m_MaterialType = type; }

    Blend                       blend                       () const { return m_BlendMode; }
    void                        setBlend                    (Blend mode) { m_BlendMode = mode; emit schemeUpdated(); }

    LightModel                  lightModel                  () const { return m_LightModel; }
    void                        setLightModel               (LightModel model) { m_LightModel = model; emit schemeUpdated(); }

    int                         setTexture                  (const QString &path, Vector4 &sub, uint8_t flags = 0);

    void                        addUniform                  (const QString &name, uint8_t type);

    void                        reportError                 (QObject *, const QString &) { }

    QFileInfo                   rawPath                     () const { return m_RawPath; }
    void                        setRawPath                  (const QFileInfo &path) { m_RawPath = path; }

private:
    bool                        build                       (QString &, const AbstractSchemeModel::Link &, uint32_t &, uint8_t &) {return true;}

    void                        addParam                    (const QString &param);

    void                        buildRoot                   (QString &result);

    void                        cleanup                     ();

    void                        addPragma                   (const string &key, const string &value);
    QString                     loadIncludes                (const QString &path, const string &define) const;

    typedef map<QString, uint8_t>   UniformMap;

    typedef QPair<QString, uint8_t> TexturePair;

    typedef QList<TexturePair>      TextureList;

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

    bool                        m_ViewSpace;

    AbstractSchemeModel::Node  *m_pNode;

    typedef map<string, string> PragmaMap;

    PragmaMap                   m_Pragmas;

    QFileInfo                   m_RawPath;
};

#endif // SHADERBUILDER_H
