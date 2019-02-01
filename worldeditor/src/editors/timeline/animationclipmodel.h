#ifndef ANIMATIONCLIPMODEL_H
#define ANIMATIONCLIPMODEL_H

#include <QAbstractItemModel>

#include <animationcurve.h>

class AnimationController;

class AnimationClipModel : public QAbstractItemModel {
    Q_OBJECT

    Q_PROPERTY(float position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int row READ row WRITE setRow NOTIFY rowChanged)
    Q_PROPERTY(int col READ col WRITE setCol NOTIFY rowChanged)

public:
    AnimationClipModel          (QObject *parent);

    void                        setController               (AnimationController *controller);

    QVariant                    data                        (const QModelIndex &index, int) const;

    QVariant                    headerData                  (int, Qt::Orientation, int) const;

    int                         columnCount                 (const QModelIndex &) const;

    QModelIndex                 index                       (int row, int column, const QModelIndex &parent = QModelIndex()) const;

    QModelIndex                 parent                      (const QModelIndex &) const;

    int                         rowCount                    (const QModelIndex &) const;

    Q_INVOKABLE QVariant        trackData                   (int track) const;

    Q_INVOKABLE void            setTrackData                (int track, const QVariant &data);

    Q_INVOKABLE int             maxPosition                 (int track);

    float                       position                    () const;
    void                        setPosition                 (float value);

    int                         row                         () const;
    void                        setRow                      (int value);

    int                         col                         () const;
    void                        setCol                      (int value);

    void                        selectItem                  (const QModelIndex &index);

    AnimationCurve::KeyFrame   *key                         (int row, int col, int index);

    void                        updateController            ();

public slots:
    void                        onAddKey                    (int row, int col, int pos);
    void                        onRemoveKey                 (int row, int col, int index);

signals:
    void                        changed                     ();

    void                        rowChanged                  ();
    void                        positionChanged             ();

protected:
    AnimationController        *m_pController;

    bool                        m_isHighlighted;

    QModelIndex                 m_HoverIndex;

    float                       m_Position;

    int                         m_Row;
    int                         m_Col;

};

#endif // ANIMATIONCLIPMODEL_H
