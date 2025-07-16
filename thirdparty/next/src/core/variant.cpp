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

    Copyright: 2008-2025 Evgeniy Prikazchikov
*/

#include "core/variant.h"

Variant::SharedPrivate::SharedPrivate(void *value) :
        ptr(value),
        ref(1) {

}

Variant::Data::Data() :
        is_shared(false),
        type(MetaType::INVALID) {

    ptr = nullptr;
}
/*!
    \class Variant
    \brief Variant represents union value in Next library and work with most common data types.
    \since Next 1.0
    \inmodule Core

    Variant can contain values with common data types and return information about this types.
    Also Variant can convert cantained values to another data types using MetaType::convert function.
    Example:
    \code
        Variant variant; // This variant invalid for now
        variant     = Variant(true); // Now varinat contain boolean true value
        variant     = Variant(42); // Now varinat contain integer 42 value
        string str  = variant.toString() + " is the answer for everything"; // Now string contain string "42 is the answer for everything" value
    \endcode

    Object based classes can be automatically registered in meta type system to be using as Variant objects.
    Example:
    \code
        MyObject::registerClassFactory();
    \endcode

    And then:
    \code
        MyObject *origin = new MyObject;
        Variant variant = Variant::fromValue(origin);
        ....
        MyObject *object = variant.value<MyObject *>();
    \endcode
*/
/*!
    \fn template<typename T> T Variant::value() const

    Returns contained value which cast or converted to type T.

    Returns default T value if invalid variant or variant can not be converted to type T.

    \sa fromValue, canConvert, MetaType::convert
*/
/*!
    \fn template<typename T> static Variant Variant::fromValue(const T &value)

    Returns the variant containing provided \a value.

    Returns an invalid variant if unknown \a value type.

    \sa value, canConvert, MetaType::convert
*/
/*!
    \fn template<typename T> bool Variant::canConvert() const

    Returns the possibility of conversion for this variant to type T.

    \sa value, MetaType::convert
*/
/*!
    Constructs an invalid variant.
*/
Variant::Variant() {
    PROFILE_FUNCTION();

}
/*!
    Constructs an uninitialized variant of \a type.
*/
Variant::Variant(MetaType::Type type) {
    PROFILE_FUNCTION();
    m_data.type = type;
}
/*!
    Constructs a new variant with a boolean \a value.
*/
Variant::Variant(bool value) {
    PROFILE_FUNCTION();
    m_data.type = MetaType::BOOLEAN;
    m_data.b = value;
}
/*!
    Constructs a new variant with an integer \a value.
*/
Variant::Variant(int value) {
    PROFILE_FUNCTION();
    m_data.type = MetaType::INTEGER;
    m_data.i = value;
}

/*!
    Constructs a new variant with an integer \a value.
*/
Variant::Variant(unsigned int value) {
    PROFILE_FUNCTION();
    m_data.type = MetaType::INTEGER;
    m_data.i = value;
}
/*!
    Constructs a new variant with a floating point \a value.
*/
Variant::Variant(float value) {
    PROFILE_FUNCTION();
    m_data.type = MetaType::FLOAT;
    m_data.f = value;
}
/*!
    Constructs a new variant with a string \a value.
*/
Variant::Variant(const char *value) {
    PROFILE_FUNCTION();
    *this = fromValue<String>(value);
}
/*!
    Constructs a new variant with a string \a value.
*/
Variant::Variant(const String &value) {
    PROFILE_FUNCTION();
    *this = fromValue<String>(value);
}
/*!
    Constructs a new variant with a map of variants \a value.
*/
Variant::Variant(const VariantMap &value) {
    PROFILE_FUNCTION();
    *this = fromValue<VariantMap>(value);
}
/*!
    Constructs a new variant with a list of variants \a value.
*/
Variant::Variant(const VariantList &value) {
    PROFILE_FUNCTION();
    *this = fromValue<VariantList>(value);
}
/*!
    Constructs a new variant with a ByteArray \a value.
*/
Variant::Variant(const ByteArray &value) {
    PROFILE_FUNCTION();
    *this = fromValue<ByteArray>(value);
}
/*!
    Constructs a new variant with a Vector2 \a value.
*/
Variant::Variant(const Vector2 &value) {
    PROFILE_FUNCTION();
    *this = fromValue<Vector2>(value);
}
/*!
    Constructs a new variant with a Vector3 \a value.
*/
Variant::Variant(const Vector3 &value) {
    PROFILE_FUNCTION();
    *this = fromValue<Vector3>(value);
}
/*!
    Constructs a new variant with a Vector4 \a value.
*/
Variant::Variant(const Vector4 &value) {
    PROFILE_FUNCTION();
    *this = fromValue<Vector4>(value);
}
/*!
    Constructs a new variant with a Quaternion \a value.
*/
Variant::Variant(const Quaternion &value) {
    PROFILE_FUNCTION();
    *this = fromValue<Quaternion>(value);
}
/*!
    Constructs a new variant with a Matrix3 \a value.
*/
Variant::Variant(const Matrix3 &value) {
    PROFILE_FUNCTION();
    *this = fromValue<Matrix3>(value);
}
/*!
    Constructs a new variant with a Matrix4 \a value.
*/
Variant::Variant(const Matrix4 &value) {
    PROFILE_FUNCTION();
    *this = fromValue<Matrix4>(value);
}
/*!
    Constructs a new variant of \a type and initialized with \a copy value.
*/
Variant::Variant(uint32_t type, void *copy) {
    PROFILE_FUNCTION();
    m_data.type = type;

    if(type == MetaType::INVALID) {
        return;
    }
    switch(type) {
        case MetaType::BOOLEAN: m_data.b = *reinterpret_cast<bool *>(copy); break;
        case MetaType::INTEGER: m_data.i = *reinterpret_cast<int *>(copy); break;
        case MetaType::FLOAT: m_data.f = *reinterpret_cast<float *>(copy); break;
        default: m_data.ptr = MetaType::create(type, copy); break;
    }
}

