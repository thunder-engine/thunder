#ifndef ANIMATIONCLIPMODEL_H
#define ANIMATIONCLIPMODEL_H

#include <QAbstractItemModel>

class AnimationController;

class AnimationClipModel : public QAbstractItemModel {
public:
    AnimationClipModel          (QObject *parent);

    void                        setController               (AnimationController *controller);

    QVariant                    data                        (const QModelIndex &index, int) const;

    QVariant                    headerData                  (int, Qt::Orientation, int) const;

    int                         columnCount                 (const QModelIndex &) const;

    QModelIndex                 index                       (int row, int column, const QModelIndex &parent = QModelIndex()) const;

    QModelIndex                 parent                      (const QModelIndex &) const;

    int                         rowCount                    (const QModelIndex &) const;

    void                        setHighlighted              (const QModelIndex &index);

protected:
    AnimationController        *m_pController;

    bool                        m_isHighlighted;

    QModelIndex                 m_HoverIndex;
};

#endif // ANIMATIONCLIPMODEL_H
