#include "converters/converter.h"

IConverterSettings::IConverterSettings() :
        mValid(false),
        mType(0),
        mCRC(0) {
}

uint8_t IConverterSettings::type() const {
    return mType;
}
void IConverterSettings::setType(uint8_t type) {
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

uint32_t IConverterSettings::subItemsCount() const {
    return mSubItems.size();
}

const char *IConverterSettings::subItem(uint32_t index) const {
    if(index < mSubItems.size()) {
        return mSubItems[index].c_str();
    }
    return nullptr;
}

void IConverterSettings::addSubItem(const char *item) {
    if(item) {
        mSubItems.push_back(item);
    }
}

IConverterSettings *IConverter::createSettings() const {
    return new IConverterSettings();
}
