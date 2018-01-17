#include "core/ametaproperty.h"

AMetaProperty::AMetaProperty(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION()
}

const char *AMetaProperty::name() const {
    PROFILE_FUNCTION()
    return m_pTable->name;
}

bool AMetaProperty::isValid() const {
    PROFILE_FUNCTION()
    return (m_pTable != nullptr);
}

const AMetaType AMetaProperty::type() const {
    PROFILE_FUNCTION()
    return AMetaType(m_pTable->type);
}

AVariant AMetaProperty::read(const AObject *obj) const {
    PROFILE_FUNCTION()
    return m_pTable->reader(obj);
}

void AMetaProperty::write(AObject *obj, const AVariant &value) const {
    PROFILE_FUNCTION()
    m_pTable->writer(obj, value);
}
