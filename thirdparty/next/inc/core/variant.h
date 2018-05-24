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

#include "common.h"

#include "math/amath.h"
#include "metatype.h"

using namespace std;

class Variant;

typedef map<string, Variant>    VariantMap;
typedef list<Variant>           VariantList;
typedef vector<int8_t>          ByteArray;

#if __ANDROID__
#include <sstream>
string to_string(auto v) {
    ostringstream ss;
    ss << v;
    return ss.str();
}
#endif

class NEXT_LIBRARY_EXPORT Variant {
public:
     struct NEXT_LIBRARY_EXPORT Data {
        Data                    ();

        uint32_t                type;
        void                   *so;
    };

public:
    Variant                     ();
    Variant                     (MetaType::Type type);
    Variant                     (bool value);
    Variant                     (int value);
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
        uint32_t type   = MetaType::type<T>();
        if(mData.so) {
            if(mData.type == type) {
                return *(reinterpret_cast<const T *>(mData.so));
            } else if(canConvert(type)) {
                T result;

                MetaType::convert(mData.so, mData.type, &result, type);

                return result;
            }
        }
        return T();
    }

    template<typename T>
    static Variant             fromValue                    (const T &value) {
        Variant result;
        result.mData.type   = MetaType::type<T>();
        if(result.mData.type != MetaType::INVALID) {
            result.mData.so = MetaType::create(result.mData.type, reinterpret_cast<const void *>(&value));
        }
        return result;
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
    Data                        mData;

};

#endif // VARIANT_H
