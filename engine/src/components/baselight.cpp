#include "baselight.h"

#include "components/transform.h"

#include "systems/rendersystem.h"

#include "material.h"

#define SIDES 6

namespace {
    const char *uniParams("params");
    const char *uniColor("color");
    const char *uniMatrix("matrix");
    const char *uniBias("bias");
};

std::vector<Quaternion> BaseLight::s_directions = {Quaternion(Vector3(0, 1, 0),-90),
                                                   Quaternion(Vector3(0, 1, 0), 90),
                                                   Quaternion(Vector3(1, 0, 0), 90),
                                                   Quaternion(Vector3(1, 0, 0),-90),
                                                   Quaternion(Vector3(0, 1, 0),180),
                                                   Quaternion()};

Matrix4 BaseLight::s_scale = Matrix4(Vector3(0.5f), Quaternion(), Vector3(0.5f));

/*!
    \class BaseLight
    \brief Base class for every light source.
    \inmodule Components

    \note This class must be a superclass only and shouldn't be created manually.
*/

BaseLight::BaseLight() :
        m_params(1.0f, 1.0f, 0.5f, 1.0f),
        m_color(1.0f),
        m_materialInstance(nullptr),
        m_hash(0),
        m_lod(0),
        m_shadows(false),
        m_dirty(true) {

    static uint32_t hash = Mathf::hashString("lights");
    addTagByHash(hash);
}

BaseLight::~BaseLight() {

}

/*!
    Returns true if the light source can cast shadows; otherwise returns false.
*/
bool BaseLight::castShadows() const {
    return m_shadows;
}
/*!
    Enables or disables cast \a shadows ability for the light source.
*/
void BaseLight::setCastShadows(const bool shadows) {
    m_shadows = shadows;
    m_dirty = true;
}
/*!
    Returns a brightness of emitting light.
*/
float BaseLight::brightness() const {
    return m_params.x;
}
/*!
    Changes a \a brightness of emitting light.
*/
void BaseLight::setBrightness(const float brightness) {
    m_params.x = brightness;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniParams, &m_params);
    }
}
/*!
    Returns a color of emitting light.
*/
Vector4 BaseLight::color() const {
    return m_color;
}
/*!
    Changes a \a color of emitting light.
*/
void BaseLight::setColor(const Vector4 color) {
    m_color = color;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniColor, &m_color);
    }
}
/*!
    Return a type of the light.
    Fot more details refer to BaseLight::LightType
*/
int BaseLight::lightType() const {
    return Invalid;
}
/*!
    Returns number of shadow map atlas tiles required for this light source.
*/
int BaseLight::tilesCount() const {
    return 6; // For point light by the default
}
/*!
    Returns the crop matrix at cascade \a index.
*/
const Matrix4 &BaseLight::cropMatrix(int index) {
    if(m_dirty) {
        cleanDirty();
    }
    return m_cropMatrix[index];
}
/*!
    \internal
*/
void BaseLight::cleanDirty() {
    // point light by the default
    float zNear = 0.1f;
    float zFar = m_params.w;
    Matrix4 crop(Matrix4::perspective(90.0f, 1.0f, zNear, zFar));

    Transform *t = transform();
    m_hash = t->hash();

    Vector3 position(t->worldTransform().position());
    Matrix4 wp;
    wp.translate(position);

    Matrix4 matrix[6];

    m_viewFrustum.resize(SIDES);
    m_cropMatrix.resize(SIDES);
    for(int32_t i = 0; i < s_directions.size(); i++) {
        m_cropMatrix[i] = (wp * Matrix4(s_directions[i].toMatrix())).inverse() * crop;
        matrix[i] = s_scale * m_cropMatrix[i];
    }

    if(m_materialInstance) {
        Vector4 bias;
        m_materialInstance->setVector4(uniBias, &bias);
        m_materialInstance->setMatrix4(uniMatrix, matrix, SIDES);
    }

    m_dirty = false;
}
/*!
    \internal
*/
MaterialInstance *BaseLight::material() const {
    return m_materialInstance;
}

bool BaseLight::isCulled(const Frustum &frustum, const Matrix4 &viewProjection) {
    A_UNUSED(frustum);
    A_UNUSED(viewProjection);
    return false;
}

const Renderable::GroupList &BaseLight::groups(int index) const {
    return m_groups[index];
}

void BaseLight::buildGroups(const Renderable::RenderList &list) {
    if(m_hash != transform()->hash()) {
        m_dirty = true;
    }
    if(m_dirty) {
        cleanDirty();
    }

    int count = tilesCount();
    if(m_groups.empty()) {
        m_groups.resize(count);
    }

    for(int i = 0; i < count; i++) {
        Renderable::RenderList culled;
        const Frustum &frustom = m_viewFrustum[i];
        for(auto it : list) {
            if(!it->isCulled(frustom, m_cropMatrix[i])) {
                culled.push_back(it);
            }
        }

        Renderable::GroupList groupList;
        Renderable::filterByLayer(culled, groupList, Material::Shadowcast);

        m_groups[i].clear();
        Renderable::group(groupList, m_groups[i]);
    }
}
/*!
    \internal
*/
void BaseLight::setMaterial(MaterialInstance *instance) {
    m_materialInstance = instance;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniParams, &m_params);
        m_materialInstance->setVector4(uniColor, &m_color);
    }
}
/*!
    \internal
*/
Vector4 BaseLight::params() const {
    return m_params;
}
/*!
    \internal
*/
void BaseLight::setParams(Vector4 &params) {
    m_params = params;
    if(m_materialInstance) {
        m_materialInstance->setVector4(uniParams, &m_params);
    }
}
/*!
    \internal
*/
Vector4 BaseLight::gizmoColor() const {
    return Vector4(1.0f, 1.0f, 0.5f, 1.0f);
}
