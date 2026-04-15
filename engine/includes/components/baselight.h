#ifndef BASELIGHT_H
#define BASELIGHT_H

#include <renderable.h>

#include <amath.h>

class MaterialInstance;
class Camera;

class ENGINE_EXPORT BaseLight : public NativeBehaviour {
    A_OBJECT(BaseLight, NativeBehaviour, General)

    A_PROPERTIES(
        A_PROPERTY(bool, castShadows, BaseLight::castShadows, BaseLight::setCastShadows),
        A_PROPERTY(float, brightness, BaseLight::brightness, BaseLight::setBrightness),
        A_PROPERTYEX(Vector4, color, BaseLight::color, BaseLight::setColor, "editor=Color")
    )
    A_NOMETHODS()

public:
    enum LightType {
        Invalid,
        DirectLight,
        AreaLight,
        PointLight,
        SpotLight
    };

public:
    BaseLight();
    ~BaseLight();

    bool castShadows() const;
    void setCastShadows(const bool shadows);

    float brightness() const;
    void setBrightness(const float brightness);

    Vector4 color() const;
    void setColor(const Vector4 color);

    virtual int lightType() const;

    MaterialInstance *material() const;

    virtual bool isCulled(const Frustum &frustum, const Matrix4 &viewProjection);

    virtual int tilesCount() const;

    const Matrix4 &cropMatrix(int index);

    const Renderable::GroupList &groups(int index) const;

    void buildGroups(const Renderable::RenderList &list);


protected:
    void setMaterial(MaterialInstance *instance);

    Vector4 params() const;
    void setParams(Vector4 &params);

    Vector4 gizmoColor() const;

    virtual void cleanDirty();

private:
    void setSystem(ObjectSystem *system) override;

protected:
    static std::vector<Quaternion> s_directions;

    static Matrix4 s_scale;

    std::vector<Matrix4> m_cropMatrix;

    std::vector<Frustum> m_viewFrustum;

    std::vector<Renderable::GroupList> m_groups;

    Vector4 m_params;

    Vector4 m_color;

    MaterialInstance *m_materialInstance;

    uint32_t m_hash;

    uint32_t m_lod;

    bool m_shadows;

    bool m_dirty;

};

#endif // BASELIGHT_H
