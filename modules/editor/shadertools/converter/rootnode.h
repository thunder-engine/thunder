#ifndef ROOTNODE_H
#define ROOTNODE_H

#include <resources/material.h>

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

class ShaderRootNode : public GraphNode {
    Q_OBJECT

    Q_PROPERTY(Type Material_Type READ materialType WRITE setMaterialType NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(Blend Blending_Mode READ blend WRITE setBlend NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(LightModel Lighting_Model READ lightModel WRITE setLightModel NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(bool Two_Sided READ isDoubleSided WRITE setDoubleSided NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(bool Depth_Test READ isDepthTest WRITE setDepthTest NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(bool Depth_Write READ isDepthWrite WRITE setDepthWrite NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(bool Wireframe READ isWireframe WRITE setWireframe NOTIFY graphUpdated DESIGNABLE true USER true)

public:
    enum LightModel {
        Unlit,
        Lit,
        Subsurface
    };

    enum Blend {
        Opaque,
        Additive,
        Translucent
    };

    enum Type {
        Surface,
        PostProcess,
        LightFunction
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
        m_viewSpace(true),
        m_wireframe(false) {

    }

    bool isDoubleSided() const { return m_doubleSided; }
    void setDoubleSided(bool value) { m_doubleSided = value; emit graphUpdated(); }

    bool isDepthTest() const { return m_depthTest; }
    void setDepthTest(bool value) { m_depthTest = value; emit graphUpdated(); }

    bool isDepthWrite() const { return m_depthWrite; }
    void setDepthWrite(bool value) { m_depthWrite = value; emit graphUpdated(); }

    bool isWireframe() const { return m_wireframe; }
    void setWireframe(bool value) { m_wireframe = value; emit graphUpdated(); }

    Type materialType() const { return m_materialType; }
    void setMaterialType(Type type) { m_materialType = type; emit graphUpdated(); }

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

    bool m_wireframe;

};

Q_DECLARE_METATYPE(ShaderRootNode::LightModel)
Q_DECLARE_METATYPE(ShaderRootNode::Blend)
Q_DECLARE_METATYPE(ShaderRootNode::Type)

#endif // ROOTNODE_H
