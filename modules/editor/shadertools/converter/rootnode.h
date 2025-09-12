#ifndef ROOTNODE_H
#define ROOTNODE_H

#include <resources/material.h>

#include <editor/graph/graphnode.h>
#include <editor/graph/abstractnodegraph.h>

class ShaderRootNode : public GraphNode {
    A_OBJECT(ShaderRootNode, GraphNode, Graph)

    A_PROPERTIES(
        A_PROPERTYEX(int, materialType, ShaderRootNode::materialType, ShaderRootNode::setMaterialType, "enum=MaterialType"),
        A_PROPERTYEX(int, lightingModel, ShaderRootNode::lightModel, ShaderRootNode::setLightModel, "enum=LightModel"),
        A_PROPERTY(bool, wireFrame, ShaderRootNode::isWireframe, ShaderRootNode::setWireframe),
        A_PROPERTY(bool, twoSided, ShaderRootNode::isDoubleSided, ShaderRootNode::setDoubleSided),

        A_PROPERTY(bool, useWithSkinned, ShaderRootNode::useWithSkinned, ShaderRootNode::setUseWithSkinned),
        A_PROPERTY(bool, useWithParticles, ShaderRootNode::useWithParticles, ShaderRootNode::setUseWithParticles),

        A_PROPERTYEX(int, blendColorOperation, ShaderRootNode::blendColorOperation, ShaderRootNode::setBlendColorOperation, "enum=BlendOp"),
        A_PROPERTYEX(int, blendAlphaOperation, ShaderRootNode::blendAlphaOperation, ShaderRootNode::setBlendAlphaOperation, "enum=BlendOp"),
        A_PROPERTYEX(int, blendSourceColor, ShaderRootNode::blendSourceColor, ShaderRootNode::setBlendSourceColor, "enum=BlendFactor"),
        A_PROPERTYEX(int, blendSourceAlpha, ShaderRootNode::blendSourceAlpha, ShaderRootNode::setBlendSourceAlpha, "enum=BlendFactor"),
        A_PROPERTYEX(int, blendDestinationColor, ShaderRootNode::blendDestinationColor, ShaderRootNode::setBlendDestinationColor, "enum=BlendFactor"),
        A_PROPERTYEX(int, blendDestinationAlpha, ShaderRootNode::blendDestinationAlpha, ShaderRootNode::setBlendDestinationAlpha, "enum=BlendFactor"),

        A_PROPERTY(bool, depthTest, ShaderRootNode::depthTest, ShaderRootNode::setDepthTest),
        A_PROPERTY(bool, depthWrite, ShaderRootNode::depthWrite, ShaderRootNode::setDepthWrite),
        A_PROPERTYEX(int, depthCompare, ShaderRootNode::depthCompare, ShaderRootNode::setDepthCompare, "enum=TestFunction"),

        A_PROPERTY(bool, stencilTest, ShaderRootNode::stencilTest, ShaderRootNode::setStencilTest),
        A_PROPERTY(int, stencilReadMask, ShaderRootNode::stencilReadMask, ShaderRootNode::setStencilReadMask),
        A_PROPERTY(int, stencilWriteMask, ShaderRootNode::stencilWriteMask, ShaderRootNode::setStencilWriteMask),
        A_PROPERTY(int, stencilReference, ShaderRootNode::stencilReference, ShaderRootNode::setStencilReference),
        A_PROPERTYEX(int, stencilCompareBack, ShaderRootNode::stencilTestCompareBack, ShaderRootNode::setStencilTestCompareBack, "enum=TestFunction"),
        A_PROPERTYEX(int, stencilCompareFront, ShaderRootNode::stencilTestCompareFront, ShaderRootNode::setStencilTestCompareFront, "enum=TestFunction"),
        A_PROPERTYEX(int, stencilFailBack, ShaderRootNode::stencilFailOperationBack, ShaderRootNode::setStencilFailOperationBack, "enum=ActionType"),
        A_PROPERTYEX(int, stencilFailFront, ShaderRootNode::stencilFailOperationFront, ShaderRootNode::setStencilFailOperationFront, "enum=ActionType"),
        A_PROPERTYEX(int, stencilPassBack, ShaderRootNode::stencilPassOperationBack, ShaderRootNode::setStencilPassOperationBack, "enum=ActionType"),
        A_PROPERTYEX(int, stencilPassFront, ShaderRootNode::stencilPassOperationFront, ShaderRootNode::setStencilPassOperationFront, "enum=ActionType"),
        A_PROPERTYEX(int, stencilZFailBack, ShaderRootNode::stencilZFailOperationBack, ShaderRootNode::setStencilZFailOperationBack, "enum=ActionType"),
        A_PROPERTYEX(int, stencilZFailFront, ShaderRootNode::stencilZFailOperationFront, ShaderRootNode::setStencilZFailOperationFront, "enum=ActionType")
    )

