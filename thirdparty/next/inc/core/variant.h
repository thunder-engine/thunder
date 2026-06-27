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

#ifndef VARIANT_H
#define VARIANT_H

#include <map>
#include <list>
#include <vector>
#include <memory>
#include <cstring>

#include <global.h>
#include <amath.h>
#include <astring.h>
#include <metatype.h>

class Variant;

typedef std::map<TString, Variant> VariantMap;
typedef std::list<Variant> VariantList;

class NEXT_LIBRARY_EXPORT Variant {
public:
    class NEXT_LIBRARY_EXPORT SharedPrivate {
    public:
        explicit SharedPrivate(void *value);

        void *ptr;
        uint32_t ref;
    };

    struct NEXT_LIBRARY_EXPORT Data {
        Data();

        bool is_shared;
        uint32_t type;
        union {
            float f;
            int i;
            bool b;
            void *ptr;
            SharedPrivate *shared;
        };
    };

public:
    Variant();
    Variant(MetaType::Type type);
    Variant(bool value);
    Variant(int value);
    Variant(unsigned int value);
    Variant(float value);
    Variant(const char *value);
    Variant(const TString &value);
    Variant(const VariantMap &value);
    Variant(const VariantList &value);
    Variant(const ByteArray &value);

    Variant(const Vector2 &value);
    Variant(const Vector3 &value);
    Variant(const Vector4 &value);
    Variant(const Quaternion &value);
    Variant(const Matrix3 &value);
    Variant(const Matrix4 &value);

    Variant(uint32_t type, void *copy);

    ~Variant();

    Variant(const Variant &value);

    Variant &operator=(const Variant &value);

    bool operator==(const Variant &right) const;
    bool operator!=(const Variant &right) const;

    void clear();

    uint32_t type() const;

    uint32_t userType() const;

    void *data() const;

    bool isValid() const;

    bool canConvert(uint32_t type) const;

    template<typename T>
    bool canConvert() const {
        return Variant::canConvert(MetaType::type<T>());
    }

    bool convert(uint32_t type);

    template<typename T>
    T value() const {
        uint32_t type = MetaType::type<T>();

        if(m_data.type < MetaType::STRING) {
            if(m_data.type == type) {
                if constexpr (std::is_trivially_copyable_v<T>) {
                    T result;
                    memcpy(&result, &m_data.ptr, sizeof(T));
                    return result;
                } else {
                    return *reinterpret_cast<const T *>(&m_data.ptr);
                }
            } else if(canConvert(type)) {
                T result;
                MetaType::convert(&m_data.ptr, m_data.type, &result, type);
                return result;
            }
        } else {
            if(m_data.ptr) {
                if(m_data.type == type) {
                    if(m_data.is_shared) {
                        return *reinterpret_cast<const T *>(m_data.shared->ptr);
                    } else {
                        return *reinterpret_cast<const T *>(m_data.ptr);
                    }
                } else if(canConvert(type)) {
                    T result;
                    if(m_data.is_shared) {
                        MetaType::convert(m_data.shared->ptr, m_data.type, &result, type);
                    } else {
                        MetaType::convert(m_data.ptr, m_data.type, &result, type);
                    }
                    return result;
                }
            }
        }
        return T();
    }

    template<typename T>
    static Variant fromValue(const T &value) {
        uint32_t type = MetaType::type<T>();
        if(type != MetaType::INVALID) {
            if(type < MetaType::STRING) {
                return Variant(value);
            }

            Variant result;
            result.m_data.type = type;
            result.m_data.ptr = MetaType::create(result.m_data.type, reinterpret_cast<const void *>(&value));
            return result;
        }
        return Variant();
    }

    // Conversion and getters
    bool toBool() const;
    int toInt() const;
    float toFloat() const;
    const TString toString() const;

    const VariantMap toMap() const;
    const VariantList toList() const;
    const ByteArray toByteArray() const;

    const Vector2 toVector2() const;
    const Vector3 toVector3() const;
    const Vector4 toVector4() const;
    const Quaternion toQuaternion() const;
    const Matrix3 toMatrix3() const;
    const Matrix4 toMatrix4() const;

protected:
    mutable Data m_data;

};

#endif // VARIANT_H
