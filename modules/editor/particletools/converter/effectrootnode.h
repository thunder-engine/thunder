#ifndef EFFECTROOTNODE_H
#define EFFECTROOTNODE_H

#include <resources/material.h>
#include <resources/mesh.h>

#include <editor/graph/graphnode.h>
#include <editor/projectsettings.h>

#include "modules/effectmodule.h"

class Foldout;

class EffectRootNode : public GraphNode {
    A_OBJECT(EffectRootNode, GraphNode, Graph)

    A_PROPERTIES(
        A_PROPERTY(bool, local, EffectRootNode::isLocal, EffectRootNode::setLocal),
        A_PROPERTY(bool, continuous, EffectRootNode::isContinuous, EffectRootNode::setContinuous),
        A_PROPERTY(int, capacity, EffectRootNode::capacity, EffectRootNode::setCapacity)
    )

    struct ParameterData {
        TString name;
        TString type;
        TString modeType;

        Variant min;
        Variant max;

        EffectModule *module = nullptr;

        EffectModule::Space mode = EffectModule::Constant;

        bool visible = true;
    };

public:
    EffectRootNode();
    ~EffectRootNode();

    bool isGpu() const { return m_gpu; }
    void setGpu(bool value);

    bool isLocal() const { return m_local; }
    void setLocal(bool value);

    bool isContinuous() const { return m_continuous; }
    void setContinuous(bool value);

    int capacity() const { return m_capacity; }
    void setCapacity(int value);

    EffectModule *insertModule(const TString &type, int index = -1);
    int moduleIndex(EffectModule *module);

    void removeModule(EffectModule *module);
    void removeAllModules();

    static EffectModule::Space getSpace(const TString &name);

    void addAttribute(const TString &name, MetaType::Type type);

    int attributeOffset(const TString &name);
    int attributeSize(const TString &name);

    void addParameter(const ParameterData &data);

    const ParameterData *parameterConst(const TString &name, bool enabledOnly) const;
    ParameterData *parameter(const TString &name, EffectModule *module);

    std::vector<ParameterData> parameters(EffectModule *owner) const;

    VariantList saveData() const override;

private:
    Vector4 color() const override;

    void toXml(pugi::xml_node &element) override;
    void fromXml(const pugi::xml_node &element) override;

    Foldout *createFold(const TString &name, Actor *parent);

    Widget *widget() override;

    bool isRemovable() const override { return false; }

private:
    struct AttributeData {
        TString name;

        int32_t size;

        int32_t offset;
    };

    std::list<AttributeData> m_attributes;

    std::vector<ParameterData> m_parameters;

    Foldout *m_emitterUpdateFold;
    Foldout *m_particleSpawnFold;
    Foldout *m_particleUpdateFold;
    Foldout *m_renderFold;

    int m_capacity;

    bool m_gpu;
    bool m_local;
    bool m_continuous;

};

#endif // EFFECTROOTNODE_H