Variant::~Variant() {
    PROFILE_FUNCTION();
    clear();
}
/*!
    Constructs a copy of variant \a value.
*/
Variant::Variant(const Variant &value) {
    PROFILE_FUNCTION();
    *this = value;
}
/*!
    Assigns the \a value of the variant to this variant.
*/
Variant &Variant::operator=(const Variant &value) {
    PROFILE_FUNCTION();
    clear();
    m_data.type  = value.m_data.type;
    if(m_data.type < MetaType::STRING) {
        m_data.ptr = value.m_data.ptr;
    } else {
        if(value.m_data.is_shared) {
            m_data = value.m_data;
            ++m_data.shared->ref;
        } else { // Make shared
            m_data.is_shared = true;
            if(value.m_data.ptr == nullptr) {
                m_data.is_shared = true;
            }
            if(value.m_data.ptr == nullptr) {
                value.m_data.ptr = MetaType::create(m_data.type, value.m_data.ptr);
            }
            m_data.shared = new Variant::SharedPrivate(value.m_data.ptr);

            value.m_data.is_shared = true;
            value.m_data.shared = m_data.shared;
            ++value.m_data.shared->ref;
        }
    }
    return *this;
}
/*!
    Compares a this variant with variant \a right value.
    Returns true if variants are equal; otherwise returns false.
*/
bool Variant::operator==(const Variant &right) const {
    PROFILE_FUNCTION();
    if(m_data.type == right.m_data.type) {
        if(m_data.type < MetaType::STRING) {
            return MetaType::compare(&m_data.ptr, &right.m_data.ptr, m_data.type);
        } else {
            return MetaType::compare(data(), right.data(), m_data.type);
        }
    }
    return false;
}
/*!
    Compares a this variant with variant \a right value.
    Returns true if variants are NOT equal; otherwise returns false.
*/
bool Variant::operator!=(const Variant &right) const {
    PROFILE_FUNCTION();
    return !(*this == right);
}
/*!
    Frees used resources and make this variant an invalid.
*/
void Variant::clear() {
    if(m_data.type >= MetaType::STRING) {
        if(m_data.is_shared) {
            --m_data.shared->ref;
            if(m_data.shared->ref == 0) {
                MetaType::destroy(m_data.type, m_data.shared->ptr);
            }
        } else {
            MetaType::destroy(m_data.type, m_data.ptr);
        }
    }
    m_data.type = 0;
    m_data.ptr  = nullptr;
}
/*!
    Returns type of variant value.
    \note If type of variant is user defined then fonction return MetaType::USERTYPE. To get the real type id use userType.

    \sa userType
*/
uint32_t Variant::type() const {
    PROFILE_FUNCTION();
    return (m_data.type < MetaType::USERTYPE) ? m_data.type : MetaType::USERTYPE;
}
/*!
    Returns user type of variant value.

    \sa type
*/
uint32_t Variant::userType() const {
    PROFILE_FUNCTION();
    return m_data.type;
}
/*!
    Returns pure pointer to value data.
*/
void *Variant::data() const {
    PROFILE_FUNCTION();
    if(m_data.is_shared) {
        return m_data.shared->ptr;
    }
    if(m_data.type < MetaType::STRING) {
        return &m_data.b;
    }
    return m_data.ptr;
}
/*!
    Returns true if variant value is valid; otherwise return false.
*/
bool Variant::isValid() const {
    PROFILE_FUNCTION();
    return (m_data.type != MetaType::INVALID);
}
/*!
    Returns true if variant converted to a \a type; otherwise return false.
*/
bool Variant::canConvert(uint32_t type) const {
    PROFILE_FUNCTION();
    return MetaType::hasConverter(m_data.type, type);
}

