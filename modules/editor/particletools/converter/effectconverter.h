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

class EffectFunction : public QObject {
    Q_OBJECT
    Q_CLASSINFO("Group", "Modificator")
    Q_PROPERTY(ModificatorType Type READ type WRITE setType NOTIFY updated DESIGNABLE true USER true)

public:
    enum ModificatorType {
        Constant = 0,
        Range
    };
    Q_ENUM(ModificatorType)

public:
    Q_INVOKABLE EffectFunction() :
            m_type(Constant) {

    }

    virtual int32_t classType() const { return 0; }

    ModificatorType type() const { return m_type; }
    void setType(ModificatorType type) { m_type = type; emit updated(); }

    Vector4 min() const { return m_min; }
    void setMin(const Vector4 &min) { m_min = min; emit updated(); }

    Vector4 max() const { return m_max; }
    void setMax(const Vector4 &max) { m_max = max; emit updated(); }

    float minFloatValue() const { return m_min.x; }
    void setFloatMinValue(float value) { m_min.x = value; emit updated(); }

    float maxFloatValue() const { return m_max.x; }
    void setFloatMaxValue(float value) { m_max.x = value; emit updated(); }

    Vector3 minValue() const { return Vector3(m_min.x, m_min.y, m_min.z); }
    void setMinValue(Vector3 value) { m_min = Vector4(value, m_min.w); emit updated(); }

    Vector3 maxValue() const { return Vector3(m_max.x, m_max.y, m_max.z); }
    void setMaxValue(Vector3 value) { m_max = Vector4(value, m_max.w); emit updated(); }

    QColor minColorValue() const { return QColor::fromRgbF(m_min.x, m_min.y, m_min.z, m_min.z); }
    void setColorMinValue(QColor value) { m_min = Vector4(value.redF(), value.greenF(), value.blueF(), value.alphaF()); emit updated(); }

    QColor maxColorValue() const { return QColor::fromRgbF(m_max.x, m_max.y, m_max.z, m_max.z); }
    void setColorMaxValue(QColor value) { m_max = Vector4(value.redF(), value.greenF(), value.blueF(), value.alphaF()); emit updated(); }

signals:
    void updated();

protected:
    ModificatorType m_type;

    Vector4 m_min;
    Vector4 m_max;

};

Q_DECLARE_METATYPE(EffectFunction::ModificatorType)

class Lifetime : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(float Value READ minFloatValue WRITE setFloatMinValue DESIGNABLE true USER true)
    Q_PROPERTY(float Min READ minFloatValue WRITE setFloatMinValue DESIGNABLE true USER true)
    Q_PROPERTY(float Max READ maxFloatValue WRITE setFloatMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE Lifetime() { }
    int32_t classType() const { return ParticleModificator::LIFETIME; }
};

class StartSize : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartSize() { }
    int32_t classType() const { return ParticleModificator::STARTSIZE; }
};

class StartColor : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(QColor Value READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Min READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Max READ maxColorValue WRITE setColorMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartColor() { }
    int32_t classType() const { return ParticleModificator::STARTCOLOR; }
};

class StartAngle : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartAngle() { }
    int32_t classType() const { return ParticleModificator::STARTANGLE; }
};

class StartPosition : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE StartPosition() { }
    int32_t classType() const { return ParticleModificator::STARTPOSITION; }
};

class ScaleSize : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE ScaleSize() { }
    int32_t classType() const { return ParticleModificator::SCALESIZE; }
};

class ScaleColor : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(QColor Value READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Min READ minColorValue WRITE setColorMinValue DESIGNABLE true USER true)
    Q_PROPERTY(QColor Max READ maxColorValue WRITE setColorMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE ScaleColor() { }
    virtual int32_t classType() const { return ParticleModificator::SCALECOLOR; }
};

class ScaleAngle : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE ScaleAngle() { }
    virtual int32_t classType() const { return ParticleModificator::SCALEANGLE; }
};

class Velocity : public EffectFunction {
    Q_OBJECT
    Q_PROPERTY(Vector3 Value READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Min READ minValue WRITE setMinValue DESIGNABLE true USER true)
    Q_PROPERTY(Vector3 Max READ maxValue WRITE setMaxValue DESIGNABLE true USER true)
public:
    Q_INVOKABLE Velocity() { }
    virtual int32_t classType() const { return ParticleModificator::VELOCITY; }
};

class EffectEmitter : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString Name READ objectName WRITE setName NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(Template Material READ material WRITE setMaterial NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(float Distribution READ distribution WRITE setDistribution NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(bool Local READ isLocal WRITE setLocal NOTIFY updated DESIGNABLE true USER true)
    Q_PROPERTY(bool Continuous READ isContinuous WRITE setContinuous NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(QVariant _ChildModel READ getFunctions NOTIFY updated DESIGNABLE true USER true)

    Q_PROPERTY(QString _IconPath READ iconPath NOTIFY updated DESIGNABLE true USER true)

public:
    EffectEmitter(QObject *parent = nullptr) :
            QObject(parent),
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
    float m_distribution;
    bool m_gpu;
    bool m_local;
    bool m_continuous;

    QString m_meshPath;
    QString m_materialPath;

};

class EffectConverterSettings : public AssetConverterSettings {
    Q_OBJECT

    Q_PROPERTY(float thumbnailWarmup READ thumbnailWarmup WRITE setThumbnailWarmup DESIGNABLE true USER true)

public:
    EffectConverterSettings();

    float thumbnailWarmup() const;
    void setThumbnailWarmup(float value);

private:
    QString defaultIcon(QString) const Q_DECL_OVERRIDE;

private:
    float m_thumbnailWarmup;

};

class EffectConverter : public AssetConverter {
    Q_OBJECT

public:
    EffectConverter();
    virtual ~EffectConverter() {}

    void load(const QString &path);
    void save(const QString &path);

    EffectEmitter *createEmitter();
    void deleteEmitter(QString name);

    EffectFunction *createFunction(const QString &name, const QString &path);
    void deleteFunction(const QString &name, const QString &path);

    EffectFunction *createFunction(const QString &path);

    Variant data() const;
    Variant object() const;

protected:
    QStringList suffixes() const Q_DECL_OVERRIDE { return {"efx"}; }
    ReturnCode convertFile(AssetConverterSettings *) Q_DECL_OVERRIDE;
    AssetConverterSettings *createSettings() const Q_DECL_OVERRIDE;

    Actor *createActor(const AssetConverterSettings *settings, const QString &guid) const Q_DECL_OVERRIDE;

    QString templatePath() const Q_DECL_OVERRIDE { return ":/templates/ParticleEffect.efx"; }

signals:
    void effectUpdated();

};

#endif // EFFECTCONVERTER_H
