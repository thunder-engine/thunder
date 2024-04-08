#ifndef ROOTNODE_H
#define ROOTNODE_H

#include <resources/material.h>

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

class ShaderRootNode : public GraphNode {
    Q_OBJECT

    Q_PROPERTY(Type materialType READ materialType WRITE setMaterialType NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(LightModel lightingModel READ lightModel WRITE setLightModel NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(bool wireFrame READ isWireframe WRITE setWireframe NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(bool twoSided READ isDoubleSided WRITE setDoubleSided NOTIFY graphUpdated DESIGNABLE true USER true)

    Q_PROPERTY(BlendOp blendColorOperation READ blendColorOperation WRITE setBlendColorOperation NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(BlendOp blendAlphaOperation READ blendAlphaOperation WRITE setBlendAlphaOperation NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(BlendFactor blendSourceColor READ blendSourceColor WRITE setBlendSourceColor NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(BlendFactor blendSourceAlpha READ blendSourceAlpha WRITE setBlendSourceAlpha NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(BlendFactor blendDestinationColor READ blendDestinationColor WRITE setBlendDestinationColor NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(BlendFactor blendDestinationAlpha READ blendDestinationAlpha WRITE setBlendDestinationAlpha NOTIFY graphUpdated DESIGNABLE true USER true)

    Q_PROPERTY(bool depthTest READ depthTest WRITE setDepthTest NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(bool depthWrite READ depthWrite WRITE setDepthWrite NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(TestFunction depthCompare READ depthCompare WRITE setDepthCompare NOTIFY graphUpdated DESIGNABLE true USER true)

    Q_PROPERTY(bool stencilTest READ stencilTest WRITE setDepthTest NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(int stencilReadMask READ stencilReadMask WRITE setStencilReadMask NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(int stencilWriteMask READ stencilWriteMask WRITE setStencilWriteMask NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(int stencilReference READ stencilReference WRITE setStencilReference NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(TestFunction stencilCompareBack READ stencilTestCompareBack WRITE setStencilTestCompareBack NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(TestFunction stencilCompareFront READ stencilTestCompareFront WRITE setStencilTestCompareFront NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(ActionType stencilFailBack READ stencilFailOperationBack WRITE setStencilFailOperationBack NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(ActionType stencilFailFront READ stencilFailOperationFront WRITE setStencilFailOperationFront NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(ActionType stencilPassBack READ stencilPassOperationBack WRITE setStencilPassOperationBack NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(ActionType stencilPassFront READ stencilPassOperationFront WRITE setStencilPassOperationFront NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(ActionType stencilZFailBack READ stencilZFailOperationBack WRITE setStencilZFailOperationBack NOTIFY graphUpdated DESIGNABLE true USER true)
    Q_PROPERTY(ActionType stencilZFailFront READ stencilZFailOperationFront WRITE setStencilZFailOperationFront NOTIFY graphUpdated DESIGNABLE true USER true)

public:

    enum Type {
        Surface,
        PostProcess,
        LightFunction
    };

    enum LightModel {
        Unlit,
        Lit,
        Subsurface
    };

    enum BlendOp {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum BlendFactor {
        Zero,
        One,
        SourceColor,
        OneMinusSourceColor,
        DestinationColor,
        OneMinusDestinationColor,
        SourceAlpha,
        OneMinusSourceAlpha,
        DestinationAlpha,
        OneMinusDestinationAlpha,
        SourceAlphaSaturate,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha
    };

    enum ActionType {
        Keep,
        Clear,
        Replace,
        Increment,
        IncrementWrap,
        Decrement,
        DecrementWrap,
        Invert
    };

    enum TestFunction {
        Never,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        Equal,
        NotEqual,
        Always
    };

    enum Flags {
        Cube   = (1<<0),
        Target = (1<<1)
    };

    Q_ENUM(Type)
    Q_ENUM(LightModel)
    Q_ENUM(BlendOp)
    Q_ENUM(BlendFactor)
    Q_ENUM(ActionType)
    Q_ENUM(TestFunction)

    ShaderRootNode() :
            m_lightModel(Lit),
            m_materialType(Surface),
            m_doubleSided(false),
            m_wireframe(false) {

    }

    bool isDoubleSided() const { return m_doubleSided; }
    void setDoubleSided(bool value) { m_doubleSided = value; emit graphUpdated(); }

    bool isWireframe() const { return m_wireframe; }
    void setWireframe(bool value) { m_wireframe = value; emit graphUpdated(); }

    Type materialType() const { return m_materialType; }
    void setMaterialType(Type type) { m_materialType = type; emit graphUpdated(); }

    LightModel lightModel() const { return m_lightModel; }
    void setLightModel(LightModel model) { m_lightModel = model; emit graphUpdated(); }

    BlendOp blendAlphaOperation() const { return static_cast<BlendOp>(m_blendState.alphaOperation); }
    void setBlendAlphaOperation(BlendOp operation) { m_blendState.alphaOperation = operation; emit graphUpdated(); }

    BlendOp blendColorOperation() const { return static_cast<BlendOp>(m_blendState.colorOperation); }
    void setBlendColorOperation(BlendOp operation) { m_blendState.colorOperation = operation; emit graphUpdated(); }

    BlendFactor blendSourceAlpha() const { return static_cast<BlendFactor>(m_blendState.sourceAlphaBlendMode); }
    void setBlendSourceAlpha(BlendFactor factor) { m_blendState.sourceAlphaBlendMode = factor; emit graphUpdated(); }

    BlendFactor blendSourceColor() const { return static_cast<BlendFactor>(m_blendState.sourceColorBlendMode); }
    void setBlendSourceColor(BlendFactor factor) { m_blendState.sourceColorBlendMode = factor; emit graphUpdated(); }

    BlendFactor blendDestinationAlpha() const { return static_cast<BlendFactor>(m_blendState.destinationAlphaBlendMode); }
    void setBlendDestinationAlpha(BlendFactor factor) { m_blendState.destinationAlphaBlendMode = factor; emit graphUpdated(); }

    BlendFactor blendDestinationColor() const { return static_cast<BlendFactor>(m_blendState.destinationColorBlendMode); }
    void setBlendDestinationColor(BlendFactor factor) { m_blendState.destinationColorBlendMode = factor; emit graphUpdated(); }

    Material::BlendState blendState() const { return m_blendState; };
    void setBlendState(const Material::BlendState &state) { m_blendState = state; }

    bool depthTest() const { return m_depthState.enabled; }
    void setDepthTest(bool value) { m_depthState.enabled = value; emit graphUpdated(); }

    bool depthWrite() const { return m_depthState.writeEnabled; }
    void setDepthWrite(bool value) { m_depthState.writeEnabled = value; emit graphUpdated(); }

    TestFunction depthCompare() const { return static_cast<TestFunction>(m_depthState.compareFunction); }
    void setDepthCompare(TestFunction value) { m_depthState.compareFunction = value; emit graphUpdated(); }

    Material::DepthState depthState() const { return m_depthState; }
    void setDepthState(const Material::DepthState &state) { m_depthState = state; }

    bool stencilTest() const { return m_stencilState.enabled; }
    void setStencilTest(bool value) { m_stencilState.enabled = value; emit graphUpdated(); }

    int32_t stencilReadMask() const { return m_stencilState.readMask; }
    void setStencilReadMask(int32_t value) { m_stencilState.readMask = value; emit graphUpdated(); }

    int32_t stencilWriteMask() const { return m_stencilState.writeMask; }
    void setStencilWriteMask(int32_t value) { m_stencilState.writeMask = value; emit graphUpdated(); }

    int32_t stencilReference() const { return m_stencilState.reference; }
    void setStencilReference(int32_t value) { m_stencilState.reference = value; emit graphUpdated(); }

    TestFunction stencilTestCompareBack() const { return static_cast<TestFunction>(m_stencilState.compareFunctionBack); }
    void setStencilTestCompareBack(TestFunction value) { m_stencilState.compareFunctionBack = value; emit graphUpdated(); }

    TestFunction stencilTestCompareFront() const { return static_cast<TestFunction>(m_stencilState.compareFunctionFront); }
    void setStencilTestCompareFront(TestFunction value) { m_stencilState.compareFunctionFront = value; emit graphUpdated(); }

    ActionType stencilFailOperationBack() const { return static_cast<ActionType>(m_stencilState.failOperationBack); }
    void setStencilFailOperationBack(ActionType value) { m_stencilState.failOperationBack = value; emit graphUpdated(); }

    ActionType stencilFailOperationFront() const { return static_cast<ActionType>(m_stencilState.failOperationFront); }
    void setStencilFailOperationFront(ActionType value) { m_stencilState.failOperationFront = value; emit graphUpdated(); }

    ActionType stencilPassOperationBack() const { return static_cast<ActionType>(m_stencilState.passOperationBack); }
    void setStencilPassOperationBack(ActionType value) { m_stencilState.passOperationBack = value; emit graphUpdated(); }

    ActionType stencilPassOperationFront() const { return static_cast<ActionType>(m_stencilState.passOperationFront); }
    void setStencilPassOperationFront(ActionType value) { m_stencilState.passOperationFront = value; emit graphUpdated(); }

    ActionType stencilZFailOperationBack() const { return static_cast<ActionType>(m_stencilState.zFailOperationBack); }
    void setStencilZFailOperationBack(ActionType value) { m_stencilState.zFailOperationBack = value; emit graphUpdated(); }

    ActionType stencilZFailOperationFront() const { return static_cast<ActionType>(m_stencilState.zFailOperationFront); }
    void setStencilZFailOperationFront(ActionType value) { m_stencilState.zFailOperationFront = value; emit graphUpdated(); }


    Material::StencilState stencilState() const { return m_stencilState; }
    void setStencilState(const Material::StencilState &state) { m_stencilState = state; }

    Vector4 color() const override { return Vector4(0.141f, 0.384f, 0.514f, 1.0f); }

signals:
    void graphUpdated();

private:
    Material::BlendState m_blendState;

    Material::DepthState m_depthState;

    Material::StencilState m_stencilState;

    LightModel m_lightModel;

    Type m_materialType;

    bool m_doubleSided;

    bool m_wireframe;

};

Q_DECLARE_METATYPE(Material::LightModel)
Q_DECLARE_METATYPE(Material::Type)

#endif // ROOTNODE_H
