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

#ifndef AVARIANT_H
#define AVARIANT_H

#include <map>
#include <list>
#include <vector>
#include <string>

#include "acommon.h"

#include "math/amath.h"
#include "ametatype.h"

using namespace std;

class AVariant;

typedef map<string, AVariant>   AVariantMap;
typedef list<AVariant>          AVariantList;
typedef vector<int8_t>          AByteArray;

class NEXT_LIBRARY_EXPORT AVariant {
public:
     struct NEXT_LIBRARY_EXPORT Data {
        Data                    ();

        uint32_t                type;
        void                   *so;
    };

public:
    AVariant                    ();
    AVariant                    (AMetaType::Type type);
    AVariant                    (bool value);
    AVariant                    (int value);
    AVariant                    (double value);
    AVariant                    (const char *value);
    AVariant                    (const string &value);
    AVariant                    (const AVariantMap &value);
    AVariant                    (const AVariantList &value);
    AVariant                    (const AByteArray &value);

    AVariant                    (const Vector2 &value);
    AVariant                    (const Vector3 &value);
    AVariant                    (const Vector4 &value);
    AVariant                    (const Quaternion &value);
    AVariant                    (const Matrix3 &value);
    AVariant                    (const Matrix4 &value);

    AVariant                    (uint32_t type, void *copy);

    ~AVariant                   ();

    AVariant                    (const AVariant &value);

    AVariant                   &operator=                   (const AVariant &value);

    bool                        operator==                  (const AVariant &right) const;
    bool                        operator!=                  (const AVariant &right) const;

    void                        clear                       ();

    uint32_t                    type                        () const;

    uint32_t                    userType                    () const;

    void                       *data                        () const;

    bool                        isValid                     () const;

    bool                        canConvert                  (uint32_t type) const;

    template<typename T>
    bool                        canConvert                  () const {
        return AVariant::canConvert(AMetaType::type<T>());
    }

    template<typename T>
    T                           value                       () const {
        uint32_t type   = AMetaType::type<T>();
        if(mData.so) {
            if(mData.type == type) {
                return *(reinterpret_cast<const T *>(mData.so));
            } else if(canConvert(type)) {
                T result;

                AMetaType::convert(mData.so, mData.type, &result, type);

                return result;
            }
        }
        return T();
    }

    template<typename T>
    static AVariant             fromValue                   (const T &value) {
        AVariant result;
        result.mData.type   = AMetaType::type<T>();
        if(result.mData.type != AMetaType::INVALID) {
            result.mData.so = AMetaType::create(result.mData.type, reinterpret_cast<const void *>(&value));
        }
        return result;
    }

    // Conversion and getters
    bool                        toBool                      () const;
    int                         toInt                       () const;
    double                      toDouble                    () const;
    const string                toString                    () const;

    const AVariantMap           toMap                       () const;
    const AVariantList          toList                      () const;
    const AByteArray            toByteArray                 () const;

    const Vector2               toVector2                   () const;
    const Vector3               toVector3                   () const;
    const Vector4               toVector4                   () const;
    const Quaternion            toQuaternion                () const;
    const Matrix3               toMatrix3                   () const;
    const Matrix4               toMatrix4                   () const;

protected:
    Data                        mData;

};

#endif // AVARIANT_H
