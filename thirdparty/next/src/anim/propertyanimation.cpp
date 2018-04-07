#include "anim/propertyanimation.h"

class PropertyAnimationPrivate {
public:
    PropertyAnimationPrivate() :
            m_pObject(nullptr),
            m_Property(nullptr) {

    }
    Object                 *m_pObject;
    MetaProperty            m_Property;
};

PropertyAnimation::PropertyAnimation() :
    p_ptr(new PropertyAnimationPrivate()) {

}

void PropertyAnimation::setTarget(Object *object, const char *property) {
    if(object) {
        const MetaObject *meta  = object->metaObject();
        int32_t index   = meta->indexOfProperty(property);
        if(index > -1) {
            p_ptr->m_pObject    = object;
            p_ptr->m_Property   = meta->property(index);
        }
    }
}

const Object *PropertyAnimation::target() const {
    return p_ptr->m_pObject;
}

const char *PropertyAnimation::targetProperty() const {
    if(p_ptr->m_Property.isValid()) {
        return p_ptr->m_Property.name();
    }
    return nullptr;
}

void PropertyAnimation::valueUpdated(const Variant &value) {
    VariantAnimation::valueUpdated(value);

    if(p_ptr->m_Property.isValid()) {
        p_ptr->m_Property.write(p_ptr->m_pObject, value);
    }
}
