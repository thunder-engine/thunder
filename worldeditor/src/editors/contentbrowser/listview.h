#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>

class ListView : public QListView {
public:
    explicit ListView(QWidget *parent = nullptr);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
};

#endif // LISTVIEW_H
