#ifndef CONTENTTREE_H
#define CONTENTTREE_H

#include "baseobjectmodel.h"

#include <QImage>

class QFileSystemWatcher;

class ContentTree : public BaseObjectModel {
    Q_OBJECT

public:
    explicit ContentTree        (QObject *parent = 0);

    int                         columnCount                 (const QModelIndex &parent) const;

    QVariant                    headerData                  (int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const;

    QVariant                    data                        (const QModelIndex &index, int role) const;

    Qt::ItemFlags               flags                       (const QModelIndex &index) const;

    QString                     dirPath                     (const QModelIndex &index) const;

public slots:
    void                        update                      (const QString &path);

protected:
    Qt::DropActions             supportedDropActions        () const;
    QStringList                 mimeTypes                   () const;
    QMimeData                  *mimeData                    (const QModelIndexList &indexes) const;
    bool                        canDropMimeData             (const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const;
    bool                        dropMimeData                (const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent);

protected:
    QObject                    *m_pContent;

    QImage                      m_Folder;
};

#endif // CONTENTTREE_H