    A_ENUMS(
        A_ENUM(MaterialType,
               A_VALUE(Surface),
               A_VALUE(PostProcess),
               A_VALUE(LightFunction)),
        A_ENUM(LightModel,
               A_VALUE(Unlit),
               A_VALUE(Lit),
               A_VALUE(Subsurface)),
        A_ENUM(BlendOp,
               A_VALUE(Add),
               A_VALUE(Subtract),
               A_VALUE(ReverseSubtract),
               A_VALUE(Min),
               A_VALUE(Max)),
        A_ENUM(BlendFactor,
               A_VALUE(Zero),
               A_VALUE(One),
               A_VALUE(SourceColor),
               A_VALUE(OneMinusSourceColor),
               A_VALUE(DestinationColor),
               A_VALUE(OneMinusDestinationColor),
               A_VALUE(SourceAlpha),
               A_VALUE(OneMinusSourceAlpha),
               A_VALUE(DestinationAlpha),
               A_VALUE(OneMinusDestinationAlpha),
               A_VALUE(SourceAlphaSaturate),
               A_VALUE(ConstantColor),
               A_VALUE(OneMinusConstantColor),
               A_VALUE(ConstantAlpha),
               A_VALUE(OneMinusConstantAlpha)),
        A_ENUM(ActionType,
               A_VALUE(Keep),
               A_VALUE(Clear),
               A_VALUE(Replace),
               A_VALUE(Increment),
               A_VALUE(IncrementWrap),
               A_VALUE(Decrement),
               A_VALUE(DecrementWrap),
               A_VALUE(Invert)),
        A_ENUM(TestFunction,
               A_VALUE(Never),
               A_VALUE(Less),
               A_VALUE(LessOrEqual),
               A_VALUE(Greater),
               A_VALUE(GreaterOrEqual),
               A_VALUE(Equal),
               A_VALUE(NotEqual),
               A_VALUE(Always)),
        A_ENUM(Flags,
               A_VALUE(Cube),
               A_VALUE(Target))
    )

public:

    enum MaterialType {
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

    ShaderRootNode() :
            m_lightModel(Lit),
            m_materialType(Surface),
            m_doubleSided(false),
            m_wireframe(false),
            m_useWithSkinned(true),
            m_useWithParticles(true) {

        setTypeName("ShaderRootNode");
    }

    bool isDoubleSided() const { return m_doubleSided; }
    void setDoubleSided(bool value) { m_doubleSided = value; }

    bool isWireframe() const { return m_wireframe; }
    void setWireframe(bool value) { m_wireframe = value; }

    int materialType() const { return m_materialType; }
    void setMaterialType(int type) { m_materialType = type; }

    int lightModel() const { return m_lightModel; }
    void setLightModel(int model) { m_lightModel = model; }

    bool useWithSkinned() const { return m_useWithSkinned; }
    void setUseWithSkinned(bool use) { m_useWithSkinned = use; }

    bool useWithParticles() const { return m_useWithParticles; }
    void setUseWithParticles(bool use) { m_useWithParticles = use; }

    int blendAlphaOperation() const { return static_cast<BlendOp>(m_blendState.alphaOperation); }
    void setBlendAlphaOperation(int operation) { m_blendState.alphaOperation = operation; }

    int blendColorOperation() const { return static_cast<BlendOp>(m_blendState.colorOperation); }
    void setBlendColorOperation(int operation) { m_blendState.colorOperation = operation; }

    int blendSourceAlpha() const { return static_cast<BlendFactor>(m_blendState.sourceAlphaBlendMode); }
    void setBlendSourceAlpha(int factor) { m_blendState.sourceAlphaBlendMode = factor; }