// Conversion and getters
/*!
    Returns variant as a bool value if variant has a type MetaType::BOOLEAN.
    Otherwise it tries to convert existing value to a bool.

    \sa value, canConvert, MetaType::convert
*/
bool Variant::toBool() const {
    PROFILE_FUNCTION();
    return value<bool>();
}
/*!
    Returns variant as an integer value if variant has a type MetaType::INTEGER.
    Otherwise it tries to convert existing value to an integer.

    \sa value, canConvert, MetaType::convert
*/
int Variant::toInt() const {
    PROFILE_FUNCTION();
    return value<int32_t>();
}
/*!
    Returns variant as a float value if variant has a type MetaType::FLOAT.
    Otherwise it tries to convert existing value to a float.

    \sa value, canConvert, MetaType::convert
*/
float Variant::toFloat() const {
    PROFILE_FUNCTION();
    return value<float>();
}
/*!
    Returns variant as a string value if variant has a type MetaType::STRING.
    Otherwise it tries to convert existing value to a string.

    \sa value, canConvert, MetaType::convert
*/
const String Variant::toString() const {
    PROFILE_FUNCTION();
    return value<String>();
}
/*!
    Returns variant as a variant map value if variant has a type MetaType::VARIANTMAP.
    Otherwise it tries to convert existing value to a variant map.

    \sa value, canConvert, MetaType::convert
*/
const VariantMap Variant::toMap() const {
    PROFILE_FUNCTION();
    return value<VariantMap>();
}
/*!
    Returns variant as a variant list value if variant has a type MetaType::VARIANTLIST.
    Otherwise it tries to convert existing value to a variant list.

    \sa value, canConvert, MetaType::convert
*/
const VariantList Variant::toList() const {
    PROFILE_FUNCTION();
    return value<VariantList>();
}
/*!
    Returns variant as a ByteArray value if variant has a type MetaType::BYTEARRAY.
    Otherwise it tries to convert existing value to a ByteArray.

    \sa value, canConvert, MetaType::convert
*/
const ByteArray Variant::toByteArray() const {
    PROFILE_FUNCTION();
    return value<ByteArray>();
}
/*!
    Returns variant as a Vector2 value if variant has a type MetaType::VECTOR2.
    Otherwise it tries to convert existing value to a Vector2.

    \sa value, canConvert, MetaType::convert
*/
const Vector2 Variant::toVector2() const {
    PROFILE_FUNCTION();
    return value<Vector2>();
}
/*!
    Returns variant as a Vector3 value if variant has a type MetaType::VECTOR3.
    Otherwise it tries to convert existing value to a Vector3.

    \sa value, canConvert, MetaType::convert
*/
const Vector3 Variant::toVector3() const {
    PROFILE_FUNCTION();
    return value<Vector3>();
}
/*!
    Returns variant as a Vector4 value if variant has a type MetaType::VECTOR4.
    Otherwise it tries to convert existing value to a Vector4.

    \sa value, canConvert, MetaType::convert
*/
const Vector4 Variant::toVector4() const {
    PROFILE_FUNCTION();
    return value<Vector4>();
}
/*!
    Returns variant as a Quaternion value if variant has a type MetaType::QUATERNION.
    Otherwise it tries to convert existing value to a Quaternion.

    \sa value, canConvert, MetaType::convert
*/
const Quaternion Variant::toQuaternion() const {
    PROFILE_FUNCTION();
    return value<Quaternion>();
}
/*!
    Returns variant as a Matrix3 value if variant has a type MetaType::MATRIX3.
    Otherwise it tries to convert existing value to a Matrix3.

    \sa value, canConvert, MetaType::convert
*/
const Matrix3 Variant::toMatrix3() const {
    PROFILE_FUNCTION();
    return value<Matrix3>();
}
/*!
    Returns variant as a Matrix4 value if variant has a type MetaType::MATRIX4.
    Otherwise it tries to convert existing value to a Matrix4.

    \sa value, canConvert, MetaType::convert
*/
const Matrix4 Variant::toMatrix4() const {
    PROFILE_FUNCTION();
    return value<Matrix4>();
}
