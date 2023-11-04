/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2023 Evgeniy Prikazchikov
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
