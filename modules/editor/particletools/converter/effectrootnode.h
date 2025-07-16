#ifndef EFFECTROOTNODE_H
#define EFFECTROOTNODE_H

#include <resources/material.h>
#include <resources/mesh.h>

#include <editor/graph/graphnode.h>
#include <editor/projectsettings.h>

class Foldout;
class EffectModule;

class EffectRootNode : public GraphNode {
    A_OBJECT(EffectRootNode, GraphNode, Graph)

    A_PROPERTIES(
        A_PROPERTY(bool, local, EffectRootNode::isLocal, EffectRootNode::setLocal),
        A_PROPERTY(bool, continuous, EffectRootNode::isContinuous, EffectRootNode::setContinuous),
        A_PROPERTY(float, spawnRate, EffectRootNode::spawnRate, EffectRootNode::setSpawnRate),
        A_PROPERTY(int, capacity, EffectRootNode::capacity, EffectRootNode::setCapacity)
    )

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
    void setSpawnRate(float value) { m_spawnRate = value; }

    int capacity() const { return m_capacity; }
    void setCapacity(int value) { m_capacity = value; }

    EffectModule *insertModule(const String &path, int index = -1);
    int moduleIndex(EffectModule *module);

    void removeModule(EffectModule *module);
    void removeAllModules();

    static int getSpace(const String &name);

    void addAttribute(const String &name, int size, int offset = -1);

    int attributeOffset(const String &name);
    int attributeSize(const String &name);

    VariantList saveData() const override;

    static int typeSize(const Variant &value);

private:
    Vector4 color() const override;

    QDomElement toXml(QDomDocument &xml) override;
    void fromXml(const QDomElement &element) override;

    Widget *widget() override;

    bool isRemovable() const override { return false; }

private:
    std::list<EffectModule *> m_modules;

    struct AttributeData {
        String name;

        int32_t size;

        int32_t offset;
    };

    std::list<AttributeData> m_attributes;

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
