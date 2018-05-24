#include "core/variant.h"

Variant::Data::Data() {
    PROFILE_FUNCTION()
    type    = MetaType::INVALID;
    so      = nullptr;
}

Variant::Variant() {
    PROFILE_FUNCTION()

}

Variant::Variant(MetaType::Type type) {
    PROFILE_FUNCTION()
    mData.type      = type;
}

Variant::Variant(bool value) {
    PROFILE_FUNCTION()
    *this   = fromValue<bool>(value);
}

Variant::Variant(int value) {
    PROFILE_FUNCTION()
    *this   = fromValue<int>(value);
}

Variant::Variant(float value) {
    PROFILE_FUNCTION()
    *this   = fromValue<float>(value);
}

Variant::Variant(const char *value) {
    PROFILE_FUNCTION()
    *this   = fromValue<string>(value);
}

Variant::Variant(const string &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<string>(value);
}

Variant::Variant(const VariantMap &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<VariantMap>(value);
}

Variant::Variant(const VariantList &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<VariantList>(value);
}

Variant::Variant(const ByteArray &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<ByteArray>(value);
}

Variant::Variant(const Vector2 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Vector2>(value);
}

Variant::Variant(const Vector3 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Vector3>(value);
}

Variant::Variant(const Vector4 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Vector4>(value);
}

Variant::Variant(const Quaternion &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Quaternion>(value);
}

Variant::Variant(const Matrix3 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Matrix3>(value);
}

Variant::Variant(const Matrix4 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Matrix4>(value);
}

Variant::Variant(uint32_t type, void *copy) {
    PROFILE_FUNCTION()
    mData.type  = type;
    mData.so    = MetaType::create(type, copy);
}

Variant::~Variant() {
    PROFILE_FUNCTION()
    clear();
}

Variant::Variant(const Variant &value) {
    PROFILE_FUNCTION()
    *this   = value;
}

Variant &Variant::operator=(const Variant &value) {
    PROFILE_FUNCTION()
    clear();
    mData.type  = value.mData.type;
    mData.so    = MetaType::create(value.mData.type, value.mData.so);
    return *this;
}

bool Variant::operator==(const Variant &right) const {
    PROFILE_FUNCTION()
    if(mData.type  == right.mData.type) {
        return MetaType::compare(mData.so, right.mData.so, mData.type);
    }
    return false;
}

bool Variant::operator!=(const Variant &right) const {
    PROFILE_FUNCTION()
    return !(*this == right);
}

void Variant::clear() {
    MetaType::destroy(mData.type, mData.so);
    mData.type  = 0;
    mData.so    = nullptr;
}

uint32_t Variant::type() const {
    PROFILE_FUNCTION()
    return (mData.type < MetaType::USERTYPE) ? mData.type : MetaType::USERTYPE;
}

uint32_t Variant::userType() const {
    PROFILE_FUNCTION()
    return mData.type;
}

void *Variant::data() const {
    PROFILE_FUNCTION()
    return mData.so;
}

bool Variant::isValid() const {
    PROFILE_FUNCTION()
    return (mData.type != MetaType::INVALID && mData.so);
}

bool Variant::canConvert(uint32_t type) const {
    PROFILE_FUNCTION()
    return MetaType::hasConverter(mData.type, type);
}

// Conversion and getters
bool Variant::toBool() const {
    PROFILE_FUNCTION()
    return value<bool>();
}

int Variant::toInt() const {
    PROFILE_FUNCTION()
    return value<int32_t>();
}

float Variant::toFloat() const {
    PROFILE_FUNCTION()
    return value<float>();
}

const string Variant::toString() const {
    PROFILE_FUNCTION()
    return value<string>();
}

const VariantMap Variant::toMap() const {
    PROFILE_FUNCTION()
    return value<VariantMap>();
}

const VariantList Variant::toList() const {
    PROFILE_FUNCTION()
    return value<VariantList>();
}

const ByteArray Variant::toByteArray() const {
    PROFILE_FUNCTION()
    return value<ByteArray>();
}

const Vector2 Variant::toVector2() const {
    PROFILE_FUNCTION()
    return value<Vector2>();
}

const Vector3 Variant::toVector3() const {
    PROFILE_FUNCTION()
    return value<Vector3>();
}

const Vector4 Variant::toVector4() const {
    PROFILE_FUNCTION()
    return value<Vector4>();
}

const Quaternion Variant::toQuaternion() const {
    PROFILE_FUNCTION()
    return value<Quaternion>();
}

const Matrix3 Variant::toMatrix3() const {
    PROFILE_FUNCTION()
    return value<Matrix3>();
}

const Matrix4 Variant::toMatrix4() const {
    PROFILE_FUNCTION()
    return value<Matrix4>();
}
