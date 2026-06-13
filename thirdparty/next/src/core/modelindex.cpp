#include "core/modelindex.h"

#include "core/abstractitemmodel.h"

ModelIndex::ModelIndex() :
    m_model(nullptr),
    m_row(-1),
    m_column(0),
    m_uuid(0) {
}

bool ModelIndex::isValid() const {
    return m_model != nullptr && m_row >= 0;
}

int ModelIndex::row() const {
    return m_row;
}

int ModelIndex::column() const {
    return m_column;
}

const AbstractItemModel *ModelIndex::model() const {
    return m_model;
}

ModelIndex ModelIndex::parent() const {
    return m_model ? m_model->parent(*this) : ModelIndex();
}

uint32_t ModelIndex::internalId() const {
    return m_uuid;
}

bool ModelIndex::operator==(const ModelIndex &other) const {
    return m_model == other.m_model && m_row == other.m_row && m_column == other.m_column && m_uuid == other.m_uuid;
}

bool ModelIndex::operator!=(const ModelIndex &other) const {
    return !(*this == other);
}
