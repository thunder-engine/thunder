#ifndef EFFECTCONVERTER_H
#define EFFECTCONVERTER_H

#include <QObject>

#include "converters/converter.h"
#include "resources/particleeffect.h"
#include "resources/material.h"

#include "assetmanager.h"
#include "projectmanager.h"

#include <metatype.h>

#include <QVariant>

class EffectFunction : public QObject {
    Q_OBJECT
    Q_CLASSINFO("Group", "Modificator")
    Q_PROPERTY(ModificatorType Type READ type WRITE setType DESIGNABLE true USER true)

public:
    enum ModificatorType {
        Constant    = 0,
        Range
    };
    Q_ENUM(ModificatorType)

public:
    Q_INVOKABLE EffectFunction() :
        m_Type(Constant) {

    }

    virtual int32_t classType() const { return 0; }

    ModificatorType type() const { return m_Type; }
    void setType(ModificatorType type) { m_Type = type; emit updated(); }

    Vector4 min() const { return m_Min; }
    void setMin(const Vector4 &min) { m_Min = min; emit updated(); }

    Vector4 max() const { return m_Max; }
    void setMax(const Vector4 &max) { m_Max = max; emit updated(); }

    float minFloatValue() const { return m_Min.x; }
    void setFloatMinValue(float value) { m_Min.x = value; emit updated(); }

    float maxFloatValue() const { return m_Max.x; }
    void setFloatMaxValue(float value) { m_Max.x = value; emit updated(); }

    Vector3 minValue() const { return Vector3(m_Min.x, m_Min.y, m_Min.z); }
    void setMinValue(Vector3 value) { m_Min = Vector4(value, m_Min.w); emit updated(); }

    Vector3 maxValue() const { return Vector3(m_Max.x, m_Max.y, m_Max.z); }
    void setMaxValue(Vector3 value) { m_Max = Vector4(value, m_Max.w); emit updated(); }

    QColor minColorValue() const { return QColor::fromRgbF(m_Min.x, m_Min.y, m_Min.z, m_Min.z); }
    void setColorMinValue(QColor value) { m_Min = Vector4(value.redF(), value.greenF(), value.blueF(), value.alphaF()); emit updated(); }

    QColor maxColorValue() const { return QColor::fromRgbF(m_Max.x, m_Max.y, m_Max.z, m_Max.z); }
    void setColorMaxValue(QColor value) { m_Max = Vector4(value.redF(), value.greenF(), value.blueF(), value.alphaF()); emit updated(); }

signals:
    void updated();

protected:
    ModificatorType m_Type;

    Vector4 m_Min;
    Vector4 m_Max;
};

class Lifetime : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(float Value READ minFloatValue WRITE setFloatMinValue DESIGNABLE true USER true)
    Q_PROPERTY(float Min READ minFloatValue WRITE setFloatMinValue DESIGNABLE true USER true)
    Q_PROPERTY(float Max READ maxFloatValue WRITE setFloatMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE Lifetime() { }
    int32_t classType() const { return ParticleEffect::MODIFICATOR_LIFETIME; }
};

class StartSize : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartSize() { }
    int32_t classType() const { return ParticleEffect::MODIFICATOR_STARTSIZE; }
};

class StartColor : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(QColor Value READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Min READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Max READ maxColorValue WRITE setColorMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartColor() { }
    int32_t classType() const { return ParticleEffect::MODIFICATOR_STARTCOLOR; }
};

class StartAngle : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartAngle() { }
    int32_t classType() const { return ParticleEffect::MODIFICATOR_STARTANGLE; }
};

class StartPosition : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartPosition() { }
    int32_t classType() const { return ParticleEffect::MODIFICATOR_STARTPOSITION; }
};

class ScaleSize : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE ScaleSize() { }
    int32_t classType() const { return ParticleEffect::MODIFICATOR_SCALESIZE; }
};

