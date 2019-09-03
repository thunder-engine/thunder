#ifndef ANIMATIONCLIPMODEL_H
#define ANIMATIONCLIPMODEL_H

#include <QAbstractItemModel>

#include <animationcurve.h>

class AnimationController;
class AnimationStateMachine;
class AnimationClip;

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

    Q_INVOKABLE bool            isExpanded                  (int track) const;

    Q_INVOKABLE int             expandHeight                (int track) const;

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

    void                        removeItem                  (const QModelIndex &index);

    AnimationCurve::KeyFrame   *key                         (int row, int col, uint32_t index);

    void                        updateController            ();

    AnimationClip              *clip                        () const { return m_pClip; }

    QStringList                 clips                       () const { return m_Clips.keys(); }

public slots:
    void                        onAddKey                    (int row, int col, int pos);
    void                        onRemoveKey                 (int row, int col, int index);

    void                        setClip                     (const QString &clip);

    void                        onExpanded                  (const QModelIndex &index);
    void                        onCollapsed                 (const QModelIndex &index);

signals:
    void                        changed                     ();

    void                        rowChanged                  ();
    void                        positionChanged             ();

protected:
    AnimationController        *m_pController;

    AnimationStateMachine      *m_pStateMachine;

    AnimationClip              *m_pClip;

    bool                        m_isHighlighted;

    QModelIndex                 m_HoverIndex;

    float                       m_Position;

    int                         m_Row;
    int                         m_Col;

    QMap<QString, AnimationClip *> m_Clips;

    QList<int>                  m_Expands;

};

#endif // ANIMATIONCLIPMODEL_H
