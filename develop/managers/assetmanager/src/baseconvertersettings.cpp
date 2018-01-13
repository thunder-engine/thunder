#include "baseconvertersettings.h"

BaseConverterSettings::BaseConverterSettings(QObject *parent) :
    mValid(false),
    mType(0),
    mCRC(0),
    QObject(parent) {
}

uint8_t BaseConverterSettings::type() const {
    return mType;
}
void BaseConverterSettings::setType(uint8_t type) {
    mType   = type;
}

bool BaseConverterSettings::isValid() const {
    return mValid;
}
void BaseConverterSettings::setValid(bool valid) {
    mValid  = valid;
}

uint32_t BaseConverterSettings::crc() const {
    return mCRC;
}
void BaseConverterSettings::setCRC(uint32_t crc) {
    mCRC    = crc;
}

const char *BaseConverterSettings::source() const {
    return mSource.c_str();
}
void BaseConverterSettings::setSource(const char *source) {
    mSource = source;
}

const char *BaseConverterSettings::destination() const {
    return mDestination.c_str();
}
void BaseConverterSettings::setDestination(const char *destination) {
    mDestination    = destination;
}
