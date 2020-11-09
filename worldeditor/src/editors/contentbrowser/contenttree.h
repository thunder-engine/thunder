#ifndef CONTENTTREE_H
#define CONTENTTREE_H

#include "baseobjectmodel.h"

#include <QImage>
#include <QSortFilterProxyModel>

class QFileSystemWatcher;

class ContentTreeFilter : public QSortFilterProxyModel {
public:
    explicit ContentTreeFilter(QObject *parent);

    void setContentTypes(const QStringList &list);

protected:
    bool canDropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent) const;

    bool dropMimeData(const QMimeData *data, Qt::DropAction, int, int, const QModelIndex &parent);

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkContentTypeFilter(int sourceRow, const QModelIndex &sourceParent) const;

    bool checkNameFilter(int sourceRow, const QModelIndex &sourceParent) const;

    QStringList m_List;
};

class ContentTree : public BaseObjectModel {
    Q_OBJECT

public:
    static ContentTree         *instance                    ();

    static void                 destroy                     ();

    int                         columnCount                 (const QModelIndex &) const;

    QVariant                    headerData                  (int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const;

    QVariant                    data                        (const QModelIndex &index, int role) const;

    bool                        setData                     (const QModelIndex &index, const QVariant &value, int role);

    Qt::ItemFlags               flags                       (const QModelIndex &index) const;

    QString                     path                        (const QModelIndex &index) const;

    bool                        removeResource              (const QModelIndex &index);

public slots:
    void                        onRendered                  (const QString &uuid);

    void                        update                      (const QString &path);

    void                        clean                       (QObject *parent);

private:
    ContentTree                 ();
    ~ContentTree                () {}

    static ContentTree         *m_pInstance;

protected:
    Qt::DropActions             supportedDropActions        () const;
    QStringList                 mimeTypes                   () const;
    QMimeData                  *mimeData                    (const QModelIndexList &indexes) const;

protected:
    QObject                    *m_pContent;

    QImage                      m_Folder;
};

#endif // CONTENTTREE_H
