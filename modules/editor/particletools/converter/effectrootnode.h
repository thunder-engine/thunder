#ifndef EFFECTROOTNODE_H
#define EFFECTROOTNODE_H

#include <resources/material.h>
#include <resources/mesh.h>

#include <editor/graph/graphnode.h>
#include <editor/projectsettings.h>

#include <pugixml.hpp>

#include "effectmodule.h"

class Foldout;

class EffectRootNode : public GraphNode {
    A_OBJECT(EffectRootNode, GraphNode, Graph)

    A_PROPERTIES(
        A_PROPERTY(bool, local, EffectRootNode::isLocal, EffectRootNode::setLocal),
        A_PROPERTY(bool, continuous, EffectRootNode::isContinuous, EffectRootNode::setContinuous),
        A_PROPERTY(float, spawnRate, EffectRootNode::spawnRate, EffectRootNode::setSpawnRate),
        A_PROPERTY(int, capacity, EffectRootNode::capacity, EffectRootNode::setCapacity)
    )

    struct ParameterData {
        TString name;
        TString type;
        TString modeType;

        Variant min;
        Variant max;

        EffectModule *module = nullptr;

        int mode = EffectModule::Constant;

        bool visible = true;
    };

public:
    EffectRootNode();
    ~EffectRootNode();

    bool isGpu() const { return m_gpu; }
    void setGpu(bool value) { m_gpu = value; }

    bool isLocal() const { return m_local; }
    void setLocal(bool value) { m_local = value; }

    bool isContinuous() const { return m_continuous; }
    void setContinuous(bool value) { m_local = value; }

    float spawnRate() const { return m_spawnRate; }
    void setSpawnRate(float value);

    int capacity() const { return m_capacity; }
    void setCapacity(int value);

    EffectModule *insertModule(const TString &path, int index = -1);
    int moduleIndex(EffectModule *module);

    void removeModule(EffectModule *module);
    void removeAllModules();

    static int getSpace(const TString &name);

    void addAttribute(const TString &name, int size);

    int attributeOffset(const TString &name);
    int attributeSize(const TString &name);

    void addParameter(const ParameterData &data);

    const ParameterData *parameterConst(const TString &name) const;
    ParameterData *parameter(const TString &name, EffectModule *module);

    std::vector<ParameterData> parameters(EffectModule *owner) const;

    VariantList saveData() const override;

    static int typeSize(const Variant &value);

private:
    Vector4 color() const override;

    void toXml(pugi::xml_node &element) override;
    void fromXml(const pugi::xml_node &element) override;

    Widget *widget() override;

    bool isRemovable() const override { return false; }

private:
    std::list<EffectModule *> m_modules;

    struct AttributeData {
        TString name;

        int32_t size;

        int32_t offset;
    };

    std::list<AttributeData> m_attributes;

    std::vector<ParameterData> m_parameters;

    Foldout *m_spawnFold;
    Foldout *m_updateFold;
    Foldout *m_renderFold;

    float m_spawnRate;

    int m_capacity;

    bool m_gpu;
    bool m_local;
    bool m_continuous;

};

#endif // EFFECTROOTNODE_H
