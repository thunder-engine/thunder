#ifndef EFFECTCONVERTER_H
#define EFFECTCONVERTER_H

#include <QObject>
#include <QColor>
#include <QVariant>

#include <editor/assetconverter.h>
#include <resources/particleeffect.h>
#include <resources/material.h>

#include <editor/projectsettings.h>

#include <metatype.h>

class EffectModule;

class EffectEmitter : public QObject {
    Q_OBJECT

public:
    EffectEmitter(QObject *parent = nullptr) :
            QObject(parent),
            m_capacity(32),
            m_distribution(1.0f),
            m_gpu(false),
            m_local(false),
            m_continuous(true) {

    }

    void setName(const QString value) { setObjectName(value); emit updated();}

    bool isGpu() const { return false; }
    void setGpu(bool value) { m_gpu = value; emit updated(); }

    bool isLocal() const { return m_local; }
    void setLocal(bool value) { m_local = value; emit updated(); }

    bool isContinuous() const { return m_continuous; }
    void setContinuous(bool value) { m_continuous = value; emit updated(); }

    float distribution() const { return m_distribution; }
    void setDistribution(float value) { m_distribution = value; emit updated(); }

    int capacity() const { return m_capacity; }
    void setCapacity(int value) { m_capacity = value; emit updated(); }

    Template mesh() const { return Template(m_meshPath, MetaType::type<Mesh *>()); }
    void setMesh(Template mesh) { m_meshPath = mesh.path; emit updated(); }

    Template material() const { return Template(m_materialPath, MetaType::type<Material *>()); }
    void setMaterial(Template material) { m_materialPath = material.path; emit updated(); }

    QString meshPath() const { return m_meshPath; }
    void setMeshPath(const QString &path) { m_meshPath = path; emit updated(); }

    QString materialPath() const { return m_materialPath; }
    void setMaterialPath(const QString &path) { m_materialPath = path; emit updated(); }

    QVariant getFunctions() const {
        QStringList result;
        foreach(QObject *it, children()) {
            result.append(it->metaObject()->className());
        }
        return result;
    }

    QString iconPath() const { return ProjectSettings::instance()->iconPath() + "/" + m_materialPath + ".png"; }

signals:
    void updated();

private:
    int m_capacity;
    float m_distribution;
    bool m_gpu;
    bool m_local;
    bool m_continuous;

    QString m_meshPath;
    QString m_materialPath;

};

class EffectConverter : public AssetConverter {
    Q_OBJECT

public:
    EffectConverter();

    void load(const QString &path);

    EffectModule *createModule(const QString &path);

protected:
    QStringList suffixes() const override { return {"efx"}; }
    ReturnCode convertFile(AssetConverterSettings *) override;
    AssetConverterSettings *createSettings() override;

signals:
    void effectUpdated();

};

#endif // EFFECTCONVERTER_H
