/*
    This file is part of Thunder Next.

    Thunder Next is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Thunder Next is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Thunder Next.  If not, see <http://www.gnu.org/licenses/>.

    Copyright: 2008-2014 Evgeniy Prikazchikov
*/

#ifndef VARIANT_H
#define VARIANT_H

#include <map>
#include <list>
#include <vector>
#include <string>
#include <memory>

#include <global.h>

#include "amath.h"
#include "metatype.h"

using namespace std;

class Variant;

typedef map<string, Variant>    VariantMap;
typedef list<Variant>           VariantList;
typedef vector<int8_t>          ByteArray;

#ifdef __ANDROID__
#include <sstream>
string to_string(auto v) {
    ostringstream ss;
    ss << v;
    return ss.str();
}
#endif

class NEXT_LIBRARY_EXPORT Variant {
public:
    class NEXT_LIBRARY_EXPORT SharedPrivate {
    public:
        explicit SharedPrivate (void *value);

        void                   *ptr;
        uint32_t                ref;
    };

    struct NEXT_LIBRARY_EXPORT Data {
        Data                    ();

        bool                    is_shared;
        uint32_t                type;
        union {
            float               f;
            int                 i;
            bool                b;
            void               *ptr;
            SharedPrivate      *shared;
        };
    };

public:
    Variant                     ();
    Variant                     (MetaType::Type type);
    Variant                     (bool value);
    Variant                     (int value);
    Variant                     (unsigned int value);
    Variant                     (float value);
    Variant                     (const char *value);
    Variant                     (const string &value);
    Variant                     (const VariantMap &value);
    Variant                     (const VariantList &value);
    Variant                     (const ByteArray &value);

    Variant                     (const Vector2 &value);
    Variant                     (const Vector3 &value);
    Variant                     (const Vector4 &value);
    Variant                     (const Quaternion &value);
    Variant                     (const Matrix3 &value);
    Variant                     (const Matrix4 &value);

    Variant                     (uint32_t type, void *copy);

    ~Variant                    ();

    Variant                     (const Variant &value);

    Variant                    &operator=                   (const Variant &value);

    bool                        operator==                  (const Variant &right) const;
    bool                        operator!=                  (const Variant &right) const;

    void                        clear                       ();

    uint32_t                    type                        () const;

    uint32_t                    userType                    () const;

    void                       *data                        () const;

    bool                        isValid                     () const;

    bool                        canConvert                  (uint32_t type) const;

    template<typename T>
    bool                        canConvert                  () const {
        return Variant::canConvert(MetaType::type<T>());
    }

    template<typename T>
    T                           value                       () const {
        uint32_t type = MetaType::type<T>();

        if(mData.type < MetaType::STRING) {
            if(mData.type == type) {
                return *reinterpret_cast<const T *>(&mData.ptr);
            } else if(canConvert(type)) {
                T result;
                MetaType::convert(&mData.ptr, mData.type, &result, type);
                return result;
            }
        } else {
            if(mData.ptr) {
                if(mData.type == type) {
                    if(mData.is_shared) {
                        T result = *reinterpret_cast<const T *>(mData.shared->ptr);
                        return *reinterpret_cast<const T *>(mData.shared->ptr);
                    } else {
                        return *reinterpret_cast<const T *>(mData.ptr);
                    }
                } else if(canConvert(type)) {
                    T result;
                    if(mData.is_shared) {
                        MetaType::convert(mData.shared->ptr, mData.type, &result, type);
                    } else {
                        MetaType::convert(mData.ptr, mData.type, &result, type);
                    }
                    return result;
                }
            }
        }
        return T();
    }

    template<typename T>
    static Variant             fromValue                   (const T &value) {
        uint32_t type = MetaType::type<T>();
        if(type != MetaType::INVALID) {
            if(type < MetaType::STRING) {
                return Variant(value);
            }

            Variant result;
            result.mData.type = type;
            result.mData.ptr = MetaType::create(result.mData.type, reinterpret_cast<const void *>(&value));
            return result;
        }
        return Variant();
    }

    // Conversion and getters
    bool                        toBool                      () const;
    int                         toInt                       () const;
    float                       toFloat                     () const;
    const string                toString                    () const;

    const VariantMap            toMap                       () const;
    const VariantList           toList                      () const;
    const ByteArray             toByteArray                 () const;

    const Vector2               toVector2                   () const;
    const Vector3               toVector3                   () const;
    const Vector4               toVector4                   () const;
    const Quaternion            toQuaternion                () const;
    const Matrix3               toMatrix3                   () const;
    const Matrix4               toMatrix4                   () const;

protected:
    mutable Data                mData;

};

#endif // VARIANT_H
