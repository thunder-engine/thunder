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

    Q_PROPERTY(bool gpu READ isGpu WRITE setGpu NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool local READ isLocal WRITE setLocal NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Template material READ material WRITE setMaterial NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Template mesh READ mesh WRITE setMesh NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY updated DESIGNABLE true USER true)

public:
    EffectRootNode();
    ~EffectRootNode();

    Template material() const { return Template(m_materialPath.c_str(), MetaType::type<Material *>()); }
    void setMaterial(Template material) { m_materialPath = material.path.toStdString(); emit updated(); }

    Template mesh() const { return Template(m_meshPath.c_str(), MetaType::type<Mesh *>()); }
    void setMesh(Template material) { m_meshPath = material.path.toStdString(); emit updated(); }

    std::string meshPath() const;

    std::string materialPath() const;

    bool isGpu() const { return m_gpu; }
    void setGpu(bool value) { m_gpu = value; emit updated(); }

    bool isLocal() const { return m_local; }
    void setLocal(bool value) { m_local = value; emit updated(); }

    bool isContinuous() const { return m_continuous; }
    void setContinuous(bool value) { m_local = value; emit updated(); }

    float spawnRate() const { return m_continuous; }
    void setSpawnRate(float value) { m_continuous = value; emit updated(); }

    int capacity() const { return m_capacity; }
    void setCapacity(int value) { m_capacity = value; emit updated(); }

    EffectModule *addModule(const std::string &path);

    void removeModule(EffectModule *function);

    void addEmitterAttribute(const std::string &name, int32_t type);

    int emitterAttributeOffset(const std::string &name);
    int emitterAttributeSize(const std::string &name);

    void addParticleAttribute(const std::string &name, int32_t type);

    int particleAttributeOffset(const std::string &name);
    int particleAttributeSize(const std::string &name);

    VariantList saveData() const;

    static int typeSize(int type);

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

    std::list<std::pair<std::string, int32_t>> m_emitterAttributes;
    std::list<std::pair<std::string, int32_t>> m_particleAttributes;

    std::string m_meshPath;
    std::string m_materialPath;

    Foldout *m_spawnFold;
    Foldout *m_updateFold;

    float m_spawnRate;

    int m_capacity;

    bool m_gpu;
    bool m_local;
    bool m_continuous;
};

#endif // EFFECTROOTNODE_H
