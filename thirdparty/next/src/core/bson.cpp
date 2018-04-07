#include "core/bson.h"

#include <streambuf>

Variant appendProperty(const Variant &container, const Variant &data, const string &name) {
    switch(container.type()) {
        case MetaType::VARIANTLIST: {
            VariantList list   = container.value<VariantList>();
            list.push_back(data);
            return list;
        } break;
        case MetaType::VARIANTMAP: {
            VariantMap map  = container.value<VariantMap>();
            map[name]   = data;
            return map;
        } break;
        default: break;
    }

    return container;
}

Variant Bson::load(const ByteArray &data, uint32_t &offset, MetaType::Type type, bool first) {
    PROFILE_FUNCTION()
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
        uint8_t t   = data[offset];
        offset++;

        string name;
        for(; offset < global + size; offset++) {
            if(data[offset] == 0) {
                offset++;
                break;
            }
            name.push_back(data[offset]);
        }

        switch(t) {
            case BOOL: {
                uint8_t value   = data[offset];
                offset++;

                result  = appendProperty(result, (value) ? true : false, name);
            } break;
            case STRING: {
                int32_t length;
                memcpy(&length, &data[offset], sizeof(uint32_t));
                offset += sizeof(uint32_t);

                char *value = new char[length];
                memcpy(value, &data[offset], length);
                offset += length;

                result  = appendProperty(result, value, name);
                delete []value;
            } break;
            case OBJECT:
            case ARRAY: {
                int32_t length;
                memcpy(&length, &data[offset], sizeof(uint32_t));
                Variant container  = load(data, offset, (t == ARRAY) ? MetaType::VARIANTLIST : MetaType::VARIANTMAP, false);
                result  = appendProperty(result, container, name);
            } break;
            case BINARY: {
                int32_t length;
                memcpy(&length, &data[offset],  sizeof(uint32_t));
                offset += sizeof(uint32_t);
                uint8_t sub;
                memcpy(&sub, &data[offset],     sizeof(uint8_t));
                offset++;
                ByteArray value(data.begin() + offset, data.begin() + offset + length);

                result  = appendProperty(result, value, name);
                offset += length;
            } break;
            case DOUBLE: {
                float value;
                memcpy(&value, &data[offset], sizeof(float));
                offset += sizeof(float);

                result  = appendProperty(result, value, name);
            } break;
            case INT32: {
                int32_t value;
                memcpy(&value, &data[offset], sizeof(uint32_t));
                offset += sizeof(uint32_t);

                result  = appendProperty(result, value, name);
            } break;
            case VECTOR2: {
                Vector2 value;
                memcpy(&value, &data[offset], sizeof(Vector2));
                offset += sizeof(Vector2);

                result  = appendProperty(result, value, name);
            } break;
            case VECTOR3: {
                Vector3 value;
                memcpy(&value, &data[offset], sizeof(Vector3));
                offset += sizeof(Vector3);

                result  = appendProperty(result, value, name);
            } break;
            case VECTOR4: {
                Vector4 value;
                memcpy(&value, &data[offset], sizeof(Vector4));
                offset += sizeof(Vector4);

                result  = appendProperty(result, value, name);
            } break;
            case MATRIX3: {
                Matrix3 value;
                memcpy(&value, &data[offset], sizeof(Matrix3));
                offset += sizeof(Matrix3);

                result  = appendProperty(result, value, name);
            } break;
            case MATRIX4: {
                Matrix4 value;
                memcpy(&value, &data[offset], sizeof(Matrix4));
                offset += sizeof(Matrix4);

                result  = appendProperty(result, value, name);
            } break;
            case QUATERNION: {
                Quaternion value;
                memcpy(&value, &data[offset], sizeof(Quaternion));
                offset += sizeof(Quaternion);

                result  = appendProperty(result, value, name);
            } break;
            default: break;
        }

    }

    if(first && result.type() == MetaType::VARIANTLIST) {
        return result.value<VariantList>();
    }
    return result;
}

ByteArray Bson::save(const Variant &data) {
    PROFILE_FUNCTION()
    ByteArray result;

    switch(data.type()) {
        case MetaType::BOOLEAN: {
            result.push_back( (data.toBool()) ? 0x01 : 0x00 );
        } break;
        case MetaType::STRING: {
            string value    = data.toString();
            uint32_t size   = value.size() + 1;
            result.resize(size + sizeof(uint32_t));

            memcpy(&result[0], &size, sizeof(uint32_t));
            memcpy(&result[sizeof(uint32_t)], value.c_str(), size);
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
                uint8_t t   = type(it.second);
                size       += element.size() + 1 + (it.first.size() + 1);
                result.resize(size);

                memcpy(&result[offset++], &t, 1);
                memcpy(&result[offset], it.first.c_str(), it.first.size() + 1);
                offset     += it.first.size() + 1;
                memcpy(&result[offset], &element[0], element.size());
                offset     += element.size();

                index++;
            }
            result.push_back(0x00);
            memcpy(&result[0], &(++size), sizeof(uint32_t));
        } break;
        case MetaType::FLOAT: {
            float value     = data.value<float>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::INTEGER: {
            int32_t value   = data.value<int32_t>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::VECTOR2: {
            Vector2 value   = data.value<Vector2>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::VECTOR3: {
            Vector3 value   = data.value<Vector3>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::VECTOR4: {
            Vector4 value   = data.value<Vector4>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::MATRIX3: {
            Matrix3 value   = data.value<Matrix3>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::MATRIX4: {
            Matrix4 value   = data.value<Matrix4>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        case MetaType::QUATERNION: {
            Quaternion value    = data.value<Quaternion>();
            result.assign( reinterpret_cast<char *>( &value ), reinterpret_cast<char *>( &value ) + sizeof( value ) );
        } break;
        default: {
            uint32_t size   = sizeof(uint32_t);
            result.resize(size);
            uint32_t offset = size;
            uint32_t index  = 0;
            VariantList list   = data.toList();
            for(auto &it: list) {
                ByteArray element   = save(it);
                string i    = to_string(index);
                uint8_t t   = type(it);
                size       += element.size() + 1 + (i.size() + 1);
                result.resize(size);

                memcpy(&result[offset++], &t, 1);
                memcpy(&result[offset], i.c_str(), i.size() + 1);
                offset     += i.size() + 1;
                memcpy(&result[offset], &element[0], element.size());
                offset     += element.size();

                index++;
            }
            result.push_back(0x00);
            memcpy(&result[0], &(++size), sizeof(uint32_t));
        } break;
    }
    return result;
}

uint8_t Bson::type(const Variant &data) {
    PROFILE_FUNCTION()
    uint8_t result;
    switch (data.type()) {
        case MetaType::INVALID:     result  = NONE; break;
        case MetaType::BOOLEAN:     result  = BOOL; break;
        case MetaType::FLOAT:       result  = DOUBLE; break;
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
