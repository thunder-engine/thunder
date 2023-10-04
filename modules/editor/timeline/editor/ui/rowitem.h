#ifndef ROWITEM_H
#define ROWITEM_H

#include <QGraphicsWidget>

class TreeRow;

class RowItem : public QGraphicsWidget {
public:
    enum ItemType {
        RulerItem = Type + 1,
        TreeItem,
        TimelineItem
    };
public:
    explicit RowItem(TreeRow *row);

    TreeRow *treeRow();

protected:
    TreeRow *m_row;

};

#endif // ROWITEM_H
