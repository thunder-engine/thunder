#include "abstractitemmodel.h"

ModelIndex AbstractItemModel::createIndex(int row, int column, uint32_t uuid) const {
    ModelIndex result;
    result.m_row = row;
    result.m_column = column;
    result.m_uuid = uuid;
    result.m_model = this;

    return result;
}
