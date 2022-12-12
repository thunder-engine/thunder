#include "components/postprocessvolume.h"

#include "components/world.h"

#include "components/actor.h"
#include "components/transform.h"
#include "components/private/postprocessorsettings.h"

#include "resources/texture.h"

namespace {
    const char *gComponents("Components");
    const char *gVolume("PostProcessVolume");
}

/*!
    \class PostProcessVolume
    \brief A post process settings volume.
    \inmodule Engine

    Used to affect post process settings in the game and editor.
*/

PostProcessVolume::PostProcessVolume() :
    m_settings(new PostProcessSettings),
    m_metaObject(nullptr),
    m_priority(0),
    m_blendWeight(1.0f),
    m_unbound(false) {

    m_propertyTable.clear();
    for(auto &it : PostProcessSettings::settings()) {
        Variant defaultValue = it.second;
        MetaType::Table *table = MetaType::table(defaultValue.type());
        if(table) {
            m_propertyTable.push_back({it.first.c_str(), table, nullptr, nullptr, nullptr, nullptr, nullptr,
                                       &Reader<decltype(&PostProcessVolume::readProperty), &PostProcessVolume::readProperty>::read,
                                       &Writer<decltype(&PostProcessVolume::writeProperty), &PostProcessVolume::writeProperty>::write});
        }

        m_settings->writeValue(it.first.c_str(), defaultValue);
    }
    m_propertyTable.push_back({nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr});

    m_metaObject = new MetaObject(gVolume, PostProcessVolume::metaClass(), &PostProcessVolume::construct,
                                  nullptr, m_propertyTable.data(), nullptr);
}

PostProcessVolume::~PostProcessVolume() {

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
const PostProcessSettings &PostProcessVolume::settings() const {
    return *m_settings;
}
/*!
    \internal
*/
const MetaObject *PostProcessVolume::metaClass() {
    OBJECT_CHECK(PostProcessVolume)
    static const MetaObject staticMetaData(
        gVolume, Component::metaClass(), &PostProcessVolume::construct,
        reinterpret_cast<const MetaMethod::Table *>(expose_method<PostProcessVolume>::exec()),
        reinterpret_cast<const MetaProperty::Table *>(expose_props_method<PostProcessVolume>::exec()),
        reinterpret_cast<const MetaEnum::Table *>(expose_enum<PostProcessVolume>::exec())
    );
    return &staticMetaData;
}
/*!
    \internal
*/
void PostProcessVolume::registerClassFactory(ObjectSystem *system) {
    REGISTER_META_TYPE(PostProcessVolume);
    system->factoryAdd<PostProcessVolume>(gComponents, PostProcessVolume::metaClass());
}
/*!
    \internal
*/
void PostProcessVolume::unregisterClassFactory(ObjectSystem *system) {
    UNREGISTER_META_TYPE(PostProcessVolume);
    system->factoryRemove<PostProcessVolume>(gComponents);
}
/*!
    \internal
*/
Object *PostProcessVolume::construct() {
    return new PostProcessVolume();
}
/*!
    \internal
*/
const MetaObject *PostProcessVolume::metaObject() const {
    PROFILE_FUNCTION();
    if(m_metaObject) {
        return m_metaObject;
    }
    return PostProcessVolume::metaClass();
}
/*!
    \internal
*/
Variant PostProcessVolume::readProperty(const MetaProperty &property) const {
    PROFILE_FUNCTION();

    return m_settings->readValue(property.name());
}
/*!
    \internal
*/
void PostProcessVolume::writeProperty(const MetaProperty &property, const Variant value) {
    PROFILE_FUNCTION();

    m_settings->writeValue(property.name(), value);
}

#ifdef SHARED_DEFINE
#include "viewport/handles.h"

bool PostProcessVolume::drawHandles(ObjectList &selected) {
    A_UNUSED(selected);
    Transform *t = transform();

    if(!m_unbound) {
        Handles::s_Color = Vector4(0.5f, 0.0f, 0.5f, 1.0f);
        Handles::drawBox(t->worldPosition(), t->worldRotation(), t->worldScale());
    }

    Handles::s_Color = Handles::s_Second = Handles::s_Normal;
    bool result = Handles::drawBillboard(t->worldPosition(), Vector2(0.5f), Engine::loadResource<Texture>(".embedded/postprocess.png"));

    return result;
}
#endif
