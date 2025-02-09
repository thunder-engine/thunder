#include "components/postprocessvolume.h"

#include "components/private/postprocessorsettings.h"

#include "components/world.h"

#include "components/actor.h"
#include "components/transform.h"

#include "resources/texture.h"

#include "systems/rendersystem.h"

#include "gizmos.h"

/*!
    \class PostProcessVolume
    \brief A post process settings volume.
    \inmodule Components

    Used to affect post process settings in the game and editor.
*/

PostProcessVolume::PostProcessVolume() :
        m_settings(new PostProcessSettings),
        m_priority(0),
        m_blendWeight(1.0f),
        m_unbound(false) {

    for(auto it : m_settings->settings()) {
        Object::setProperty(it.first.c_str(), it.second);
    }
}

PostProcessVolume::~PostProcessVolume() {
    static_cast<RenderSystem *>(system())->removePostProcessVolume(this);
}
/*!
    Returns the priority of volume in the list.
*/
int PostProcessVolume::priority() const {
    return m_priority;
}
/*!
    Sets the \a priority of volume in the list.
*/
void PostProcessVolume::setPriority(int priority) {
    m_priority = priority;
}
/*!
    Returns true in case of component has no bounding volume; otherwise return false.
*/
bool PostProcessVolume::unbound() const {
    return m_unbound;
}
/*!
    Sets flag \a unbound if current settings must be applied entire scene.
*/
void PostProcessVolume::setUnbound(bool unbound) {
    m_unbound = unbound;
}
/*!
    Returns the weight of settings for blending process.
*/
float PostProcessVolume::blendWeight() const {
    return m_blendWeight;
}
/*!
    Sets the \a weight of settings for blending process.
*/
void PostProcessVolume::setBlendWeight(float weight) {
    m_blendWeight = weight;
}
/*!
    \internal
*/
AABBox PostProcessVolume::bound() const {
    Transform *t = transform();
    AABBox result;
    result.center = t->worldPosition();
    result.extent = t->worldScale() * 0.5;
    return result;
}
/*!
    \internal
*/
const PostProcessSettings *PostProcessVolume::settings() const {
    return m_settings;
}
/*!
    \internal
*/
void PostProcessVolume::setProperty(const char *name, const Variant &value) {
    Object::setProperty(name, value);

    m_settings->writeValue(name, value);
}
/*!
    \internal
*/
void PostProcessVolume::setSystem(ObjectSystem *system) {
    Component::setSystem(system);

    RenderSystem *render = static_cast<RenderSystem *>(system);
    render->addPostProcessVolume(this);
}
/*!
    \internal
*/
void PostProcessVolume::drawGizmos() {
    Transform *t = transform();

    if(!m_unbound) {
        Gizmos::drawWireBox(Vector3(), 1.0f, Vector4(0.5f, 0.0f, 0.5f, 1.0f), t->worldTransform());
    }

    Gizmos::drawIcon(t->worldPosition(), Vector2(0.5f), ".embedded/postprocess.png", Vector4(1.0f));
}
