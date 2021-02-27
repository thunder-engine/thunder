#include "rowitem.h"

RowItem::RowItem(TreeRow *row) :
    m_row(row) {

}

TreeRow *RowItem::treeRow() {
    return m_row;
}
