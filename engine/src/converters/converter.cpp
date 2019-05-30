#include "converters/converter.h"

IConverterSettings::IConverterSettings() :
        mValid(false),
        mType(0),
        mCRC(0) {
}

uint32_t IConverterSettings::type() const {
    return mType;
}
void IConverterSettings::setType(uint32_t type) {
    mType   = type;
}

bool IConverterSettings::isValid() const {
    return mValid;
}
void IConverterSettings::setValid(bool valid) {
    mValid  = valid;
}

uint32_t IConverterSettings::crc() const {
    return mCRC;
}
void IConverterSettings::setCRC(uint32_t crc) {
    mCRC    = crc;
}

const char *IConverterSettings::source() const {
    return mSource.c_str();
}
void IConverterSettings::setSource(const char *source) {
    mSource = source;
}

const char *IConverterSettings::destination() const {
    return mDestination.c_str();
}
void IConverterSettings::setDestination(const char *destination) {
    mDestination    = destination;
}

const char *IConverterSettings::absoluteDestination() const {
    return mAbsoluteDestination.c_str();
}

void IConverterSettings::setAbsoluteDestination(const char *destination) {
    mAbsoluteDestination    = destination;
}

const QStringList IConverterSettings::subKeys() const {
    return mSubItems.keys();
}

QString IConverterSettings::subItem(const QString &key) const {
    return mSubItems.value(key);
}

int32_t IConverterSettings::subType(const QString &key) const {
    return mSubTypes.value(key);
}

void IConverterSettings::setSubItem(const QString &name, const QString &uuid, int32_t type) {
    if(!name.isEmpty() && !uuid.isEmpty()) {
        mSubItems[name] = uuid;
        mSubTypes[name] = type;
    }
}

IConverterSettings *IConverter::createSettings() const {
    return new IConverterSettings();
}
