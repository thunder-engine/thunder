#ifndef PLUGINDELEGATE_H
#define PLUGINDELEGATE_H

#include <QStyledItemDelegate>

class PluginDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    PluginDelegate                  (QObject *parent = 0);

    /// Destructor
    virtual ~PluginDelegate         ();

    void                            paint                       (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

#endif // PLUGINDELEGATE_H