class ScaleColor : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(QColor Value READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Min READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Max READ maxColorValue WRITE setColorMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE ScaleColor() { }
    virtual int32_t classType() const { return ParticleEffect::MODIFICATOR_SCALECOLOR; }
};

class ScaleAngle : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE ScaleAngle() { }
    virtual int32_t classType() const { return ParticleEffect::MODIFICATOR_SCALEANGLE; }
};

class ScaleVelocity : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE ScaleVelocity() { }
    virtual int32_t classType() const { return ParticleEffect::MODIFICATOR_VELOCITY; }
};

class EffectEmitter : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Name READ objectName WRITE setName NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Template Material READ material WRITE setMaterial DESIGNABLE true USER true)

    Q_PROPERTY(float Distribution READ distribution WRITE setDistribution DESIGNABLE true USER true)

    Q_PROPERTY(bool Local READ isLocal WRITE setLocal DESIGNABLE true USER true)
    Q_PROPERTY(bool Continuous READ isContinuous WRITE setContinuous DESIGNABLE true USER true)

    Q_PROPERTY(QVariant _ChildModel READ getFunctions NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(QString _IconPath READ iconPath NOTIFY updated DESIGNABLE true USER true)

public:
    EffectEmitter(QObject *parent = nullptr) :
            QObject(parent),
            m_Distribution(1.0f),
            m_Gpu(false),
            m_Local(false),
            m_Continuous(true) {

    }

    void setName(const QString value) { setObjectName(value); emit updated();}

    bool isGpu() const { return false; }
    void setGpu(bool value) { m_Gpu = value; emit updated(); }

    bool isLocal() const { return true; }
    void setLocal(bool value) { m_Local = value; emit updated(); }

    bool isContinuous() const { return m_Continuous; }
    void setContinuous(bool value) { m_Continuous = value; emit updated(); }

    float distribution() const { return m_Distribution; }
    void setDistribution(float value) { m_Distribution = value; emit updated(); }

    Template mesh() const { return Template(m_MeshPath, MetaType::type<Mesh *>()); }
    void setMesh(Template mesh) { m_MeshPath = mesh.path; emit updated(); }

    Template material() const { return Template(m_MaterialPath, MetaType::type<Material *>()); }
    void setMaterial(Template material) { m_MaterialPath = material.path; emit updated(); }

    QString meshPath() const { return m_MeshPath; }
    void setMeshPath(const QString &path) { m_MeshPath = path; emit updated(); }

    QString materialPath() const { return m_MaterialPath; }
    void setMaterialPath(const QString &path) { m_MaterialPath = path; emit updated(); }

    QVariant getFunctions() const {
        QStringList result;
        foreach(QObject *it, children()) {
            result.append(it->metaObject()->className());
        }
        return result;
    }

    QString iconPath() const { return ProjectManager::instance()->iconPath() + "/" + m_MaterialPath + ".png"; }

signals:
    void updated();

private:
    float m_Distribution;
    bool m_Gpu;
    bool m_Local;
    bool m_Continuous;

    QString m_MeshPath;
    QString m_MaterialPath;

};

class EffectConverter : public QObject, public IConverter {
    Q_OBJECT

public:
    EffectConverter();
    virtual ~EffectConverter() {}

    void load(const QString &path);
    void save(const QString &path);

    string format() const { return "efx"; }
    uint32_t contentType() const { return ContentEffect; }
    uint32_t type() const { return MetaType::type<ParticleEffect *>(); }
    uint8_t convertFile(IConverterSettings *);

    EffectEmitter *createEmitter();
    void deleteEmitter(QString name);

    EffectFunction *createFunction(const QString &name, const QString &path);
    void deleteFunction(const QString &name, const QString &path);

    EffectFunction *createFunction(const QString &path);

    Variant data() const;
    Variant object() const;

signals:
    void effectUpdated();

};

#endif // EFFECTCONVERTER_H
