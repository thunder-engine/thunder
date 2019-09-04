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
/*!
    \class PropertyAnimation
    \brief The PropertyAnimation class animates one particular Object property.
    \since Next 1.0
    \inmodule Anim

    PropertyAnimation interpolates animated property between key-frames.

    \code
        PropertyAnimation anim;
        anim.setLoopDuration(1000);

        MyObject object;
        anim.setTarget(&object, "position");
        anim.setKeyFrames({ KeyFrame(0.0f, Vector2(0.0f, 0.0f)),
                            KeyFrame(1.0f, Vector2(1.0f, 2.0f)) });

        anim.start();
    \endcode
*/
/*!
    Constructs PropertyAnimation object.
*/
PropertyAnimation::PropertyAnimation() :
    p_ptr(new PropertyAnimationPrivate()) {

}

PropertyAnimation::~PropertyAnimation() {
    if(p_ptr->m_Property.isValid()) {
        p_ptr->m_Property.write(p_ptr->m_pObject, defaultValue());
    }

    delete p_ptr;
}
/*!
    Sets the new animated \a property of the \a object.
*/
void PropertyAnimation::setTarget(Object *object, const char *property) {
    if(p_ptr->m_Property.isValid()) {
        p_ptr->m_Property.write(p_ptr->m_pObject, defaultValue());
    }

    if(object) {
        const MetaObject *meta  = object->metaObject();
        int32_t index   = meta->indexOfProperty(property);
        if(index > -1) {
            p_ptr->m_pObject    = object;
            p_ptr->m_Property   = meta->property(index);
            setDefaultValue(p_ptr->m_Property.read(p_ptr->m_pObject));
        }
    }
}
/*!
    Returns the root object of the animated property.
*/
const Object *PropertyAnimation::target() const {
    return p_ptr->m_pObject;
}
/*!
    Returns the name of animates property of the object.
*/
const char *PropertyAnimation::targetProperty() const {
    if(p_ptr->m_Property.isValid()) {
        return p_ptr->m_Property.name();
    }
    return nullptr;
}
/*!
    \overload
    Sets the new current \a value for the animated Variant. And updates animated property of the object.
*/
void PropertyAnimation::setCurrentValue(const Variant &value) {
    BlenderAnimation::setCurrentValue(value);

    if(p_ptr->m_Property.isValid()) {
        p_ptr->m_Property.write(p_ptr->m_pObject, value);
    }
}

void PropertyAnimation::setValid(bool valid) {
    if(!valid && p_ptr->m_Property.isValid()) {
        p_ptr->m_Property.write(p_ptr->m_pObject, defaultValue());
    }

    BlenderAnimation::setValid(valid);
}
