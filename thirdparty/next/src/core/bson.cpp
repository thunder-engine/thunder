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

#include "core/bson.h"

#include <streambuf>

#include <cstring>

void appendProperty(Variant &container, const Variant &data, const std::string &name) {
    switch(container.type()) {
        case MetaType::VARIANTLIST: {
            if(container.data() == nullptr) {
                container = VariantList();
            }
            VariantList &list = *(reinterpret_cast<VariantList *>(container.data()));
            list.push_back(data);
        } break;
        case MetaType::VARIANTMAP: {
            if(container.data() == nullptr) {
                container = VariantMap();
            }
            VariantMap &map = *(reinterpret_cast<VariantMap *>(container.data()));
            map[name] = data;
        } break;
        default: break;
    }
}

enum DataTypes {
    FLOAT      = 1,
    STRING,
    OBJECT,
    ARRAY,
    BINARY,
    BOOL       = 8,
    INT32      = 16,
    VECTOR2    = 128,
    VECTOR3,
    VECTOR4,
    MATRIX3,
    MATRIX4,
    QUATERNION
};

Variant parse(const ByteArray &data, uint32_t &offset, MetaType::Type type, bool first) {
    PROFILE_FUNCTION();
    Variant result(type);
    if(data.empty()) {
        return result;
    }

    uint32_t global = offset;

    uint32_t size;
    memcpy(&size, &data[offset], sizeof(uint32_t));
    if(offset + size > data.size()) {
        return Variant();
    }
    offset  += sizeof(uint32_t);

    while(offset < global + size) {
        uint8_t t = data[offset];
        offset++;

        std::string name;
        for(; offset < global + size; offset++) {
            if(data[offset] == 0) {
                offset++;
                break;
            }
            name.push_back(data[offset]);
        }

        if(global == 225) {
            global = 225;
        }

        switch(t) {
            case BOOL: {
                uint8_t value = data[offset];
                offset++;

                appendProperty(result, (value) ? true : false, name);
            } break;
            case STRING: {
                uint32_t length;
                memcpy(&length, &data[offset], sizeof(uint32_t));
                offset += sizeof(uint32_t);

                char *value = new char[length];
                memcpy(value, &data[offset], length);
                offset += length;

                appendProperty(result, value, name);
                delete []value;
            } break;
            case OBJECT: {
                int32_t length;
                memcpy(&length, &data[offset], sizeof(uint32_t));

                Variant container  = parse(data, offset, MetaType::VARIANTMAP, false);
                appendProperty(result, container, name);
            } break;
            case ARRAY: {
                int32_t length;
                memcpy(&length, &data[offset], sizeof(uint32_t));

                Variant container  = parse(data, offset, MetaType::VARIANTLIST, false);
                appendProperty(result, container, name);
            } break;
            case BINARY: {
                uint32_t length;
                memcpy(&length, &data[offset],  sizeof(uint32_t));
                offset += sizeof(uint32_t);
                uint8_t sub;
                memcpy(&sub, &data[offset],     sizeof(uint8_t));
                offset++;
                ByteArray value(data.begin() + offset, data.begin() + offset + length);

                appendProperty(result, value, name);
                offset += length;
            } break;
            case FLOAT: {
                float value;
                memcpy(&value, &data[offset], sizeof(float));
                offset += sizeof(float);

                appendProperty(result, value, name);
            } break;
            case INT32: {
                int32_t value;
                memcpy(&value, &data[offset], sizeof(uint32_t));
                offset += sizeof(uint32_t);

                appendProperty(result, value, name);
            } break;
            case VECTOR2: {
                Vector2 value;
                memcpy(&value, &data[offset], sizeof(Vector2));
                offset += sizeof(Vector2);

                appendProperty(result, value, name);
            } break;
            case VECTOR3: {
                Vector3 value;
                memcpy(&value, &data[offset], sizeof(Vector3));
                offset += sizeof(Vector3);

                appendProperty(result, value, name);
            } break;
            case VECTOR4: {
                Vector4 value;
                memcpy(&value, &data[offset], sizeof(Vector4));
                offset += sizeof(Vector4);

                appendProperty(result, value, name);
            } break;
            case MATRIX3: {
                Matrix3 value;
                memcpy(&value, &data[offset], sizeof(Matrix3));
                offset += sizeof(Matrix3);

                appendProperty(result, value, name);
            } break;
            case MATRIX4: {
                Matrix4 value;
                memcpy(&value, &data[offset], sizeof(Matrix4));
                offset += sizeof(Matrix4);

                appendProperty(result, value, name);
            } break;
            case QUATERNION: {
                Quaternion value;
                memcpy(&value, &data[offset], sizeof(Quaternion));
                offset += sizeof(Quaternion);

                appendProperty(result, value, name);
            } break;
            default: break;
        }

    }

    if(first && result.type() == MetaType::VARIANTLIST) {
        return result.value<VariantList>();
    }

    if(result.data() == nullptr) {
        return result;
    }

    return result;
}

