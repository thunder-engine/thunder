/*
    This file is part of Thunder Next.

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

#include "anim/propertyanimation.h"

/*!
    \class PropertyAnimation
    \brief The PropertyAnimation class animates one particular Object property.
    \since Next 1.0
    \inmodule Animation

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

PropertyAnimation::PropertyAnimation() :
    m_object(nullptr),
    m_property(nullptr) {

}

PropertyAnimation::~PropertyAnimation() {
    if(m_property.isValid()) {
        m_property.write(m_object, m_default);
    }
}
/*!
    Sets the new animated \a property of the \a object.
*/
void PropertyAnimation::setTarget(Object *object, const char *property) {
    setCurrentValue(m_default);

    if(object) {
        const MetaObject *meta = object->metaObject();
        int32_t index = meta->indexOfProperty(property);
        if(index > -1) {
            m_object = object;
            m_property = meta->property(index);
            m_default = m_property.read(m_object);
            setCurrentValue(m_default);
        }
    }
}
/*!
    Returns the default value of the animated property.
*/
Variant PropertyAnimation::defaultValue() const {
    return m_default;
}
/*!
    Returns the root object of the animated property.
*/
const Object *PropertyAnimation::target() const {
    return m_object;
}
/*!
    Returns the name of animates property of the object.
*/
const char *PropertyAnimation::targetProperty() const {
    if(m_property.isValid()) {
        return m_property.name();
    }
    return nullptr;
}
/*!
    \overload
    Sets the new current \a value for the animated Variant. And updates animated property of the object.
*/
void PropertyAnimation::setCurrentValue(const Variant &value) {
    VariantAnimation::setCurrentValue(value);

    if(m_property.isValid()) {
        m_property.write(m_object, value);
    }
}
/*!
    \overload
    Sets the \a valid state of animation. The invalid animations will not affect anything.
*/
void PropertyAnimation::setValid(bool valid) {
    if(!valid && m_property.isValid()) {
        m_property.write(m_object, m_default);
    }

    VariantAnimation::setValid(valid);
}
