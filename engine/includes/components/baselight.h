/*
    This file is part of Thunder Engine.

    Copyright 2008-2026 Evgeniy Prikazchikov

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