    int blendSourceColor() const { return static_cast<BlendFactor>(m_blendState.sourceColorBlendMode); }
    void setBlendSourceColor(int factor) { m_blendState.sourceColorBlendMode = factor; }

    int blendDestinationAlpha() const { return static_cast<BlendFactor>(m_blendState.destinationAlphaBlendMode); }
    void setBlendDestinationAlpha(int factor) { m_blendState.destinationAlphaBlendMode = factor; }

    int blendDestinationColor() const { return static_cast<BlendFactor>(m_blendState.destinationColorBlendMode); }
    void setBlendDestinationColor(int factor) { m_blendState.destinationColorBlendMode = factor; }

    Material::BlendState blendState() const { return m_blendState; };
    void setBlendState(const Material::BlendState &state) { m_blendState = state; }

    bool depthTest() const { return m_depthState.enabled; }
    void setDepthTest(bool value) { m_depthState.enabled = value; }

    bool depthWrite() const { return m_depthState.writeEnabled; }
    void setDepthWrite(bool value) { m_depthState.writeEnabled = value; }

    int depthCompare() const { return static_cast<TestFunction>(m_depthState.compareFunction); }
    void setDepthCompare(int value) { m_depthState.compareFunction = value; }

    Material::DepthState depthState() const { return m_depthState; }
    void setDepthState(const Material::DepthState &state) { m_depthState = state; }

    bool stencilTest() const { return m_stencilState.enabled; }
    void setStencilTest(bool value) { m_stencilState.enabled = value; }

    int stencilReadMask() const { return m_stencilState.readMask; }
    void setStencilReadMask(int value) { m_stencilState.readMask = value; }

    int stencilWriteMask() const { return m_stencilState.writeMask; }
    void setStencilWriteMask(int value) { m_stencilState.writeMask = value; }

    int stencilReference() const { return m_stencilState.reference; }
    void setStencilReference(int value) { m_stencilState.reference = value; }

    int stencilTestCompareBack() const { return static_cast<TestFunction>(m_stencilState.compareFunctionBack); }
    void setStencilTestCompareBack(int value) { m_stencilState.compareFunctionBack = value; }

    int stencilTestCompareFront() const { return static_cast<TestFunction>(m_stencilState.compareFunctionFront); }
    void setStencilTestCompareFront(int value) { m_stencilState.compareFunctionFront = value; }

    int stencilFailOperationBack() const { return static_cast<ActionType>(m_stencilState.failOperationBack); }
    void setStencilFailOperationBack(int value) { m_stencilState.failOperationBack = value; }

    int stencilFailOperationFront() const { return static_cast<ActionType>(m_stencilState.failOperationFront); }
    void setStencilFailOperationFront(int value) { m_stencilState.failOperationFront = value; }

    int stencilPassOperationBack() const { return static_cast<ActionType>(m_stencilState.passOperationBack); }
    void setStencilPassOperationBack(int value) { m_stencilState.passOperationBack = value; }

    int stencilPassOperationFront() const { return static_cast<ActionType>(m_stencilState.passOperationFront); }
    void setStencilPassOperationFront(int value) { m_stencilState.passOperationFront = value; }

    int stencilZFailOperationBack() const { return static_cast<ActionType>(m_stencilState.zFailOperationBack); }
    void setStencilZFailOperationBack(int value) { m_stencilState.zFailOperationBack = value; }

    int stencilZFailOperationFront() const { return static_cast<ActionType>(m_stencilState.zFailOperationFront); }
    void setStencilZFailOperationFront(int value) { m_stencilState.zFailOperationFront = value; }

    Material::StencilState stencilState() const { return m_stencilState; }
    void setStencilState(const Material::StencilState &state) { m_stencilState = state; }

    Vector4 color() const override { return Vector4(0.141f, 0.384f, 0.514f, 1.0f); }

private:
    Material::BlendState m_blendState;

    Material::DepthState m_depthState;

    Material::StencilState m_stencilState;

    int m_lightModel;

    int m_materialType;

    bool m_doubleSided;

    bool m_wireframe;

    bool m_useWithSkinned;

    bool m_useWithParticles;

};

#endif // ROOTNODE_H