uint8_t type(const Variant &data) {
    PROFILE_FUNCTION();
    uint8_t result;
    switch (data.type()) {
        case MetaType::BOOLEAN:     result  = BOOL; break;
        case MetaType::FLOAT:       result  = FLOAT; break;
        case MetaType::INTEGER:     result  = INT32; break;
        case MetaType::STRING:      result  = STRING; break;
        case MetaType::VARIANTMAP:  result  = OBJECT; break;
        case MetaType::BYTEARRAY:   result  = BINARY; break;
        case MetaType::VECTOR2:     result  = VECTOR2; break;
        case MetaType::VECTOR3:     result  = VECTOR3; break;
        case MetaType::VECTOR4:     result  = VECTOR4; break;
        case MetaType::MATRIX3:     result  = MATRIX3; break;
        case MetaType::MATRIX4:     result  = MATRIX4; break;
        case MetaType::QUATERNION:  result  = QUATERNION; break;
        default:                    result  = ARRAY; break;
    }
    return result;
}
/*!
    \class Bson
    \brief Binary JSON format parser.
    \since Next 1.0
    \inmodule Core

    This class implements Binary JSON parser with Variant based DOM structure input/output.
    It allows to serialize and deserialize object structures represented in Variant DOM structure.

    Example:
    \code
        VariantMap dictionary;
        dictionary["bool"]  = true;
        dictionary["str"]   = "string";
        dictionary["int"]   = 1;
        dictionary["float"] = 2.0f;

        ByteArray data = Bson::save(dictionary); // Serializing dictionary to binary format
        ....
        VariantMap result = Bson::load(data).toMap(); // Resotoring it back
    \endcode
*/

/*!
    \fn Variant Bson::load(const ByteArray &data, MetaType::Type type)

    Returns deserialized binary \a data as Variant based DOM structure with expected \a type of container (can be MetaType::VARIANTLIST or MetaType::VARIANTMAP).
*/
Variant Bson::load(const ByteArray &data, MetaType::Type type) {
    uint32_t offset = 0;
    return parse(data, offset, type, true);
}
/*!
    \fn ByteArray Bson::save(const Variant &data)

    Returns serialized \a data as binary buffer.
*/
ByteArray Bson::save(const Variant &data) {
    PROFILE_FUNCTION();
    ByteArray result;

    switch(data.type()) {
        case MetaType::BOOLEAN: {
            result.push_back( (data.toBool()) ? 0x01 : 0x00 );
        } break;
        case MetaType::STRING: {
            TString value = data.toString();
            uint32_t size = value.size() + 1;
            result.resize(size + sizeof(uint32_t));

            memcpy(&result[0], &size, sizeof(uint32_t));
            memcpy(&result[sizeof(uint32_t)], value.data(), size);
        } break;
        case MetaType::BYTEARRAY: {
            ByteArray value = data.toByteArray();
            uint32_t size   = value.size();
            result.resize(sizeof(uint32_t) + 1 + size);

            memcpy(&result[0], &size, sizeof(uint32_t));
            result[sizeof(uint32_t)] = '\x00';
            if(size) {
                memcpy(&result[sizeof(uint32_t) + 1], &value[0], size);
            }
        } break;
        case MetaType::VARIANTMAP: {
            uint32_t size   = sizeof(uint32_t);
            result.resize(size);
            uint32_t offset = size;
            uint32_t index  = 0;
            VariantMap map  = data.toMap();
            for(auto &it: map) {
                ByteArray element   = save(it.second);
                uint8_t t = type(it.second);
                size += element.size() + 1 + (it.first.size() + 1);
                result.resize(size);

                memcpy(&result[offset++], &t, 1);
                memcpy(&result[offset], it.first.data(), it.first.size() + 1);
                offset += it.first.size() + 1;
                memcpy(&result[offset], &element[0], element.size());
                offset += element.size();

                index++;
            }
            result.push_back(0x00);
            memcpy(&result[0], &(++size), sizeof(uint32_t));
        } break;
        case MetaType::FLOAT: {
            float value = data.value<float>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::INTEGER: {
            int32_t value = data.value<int32_t>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::VECTOR2: {
            Vector2 value = data.value<Vector2>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::VECTOR3: {
            Vector3 value = data.value<Vector3>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::VECTOR4: {
            Vector4 value = data.value<Vector4>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::MATRIX3: {
            Matrix3 value = data.value<Matrix3>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::MATRIX4: {
            Matrix4 value = data.value<Matrix4>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::QUATERNION: {
            Quaternion value = data.value<Quaternion>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        default: {
            uint32_t size   = sizeof(uint32_t);
            result.resize(size);
            uint32_t offset = size;
            uint32_t index  = 0;
            VariantList list   = data.toList();
            for(auto &it: list) {
                ByteArray element = save(it);
                std::string i = std::to_string(index);
                uint8_t t = type(it);
                size += element.size() + 1 + (i.size() + 1);
                result.resize(size);

                memcpy(&result[offset++], &t, 1);
                memcpy(&result[offset], i.c_str(), i.size() + 1);
                offset += i.size() + 1;
                memcpy(&result[offset], &element[0], element.size());
                offset += element.size();

                index++;
            }
            result.push_back(0x00);
            memcpy(&result[0], &(++size), sizeof(uint32_t));
        } break;
    }
    return result;
}
