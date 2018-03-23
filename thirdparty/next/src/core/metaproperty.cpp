#include "core/metaproperty.h"

MetaProperty::MetaProperty(const Table *table) :
        m_pTable(table) {
    PROFILE_FUNCTION()
}

const char *MetaProperty::name() const {
    PROFILE_FUNCTION()
    return m_pTable->name;
}

bool MetaProperty::isValid() const {
    PROFILE_FUNCTION()
    return (m_pTable != nullptr);
}

const MetaType MetaProperty::type() const {
    PROFILE_FUNCTION()
    return MetaType(m_pTable->type);
}

Variant MetaProperty::read(const Object *obj) const {
    PROFILE_FUNCTION()
    return m_pTable->reader(obj);
}

void MetaProperty::write(Object *obj, const Variant &value) const {
    PROFILE_FUNCTION()
    m_pTable->writer(obj, value);
}
