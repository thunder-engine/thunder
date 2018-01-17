#include "core/avariant.h"

AVariant::Data::Data() {
    PROFILE_FUNCTION()
    type    = AMetaType::INVALID;
    so      = nullptr;
}

AVariant::AVariant() {
    PROFILE_FUNCTION()

}

AVariant::AVariant(AMetaType::Type type) {
    PROFILE_FUNCTION()
    mData.type      = type;
}

AVariant::AVariant(bool value) {
    PROFILE_FUNCTION()
    *this   = fromValue<bool>(value);
}

AVariant::AVariant(int value) {
    PROFILE_FUNCTION()
    *this   = fromValue<int>(value);
}

AVariant::AVariant(double value) {
    PROFILE_FUNCTION()
    *this   = fromValue<double>(value);
}

AVariant::AVariant(const char *value) {
    PROFILE_FUNCTION()
    *this   = fromValue<string>(value);
}

AVariant::AVariant(const string &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<string>(value);
}

AVariant::AVariant(const AVariantMap &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<AVariantMap>(value);
}

AVariant::AVariant(const AVariantList &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<AVariantList>(value);
}

AVariant::AVariant(const AByteArray &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<AByteArray>(value);
}

AVariant::AVariant(const Vector2 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Vector2>(value);
}

AVariant::AVariant(const Vector3 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Vector3>(value);
}

AVariant::AVariant(const Vector4 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Vector4>(value);
}

AVariant::AVariant(const Quaternion &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Quaternion>(value);
}

AVariant::AVariant(const Matrix3 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Matrix3>(value);
}

AVariant::AVariant(const Matrix4 &value) {
    PROFILE_FUNCTION()
    *this   = fromValue<Matrix4>(value);
}

AVariant::AVariant(uint32_t type, void *copy) {
    PROFILE_FUNCTION()
    mData.type  = type;
    mData.so    = AMetaType::create(type, copy);
}

AVariant::~AVariant() {
    PROFILE_FUNCTION()
    clear();
}

AVariant::AVariant(const AVariant &value) {
    PROFILE_FUNCTION()
    mData.type  = value.mData.type;
    mData.so    = AMetaType::create(value.mData.type, value.mData.so);
}

AVariant &AVariant::operator=(const AVariant &value) {
    PROFILE_FUNCTION()
    clear();
    mData.type  = value.mData.type;
    mData.so    = AMetaType::create(value.mData.type, value.mData.so);
    return *this;
}

bool AVariant::operator==(const AVariant &right) const {
    PROFILE_FUNCTION()
    if(mData.type  == right.mData.type) {
        return AMetaType::compare(mData.so, right.mData.so, mData.type);
    }
    return false;
}

bool AVariant::operator!=(const AVariant &right) const {
    PROFILE_FUNCTION()
    return !(*this == right);
}

void AVariant::clear() {
    AMetaType::destroy(mData.type, mData.so);
    mData.type  = 0;
}

uint32_t AVariant::type() const {
    PROFILE_FUNCTION()
    return (mData.type < AMetaType::USERTYPE) ? mData.type : AMetaType::USERTYPE;
}

uint32_t AVariant::userType() const {
    PROFILE_FUNCTION()
    return mData.type;
}

void *AVariant::data() const {
    PROFILE_FUNCTION()
    return mData.so;
}

bool AVariant::isValid() const {
    PROFILE_FUNCTION()
    return (mData.type != AMetaType::INVALID && mData.so);
}

bool AVariant::canConvert(uint32_t type) const {
    PROFILE_FUNCTION()
    return AMetaType::hasConverter(mData.type, type);
}

// Conversion and getters
bool AVariant::toBool() const {
    PROFILE_FUNCTION()
    return value<bool>();
}

int AVariant::toInt() const {
    PROFILE_FUNCTION()
    return value<int>();
}

double AVariant::toDouble() const {
    PROFILE_FUNCTION()
    return value<double>();
}

const string AVariant::toString() const {
    PROFILE_FUNCTION()
    return value<string>();
}

const AVariantMap AVariant::toMap() const {
    PROFILE_FUNCTION()
    return value<AVariantMap>();
}

const AVariantList AVariant::toList() const {
    PROFILE_FUNCTION()
    AVariantList result = value<AVariantList>();
    result.push_front(static_cast<int>(mData.type));
    return result;
}

const AByteArray AVariant::toByteArray() const {
    PROFILE_FUNCTION()
    return value<AByteArray>();
}

const Vector2 AVariant::toVector2() const {
    PROFILE_FUNCTION()
    return value<Vector2>();
}

const Vector3 AVariant::toVector3() const {
    PROFILE_FUNCTION()
    return value<Vector3>();
}

const Vector4 AVariant::toVector4() const {
    PROFILE_FUNCTION()
    return value<Vector4>();
}

const Quaternion AVariant::toQuaternion() const {
    PROFILE_FUNCTION()
    return value<Quaternion>();
}

const Matrix3 AVariant::toMatrix3() const {
    PROFILE_FUNCTION()
    return value<Matrix3>();
}

const Matrix4 AVariant::toMatrix4() const {
    PROFILE_FUNCTION()
    return value<Matrix4>();
}
