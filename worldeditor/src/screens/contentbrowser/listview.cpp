#include "listview.h"

#include <QDrag>

ListView::ListView(QWidget *parent) :
        QListView(parent) {

}

void ListView::startDrag(Qt::DropActions supportedActions) {
    QDrag drag(this);
    const QModelIndexList indexes = selectedIndexes();
    drag.setMimeData(model()->mimeData(indexes));
    drag.setPixmap(QPixmap());
    drag.exec( supportedActions );
}
