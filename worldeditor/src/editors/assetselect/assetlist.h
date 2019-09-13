#ifndef ASSETLIST_H
#define ASSETLIST_H

#include "baseobjectmodel.h"

#include <QRect>

class Engine;

class AssetList  : public BaseObjectModel {
    Q_OBJECT

public:
    static AssetList           *instance                    ();

    static void                 destroy                     ();

    void                        init                        (Engine *engine);

    int                         columnCount                 (const QModelIndex &parent) const;

    QVariant                    headerData                  (int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const;

    QVariant                    data                        (const QModelIndex &index, int role) const;

    Qt::ItemFlags               flags                       (const QModelIndex &index) const;

    QImage                      icon                        (const QModelIndex &index) const;

    QString                     path                        (const QModelIndex &index) const;

    QModelIndex                 findResource                (const QString &resource) const;

public slots:
    void                        onRendered                  (const QString &uuid);

    void                        update                      ();

private:
    AssetList                   ();
    ~AssetList                  () {}

    static AssetList           *m_pInstance;

    Engine                     *m_pEngine;

    QRect                       m_DefaultIcon;

};

#endif // ASSETLIST_H
