#include "converters/converter.h"

IConverterSettings::IConverterSettings() :
        m_Valid(false),
        m_Type(0),
        m_Version(0),
        m_CurrentVersion(0),
        m_CRC(0) {
}

uint32_t IConverterSettings::type() const {
    return m_Type;
}
void IConverterSettings::setType(uint32_t type) {
    m_Type = type;
}

bool IConverterSettings::isValid() const {
    return m_Valid;
}
void IConverterSettings::setValid(bool valid) {
    m_Valid = valid;
}

uint32_t IConverterSettings::crc() const {
    return m_CRC;
}
void IConverterSettings::setCRC(uint32_t crc) {
    m_CRC = crc;
}

uint32_t IConverterSettings::version() const {
    return m_Version;
}

void IConverterSettings::setVersion(uint32_t version) {
    m_Version = version;
}

uint32_t IConverterSettings::currentVersion() const {
    return m_CurrentVersion;
}

void IConverterSettings::setCurrentVersion(uint32_t version) {
    m_CurrentVersion = version;
}

const char *IConverterSettings::source() const {
    return m_Source.c_str();
}
void IConverterSettings::setSource(const char *source) {
    m_Source = source;
}

const char *IConverterSettings::destination() const {
    return m_Destination.c_str();
}
void IConverterSettings::setDestination(const char *destination) {
    m_Destination = destination;
}

const char *IConverterSettings::absoluteDestination() const {
    return m_AbsoluteDestination.c_str();
}

void IConverterSettings::setAbsoluteDestination(const char *destination) {
    m_AbsoluteDestination = destination;
}

const QStringList IConverterSettings::subKeys() const {
    return m_SubItems.keys();
}

QString IConverterSettings::subItem(const QString &key) const {
    return m_SubItems.value(key);
}

int32_t IConverterSettings::subType(const QString &key) const {
    return m_SubTypes.value(key);
}

void IConverterSettings::setSubItem(const QString &name, const QString &uuid, int32_t type) {
    if(!name.isEmpty() && !uuid.isEmpty()) {
        m_SubItems[name] = uuid;
        m_SubTypes[name] = type;
    }
}

IConverterSettings *IConverter::createSettings() const {
    return new IConverterSettings();
}

QString IConverter::templatePath() const {
    return QString();
}
