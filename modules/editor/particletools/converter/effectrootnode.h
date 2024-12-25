#ifndef EFFECTROOTNODE_H
#define EFFECTROOTNODE_H

#include <resources/material.h>
#include <resources/mesh.h>

#include <editor/graph/graphnode.h>
#include <editor/projectsettings.h>

class Foldout;
class EffectModule;

class EffectRootNode : public GraphNode {
    Q_OBJECT

    Q_PROPERTY(bool local READ isLocal WRITE setLocal NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool continuous READ isContinuous WRITE setContinuous NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(float spawnRate READ spawnRate WRITE setSpawnRate NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY updated DESIGNABLE true USER true)

public:
    EffectRootNode();
    ~EffectRootNode();

    bool isGpu() const { return m_gpu; }
    void setGpu(bool value) { m_gpu = value; emit updated(); }

    bool isLocal() const { return m_local; }
    void setLocal(bool value) { m_local = value; emit updated(); }

    bool isContinuous() const { return m_continuous; }
    void setContinuous(bool value) { m_local = value; emit updated(); }

    float spawnRate() const { return m_spawnRate; }
    void setSpawnRate(float value) { m_spawnRate = value; emit updated(); }

    int capacity() const { return m_capacity; }
    void setCapacity(int value) { m_capacity = value; emit updated(); }

    EffectModule *addModule(const std::string &path);

    void removeModule(EffectModule *function);
    void removeAllModules();

    static int getSpace(const std::string &name);

    void addAttribute(const std::string &name, int size, int offset = -1);

    int attributeOffset(const std::string &name);
    int attributeSize(const std::string &name);

    VariantList saveData() const;

    static int typeSize(const QVariant &value);

public slots:
    void onShowMenu() const;

private:
    Vector4 color() const override;

    QDomElement toXml(QDomDocument &xml) override;
    void fromXml(const QDomElement &element) override;

    Widget *widget() override;

    bool isRemovable() const override { return false; }

private:
    std::list<EffectModule *> m_modules;

    struct AttributeData {
        std::string name;

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
